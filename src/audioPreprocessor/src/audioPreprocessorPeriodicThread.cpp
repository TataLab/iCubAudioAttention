// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2018 Department of Neuroscience - University of Lethbridge
 * Author: Austin Kothig, Francesco Rea, Marko Ilievski, Matt Tata
 * email: kothiga@uleth.ca, francesco.reak@iit.it, marko.ilievski@uwaterloo.ca, matthew.tata@uleth.ca
 * 
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

/* ===========================================================================
 * @file  audioPreprocessorPeriodicThread.cpp
 * @brief Implementation of the audioPreprocessorPeriodicThread (see header file).
 * =========================================================================== */

#include <iCub/audioPreprocessorPeriodicThread.h>

#define THPERIOD 0.08 // seconds.


template<class T>
inline int myRound(T a) {
    int ceilValue  = (int)ceil(a);
    int floorValue = (int)floor(a);
    return (a - floorValue <= 0.5) ? floorValue : ceilValue;
}


inline void downSampleMatrix(const yarp::sig::Matrix& source, yarp::sig::Matrix& target, const int downSamp) {
	
	//-- Don't do any down sampling. Standard Copy.
	if (downSamp == 1) { target = source; return; }

	//-- Allocate space for the down sampled matrix.
	const int RowSize = source.rows();
	const int ColSize = source.cols() / downSamp;
	const int offset  = source.cols() % downSamp;
	target.resize(RowSize, ColSize);
	
	//-- Set a pointer to the target.
	double *trg = target.data();

	//-- Set a pointer to the source.
	const double *src = source.data();

	for (int row = 0; row < RowSize; row++) {
		for (int col = 0; col < ColSize; col++) {
			//-- Dereference and copy data.
			*trg = *src;
			trg++; src += downSamp;
		}
		//-- Skip any remaining points at the end of the row.
		src += offset;
	}
}


inline void MatrixToImageOfFloat(const yarp::sig::Matrix& source, YarpImageOfFloat& target) {

	//-- Allocate space for the Image.
	const size_t RowSize = source.rows();
	const size_t ColSize = source.cols();
	target.resize(ColSize, RowSize);
	
	//-- Set a pointer to the data.
	const double *src = source.data();

	//-- Iterate through copying the contents
	//-- from the matrix, into the target image.
	//-- Matrix holds doubles. Conversion to
	//-- floats should be ok.
	for (int row = 0; row < RowSize; row++) {
		for (int col = 0; col < ColSize; col++) {
			target(col, row) = *src;
			src++;
		}
	}
}


inline void makeTimeStamp(double& totalStat, double& timeStat, double& startTime, double& stopTime) {
	stopTime   = yarp::os::Time::now();
	timeStat   = stopTime - startTime;
	totalStat += timeStat;
	startTime  = stopTime;
}


AudioPreprocessorPeriodicThread::AudioPreprocessorPeriodicThread() : 
	PeriodicThread(THPERIOD) {

	robot = "icub";
	startTime = yarp::os::Time::now();        
}


AudioPreprocessorPeriodicThread::AudioPreprocessorPeriodicThread(std::string _robot, std::string _configFile) : 
	PeriodicThread(THPERIOD) {

	robot = _robot;
	configFile = _configFile;
	startTime = yarp::os::Time::now();
}


AudioPreprocessorPeriodicThread::~AudioPreprocessorPeriodicThread() {
	delete gammatoneFilterBank;
	delete interauralCues;
}


bool AudioPreprocessorPeriodicThread::configure(yarp::os::ResourceFinder &rf) {

	/* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );

	panAngle    = rf.findGroup("robotspec").check("panAngle",    yarp::os::Value(2),     "index of pan joint (int)"          ).asInt();
	numMics     = rf.findGroup("robotspec").check("numMics",     yarp::os::Value(2),     "number of mics (int)"              ).asInt();
	micDistance = rf.findGroup("robotspec").check("micDistance", yarp::os::Value(0.145), "distance between the mics (double)").asDouble();

	speedOfSound    = rf.findGroup("sampling").check("speedOfSound",    yarp::os::Value(336.628), "speed of sound (double)"               ).asDouble();
	samplingRate    = rf.findGroup("sampling").check("samplingRate",    yarp::os::Value(48000),   "sampling rate of mics (int)"           ).asInt();
	numFrameSamples = rf.findGroup("sampling").check("numFrameSamples", yarp::os::Value(4096),    "number of frame samples received (int)").asInt();

	numBands      = rf.findGroup("processing").check("numBands",      yarp::os::Value(128),    "number of frequency bands (int)"              ).asInt();
	lowCf         = rf.findGroup("processing").check("lowCf",         yarp::os::Value(380.0),  "lowest center frequency (double)"             ).asDouble();
	highCf        = rf.findGroup("processing").check("highCf",        yarp::os::Value(2800.0), "highest center frequency (double)"            ).asDouble();
	halfRec       = rf.findGroup("processing").check("halfRec",       yarp::os::Value(false),  "half wave rectifying (boolean)"               ).asBool();
	erbSpaced     = rf.findGroup("processing").check("erbSpaced",     yarp::os::Value(true),   "ERB spaced centre frequencies (boolean)"      ).asBool();
	bandPassFreq  = rf.findGroup("processing").check("bandPassFreq",  yarp::os::Value(5.0),    "frequency to use in band pass filter (double)").asDouble();
	angleRes      = rf.findGroup("processing").check("angleRes",      yarp::os::Value(1),      "degree resolution for a single position (int)").asInt();
	downSamp      = rf.findGroup("processing").check("downSamp",      yarp::os::Value(1),      "number of frames to down sample by (int)"     ).asInt();
	numOmpThreads = rf.findGroup("processing").check("numOmpThreads", yarp::os::Value(4),      "if enabled, the number of omp threads (int)"  ).asInt();


	/* ===========================================================================
	 *  Derive additional variables given the ones above.
	 * =========================================================================== */
	
	//-- Take the ceiling of of (D/C)/Rate.
	//--   ceiling = (x + y - 1) / y
	numBeamsPerHemifield = ((micDistance * samplingRate) + speedOfSound - 1.0) / speedOfSound;

	//-- Take the one less the floor of (D/C)*Rate
	//numBeamsPerHemifield = int((micDistance / speedOfSound) * samplingRate) - 1; //-- For a different number of beams.
	
	//-- Find the total number of beams for the front field.
	numBeams = 2 * numBeamsPerHemifield + 1;

	//-- Using the provided angle resolution, find number of 
	//-- front and full field angle positions we will use.
	numFrontFieldAngles = _baseAngles * angleRes + 1;
	numFullFieldAngles  = _baseAngles * angleRes * 2;


	/* ===========================================================================
	 *  Initialize the matrices used for data processing.
	 * =========================================================================== */
	RawAudioMatrix.resize(numMics, numFrameSamples);
	RawAudioMatrix.zero();

	GammatoneFilteredAudioMatrix.resize(numMics * numBands, numFrameSamples);
	GammatoneFilteredAudioMatrix.zero();

	GammatoneFilteredPowerMatrix.resize(numBands, numMics);
	GammatoneFilteredPowerMatrix.zero();

	HilbertEnvelopeMatrix.resize(numMics * numBands, numFrameSamples);
	HilbertEnvelopeMatrix.zero();

	BandPassedAudioMatrix.resize(numMics * numBands, numFrameSamples);
	BandPassedAudioMatrix.zero();

	BeamformedAudioMatrix.resize(numBands * numBeams, numFrameSamples);
	BeamformedAudioMatrix.zero();

	BeamformedRmsAudioMatrix.resize(numBands, numBeams);
	BeamformedRmsAudioMatrix.zero();

	BeamformedRmsPowerMatrix.resize(numBands, numMics);
	BeamformedRmsPowerMatrix.zero();

	AllocentricAudioMatrix.resize(numBands, numFullFieldAngles);
	AllocentricAudioMatrix.zero();


	/* ===========================================================================
	 *  Initialize the processing objects.
	 * =========================================================================== */
	gammatoneFilterBank = new GammatoneFilterBank(numMics, samplingRate, numFrameSamples, numBands, lowCf, highCf, halfRec, erbSpaced);
	interauralCues      = new InterauralCues(numMics, micDistance, speedOfSound, samplingRate, numFrameSamples, numBands, numBeamsPerHemifield, angleRes);


	/* ===========================================================================
	 *  Initialize time counters to zero.
	 * =========================================================================== */
	totalDelay        = 0.0;
	totalReading      = 0.0;
	totalProcessing   = 0.0;
	totalTransmission = 0.0;
	totalTime         = 0.0;
	totalIterations   = 0;


	/* =========================================================================== 
	 *  Print the resulting variables to the console.
	 * =========================================================================== */
	yInfo( " " );
	yInfo( "\t               [ROBOT SPECIFIC]               "                                    );
	yInfo( "\t ============================================ "                                    );
	yInfo( "\t Index of Pan Joint               : %d",       panAngle                            );
	yInfo( "\t Number of Microphones            : %d",       numMics                             );
	yInfo( "\t Microphone Distance              : %.3f m",   micDistance                         );
	yInfo( " " );
	yInfo( "\t                  [SAMPLING]                  "                                    );
	yInfo( "\t ============================================ "                                    );
	yInfo( "\t Speed of Sound                   : %.2f m/s", speedOfSound                        );
	yInfo( "\t Sampling Rate                    : %d Hz",    samplingRate                        );
	yInfo( "\t Number Samples per Frame         : %d",       numFrameSamples                     );
	yInfo( " " );
	yInfo( "\t                 [PROCESSING]                 "                                    );
	yInfo( "\t ============================================ "                                    );
	yInfo( "\t Number of Frequency Bands        : %d",       numBands                            );
	yInfo( "\t Lowest Center Frequency          : %.2f Hz",  lowCf                               );
	yInfo( "\t Highest Center Frequency         : %.2f Hz",  highCf                              );
	yInfo( "\t Half-Wave Rectifying             : %s",       halfRec   ? "ENABLED"  : "DISABLED" );
	yInfo( "\t Center Frequency Spacing         : %s",       erbSpaced ? "ERB-Rate" :  "Linear"  );
	yInfo( "\t Band Pass Frequency              : %.2f Hz",  bandPassFreq                        );
	yInfo( "\t Number of Beams Per Hemifield    : %d",       numBeamsPerHemifield                );
	yInfo( "\t Number of Front Field Beams      : %d",       numBeams                            );
	yInfo( "\t Number of Front Field Angles     : %d",       numFrontFieldAngles                 );
	yInfo( "\t Number of Full Field Angles      : %d",       numFullFieldAngles                  );
	yInfo( "\t Big Matrix Down Sample           : %d",       downSamp                            );
	#ifdef WITH_OMP
	yInfo( "\t Number of OpenMP Threads         : %d",       numOmpThreads                       );
	#else
	yInfo( "\t Number of OpenMP Threads         : DISABLED"                                      );
	#endif
	yInfo( " " );

	return true;
}


bool AudioPreprocessorPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */

	if (!inRawAudioPort.open(getName("/rawAudio:i").c_str())) {
		yError("Unable to open port for receiving raw audio from head.");
		return false;
	}

	if (!inHeadAnglePort.open(getName("/headAngle:i").c_str())) {
		yError("Unable to open port for receiving robot head angle.");
		return false;
	}

	if (!outGammatoneFilteredAudioPort.open(getName("/gammatoneFilteredAudio:o").c_str())) {
		yError("Unable to open port for sending the output of the gammatone filter bank.");
		return false;
	}

	if (!outGammatoneFilteredPowerPort.open(getName("/gammatoneFilteredPower:o").c_str())) {
		yError("Unable to open port for sending the power of bands of the gammatone filter bank.");
		return false;
	}

	if (!outHilbertEnvelopePort.open(getName("/hilbertEnvelope:o").c_str())) {
		yError("Unable to open port for sending the Hilbert Envelope of the Filter bank.");
		return false;
	}

	if (!outBandPassedAudioPort.open(getName("/bandPassedAudio:o").c_str())) {
		yError("Unable to open port for sending the band passed Hilbert Envelope.");
		return false;
	}

	if (!outBeamformedAudioPort.open(getName("/beamformedAudio:o").c_str())) {
		yError("Unable to open port for sending the beamformed audio.");
		return false;
	}

	if (!outBeamformedRmsAudioPort.open(getName("/beamformedRmsAudio:o").c_str())) {
		yError("Unable to open port for sending the root mean square of beamformed audio.");
		return false;
	}

	if (!outBeamformedRmsPowerPort.open(getName("/beamformedRmsPower:o").c_str())) {
		yError("Unable to open port for sending the power of bands of the root mean square of beamformed audio.");
		return false;
	}

	if (!outAllocentricAudioPort.open(getName("/allocentricAudio:o").c_str())) {
		yError("Unable to open port for sending the allocentric audio map.");
		return false;
	}

	stopTime = yarp::os::Time::now();
	yInfo("Initialization of the processing thread correctly ended. Elapsed Time: %f.", stopTime - startTime);
	startTime = stopTime;

	return true;
}


void AudioPreprocessorPeriodicThread::threadRelease() {

	//-- Stop all threads.
	inRawAudioPort.interrupt();
	inHeadAnglePort.interrupt();
	outGammatoneFilteredAudioPort.interrupt();
	outGammatoneFilteredPowerPort.interrupt();
	outHilbertEnvelopePort.interrupt();
	outBandPassedAudioPort.interrupt();
	outBeamformedAudioPort.interrupt();
	outBeamformedRmsAudioPort.interrupt();
	outBeamformedRmsPowerPort.interrupt();
	outAllocentricAudioPort.interrupt();
	
	//-- Close the threads.
	inRawAudioPort.close();
	inHeadAnglePort.close();
	outGammatoneFilteredAudioPort.close();
	outGammatoneFilteredPowerPort.close();
	outHilbertEnvelopePort.close();
	outBandPassedAudioPort.close();
	outBeamformedAudioPort.close();
	outBeamformedRmsAudioPort.close();
	outBeamformedRmsPowerPort.close();
	outAllocentricAudioPort.close();
	
	//-- Print thread stats.
	endOfProcessingStats();	
}


void AudioPreprocessorPeriodicThread::setName(std::string str) {
	this->name=str;
}


std::string AudioPreprocessorPeriodicThread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void AudioPreprocessorPeriodicThread::setInputPortName(std::string InpPort) {
	//-- Do nothing.
}


void AudioPreprocessorPeriodicThread::run() {    
	
	if (inRawAudioPort.getInputCount()) {

		makeTimeStamp(totalDelay, timeDelay, startTime, stopTime);
		
		//-- Get Input.
		inputSound = inRawAudioPort.read(true);
		inRawAudioPort.getEnvelope(timeStamp);

		//-- Get head position.
		headOffset = 0.0;
		if (inHeadAnglePort.getInputCount()) {
			headAngleBottle = inHeadAnglePort.read(true);
			headOffset += headAngleBottle->get(panAngle).asDouble();
		}

		makeTimeStamp(totalReading, timeReading, startTime, stopTime);

		//-- Main Loop.
		result = processing();

		makeTimeStamp(totalProcessing, timeProcessing, startTime, stopTime);

		//-- Write data to outgoing ports.
		publishOutPorts();	

		makeTimeStamp(totalTransmission, timeTransmission, startTime, stopTime);

		//-- Give time stats to the user.
		timeTotal  = timeDelay + timeReading + timeProcessing + timeTransmission;
		totalTime += timeTotal;
		totalIterations++;
		yInfo("End of Loop %d:  Offset  %.2f  |  Delay  %f  |  Reading  %f  |  Processing  %f  |  Transmission  %f  |  Total  %f  |", timeStamp.getCount(), headOffset, timeDelay, timeReading, timeProcessing, timeTransmission, timeTotal);
	}
}


bool AudioPreprocessorPeriodicThread::processing() {

	#ifdef WITH_OMP
	//-- Ensure we are always using the 
	//-- specified number of threads.
	omp_set_num_threads(numOmpThreads);
	#endif

	//-- Separate the sound into left and right channels.	
	for (int channel = 0; channel < numMics; channel++) {
		for (int sample = 0; sample < numFrameSamples; sample++) {
			RawAudioMatrix[channel][sample] = inputSound->get(sample, channel) / _norm;
		}
	}


	/* ===========================================================================
	 *  Apply a 4th order gammatone filter onto the raw audio for the 
	 *    specified number of bands. Also returns a bank of envelopes.
	 * =========================================================================== */
	gammatoneFilterBank->getGammatoneFilteredAudio (
		/* Raw Audio Source = */ RawAudioMatrix,
		/* Basilar Memb Res = */ GammatoneFilteredAudioMatrix,
		/* Hilbert Envelope = */ HilbertEnvelopeMatrix
	);


	/* ===========================================================================
	 *  Filter the Envelopes looking for specifically the envelope of human voices.
	 * =========================================================================== */
	gammatoneFilterBank->getBandPassedAudio (
		/* Audio Bank Source  = */ HilbertEnvelopeMatrix,
		/* Band Passed Bank   = */ BandPassedAudioMatrix,
		/* Desired Audio Band = */ bandPassFreq
	);


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute the 
	 *    RMS power of the filter banks bands.
	 * =========================================================================== */
	if (outGammatoneFilteredPowerPort.getOutputCount()) {
		gammatoneFilterBank->getGammatoneFilteredPower (
			/* Basilar Memb Res  = */ BandPassedAudioMatrix, //GammatoneFilteredAudioMatrix,
			/* Power of the Bank = */ GammatoneFilteredPowerMatrix
		);
	}

	
	/* ===========================================================================
	 *  Apply a delay and sum beamformer with RMS applied 
	 *    across the frame onto the filtered left and right channel. 
	 * =========================================================================== */
	interauralCues->getBeamformedRmsAudio (
		/* Basilar Memb Res = */ GammatoneFilteredAudioMatrix,
		/* RMS Beamformer   = */ BeamformedRmsAudioMatrix
	);


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute the 
	 *    RMS power of the beamformed bands.
	 * =========================================================================== */
	if (outBeamformedRmsPowerPort.getOutputCount()) {
		interauralCues->getBeamformedRmsPower (
			/* RMS Beamformer = */ BeamformedRmsAudioMatrix,
			/* Power of Beams = */ BeamformedRmsPowerMatrix
		);
	}


	/* ===========================================================================
	 *  Interpolate over, and mirror the front field auditory scene
	 * =========================================================================== */
	interauralCues->getAngleNormalAudioMap (
		/* Source = */ BeamformedRmsAudioMatrix,
		/* Target = */ AllocentricAudioMatrix,
		/* Offset = */ myRound(headOffset)
	);

	return true;
}


void AudioPreprocessorPeriodicThread::publishOutPorts() {
	
	//-- Write to Active Ports.
	if (outGammatoneFilteredAudioPort.getOutputCount()) {

		//-- This Matrix can be very big. Down sample if enabled.
		//downSampleMatrix(GammatoneFilteredAudioMatrix, outGammatoneFilteredAudioPort.prepare(), downSamp);
		MatrixToImageOfFloat(GammatoneFilteredAudioMatrix, outGammatoneFilteredAudioPort.prepare());
		outGammatoneFilteredAudioPort.setEnvelope(timeStamp);
		outGammatoneFilteredAudioPort.write();

	}

	if (outGammatoneFilteredPowerPort.getOutputCount()) {

		outGammatoneFilteredPowerPort.prepare() = GammatoneFilteredPowerMatrix;
		outGammatoneFilteredPowerPort.setEnvelope(timeStamp);
		outGammatoneFilteredPowerPort.write();

	}

	if (outHilbertEnvelopePort.getOutputCount()) {

		//-- This Matrix can be very big. Down sample if enabled.
		downSampleMatrix(HilbertEnvelopeMatrix, outHilbertEnvelopePort.prepare(), downSamp);
		outHilbertEnvelopePort.setEnvelope(timeStamp);
		outHilbertEnvelopePort.write();

	}

	if (outBandPassedAudioPort.getOutputCount()) {

		//-- This Matrix can be very big. Down sample if enabled.
		downSampleMatrix(BandPassedAudioMatrix, outBandPassedAudioPort.prepare(), downSamp);
		outBandPassedAudioPort.setEnvelope(timeStamp);
		outBandPassedAudioPort.write();

	}


	if (outBeamformedAudioPort.getOutputCount()) {

		//outBeamformedAudioPort.prepare() = BeamformedAudioMatrix;
		MatrixToImageOfFloat(BeamformedAudioMatrix, outBeamformedAudioPort.prepare());
		outBeamformedAudioPort.setEnvelope(timeStamp);
		outBeamformedAudioPort.write();

	}

	if (outBeamformedRmsAudioPort.getOutputCount()) {

		outBeamformedRmsAudioPort.prepare() = BeamformedRmsAudioMatrix;
		outBeamformedRmsAudioPort.setEnvelope(timeStamp);
		outBeamformedRmsAudioPort.write();

	}

	if (outBeamformedRmsPowerPort.getOutputCount()) {

		outBeamformedRmsPowerPort.prepare() = BeamformedRmsPowerMatrix;
		outBeamformedRmsPowerPort.setEnvelope(timeStamp);
		outBeamformedRmsPowerPort.write();

	}

	if (outAllocentricAudioPort.getOutputCount()) {

		//outAllocentricAudioPort.prepare() = AllocentricAudioMatrix;
		MatrixToImageOfFloat(AllocentricAudioMatrix, outAllocentricAudioPort.prepare());
		outAllocentricAudioPort.setEnvelope(timeStamp);
		outAllocentricAudioPort.write();
		
	}
}


void AudioPreprocessorPeriodicThread::endOfProcessingStats() {

	//-- Display Execution stats.
	yInfo(" ");
	yInfo("End of Thread . . . ");
	yInfo(" ");
	yInfo("\t Total Iterations : %d", totalIterations);
	yInfo("\t Total Time       : %.2f", totalTime);
	yInfo(" ");
	yInfo("Average Stats . . . ");
	yInfo(" ");
	yInfo("\t Delay        : %f", totalDelay        / (double) totalIterations );
	yInfo("\t Reading      : %f", totalReading      / (double) totalIterations );
	yInfo("\t Processing   : %f", totalProcessing   / (double) totalIterations );
	yInfo("\t Transmission : %f", totalTransmission / (double) totalIterations );
	yInfo("\t Loop Time    : %f", totalTime         / (double) totalIterations );
	yInfo(" ");
}
