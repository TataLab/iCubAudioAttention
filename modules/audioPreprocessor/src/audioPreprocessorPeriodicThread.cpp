// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Department of Neuroscience - University of Lethbridge
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

#define THPERIOD 0.01 // seconds.

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
	delete beamformer;
	delete hilbertTransform;
	delete butterworthFilter;
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

	numBands      = rf.findGroup("processing").check("numBands",      yarp::os::Value(128),    "number of frequency bands (int)"                       ).asInt();
	lowCf         = rf.findGroup("processing").check("lowCf",         yarp::os::Value(380.0),  "lowest center frequency (double)"                      ).asDouble();
	highCf        = rf.findGroup("processing").check("highCf",        yarp::os::Value(2800.0), "highest center frequency (double)"                     ).asDouble();
	halfRec       = rf.findGroup("processing").check("halfRec",       yarp::os::Value(false),  "half wave rectifying (boolean)"                        ).asBool();
	erbSpaced     = rf.findGroup("processing").check("erbSpaced",     yarp::os::Value(true),   "ERB spaced centre frequencies (boolean)"               ).asBool();
	exitEarly     = rf.findGroup("processing").check("exitEarly",     yarp::os::Value(false),  "exit process without computing hilbert trans (boolean)").asBool();
	bandPassFreq  = rf.findGroup("processing").check("bandPassFreq",  yarp::os::Value(5.0),    "frequency to use in band pass filter (double)"         ).asDouble();
	bandPassWidth = rf.findGroup("processing").check("bandPassWidth", yarp::os::Value(0.5),    "bandwidth allowed for band pass filter (double)"       ).asDouble();
	angleRes      = rf.findGroup("processing").check("angleRes",      yarp::os::Value(1),      "degree resolution for a single position (int)"         ).asInt();
	downSampEnv   = rf.findGroup("processing").check("downSampEnv",   yarp::os::Value(1),      "rate to down sample pre-envelope mat by (int)"         ).asInt();
	numOmpThreads = rf.findGroup("processing").check("numOmpThreads", yarp::os::Value(4),      "if enabled, the number of omp threads (int)"           ).asInt();

	downSampVis   = rf.findGroup("other").check("downSampVis",   yarp::os::Value(1),      "rate to down sample visualisation by (int)"               ).asInt();
	saveMatrices  = rf.findGroup("other").check("saveMatrices",  yarp::os::Value("NONE"), "if specified, will save all matrices to this dir (string)").asString();

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

	numFrameDownSamples = numFrameSamples / downSampEnv;


	//-- If save matricies was specified set process all flag to true.
	processAll = (saveMatrices != "NONE");
	if (processAll) {

		//-- Expand env vars if they are present.
		saveMatrices = AudioUtil::expandEnvironmentVariables(saveMatrices);

		//-- Create a directory for the matrices.
		if (!AudioUtil::makeDirectory(saveMatrices)) {
			yInfo("Failed to create directory %s. -- Check Permissions?", saveMatrices.c_str());
			return false;
		}
	}


	/* ===========================================================================
	 *  Initialize the matrices used for data processing.
	 * =========================================================================== */
	RawAudioMatrix.resize(numMics, numFrameSamples);
	RawAudioMatrix.zero();

	GammatoneFilteredAudioMatrix.resize(numMics * numBands, numFrameSamples);
	GammatoneFilteredAudioMatrix.zero();

	GammatoneFilteredPowerMatrix.resize(numBands, numMics);
	GammatoneFilteredPowerMatrix.zero();

	BeamformedAudioMatrix.resize(numBands * numBeams, numFrameSamples);
	BeamformedAudioMatrix.zero();

	BeamformedDownSampAudioMatrix.resize(numBands * numBeams, numFrameDownSamples);
	BeamformedDownSampAudioMatrix.zero();

	BeamformedRmsAudioMatrix.resize(numBands, numBeams);
	BeamformedRmsAudioMatrix.zero();

	BeamformedRmsPowerMatrix.resize(numBands, numMics);
	BeamformedRmsPowerMatrix.zero();

	AllocentricAudioMatrix.resize(numBands, numFullFieldAngles);
	AllocentricAudioMatrix.zero();

	HilbertEnvelopeMatrix.resize(numBands * numBeams, numFrameDownSamples);
	HilbertEnvelopeMatrix.zero();

	BandPassedEnvelopeMatrix.resize(numBands * numBeams, numFrameDownSamples);
	BandPassedEnvelopeMatrix.zero();

	BandPassedRmsEnvelopeMatrix.resize(numBands, numBeams);
	BandPassedRmsEnvelopeMatrix.zero();

	BandPassedRmsPowerMatrix.resize(numBands, numMics);
	BandPassedRmsPowerMatrix.zero();

	AllocentricEnvelopeMatrix.resize(numBands, numFullFieldAngles);
	AllocentricEnvelopeMatrix.zero();


	/* ===========================================================================
	 *  Initialize the processing objects.
	 * =========================================================================== */
	gammatoneFilterBank = new Filters::GammatoneFilterBank(numMics, samplingRate, numFrameSamples, numBands, lowCf, highCf, halfRec, erbSpaced);
	beamformer          = new Filters::Beamformer(numMics, micDistance, speedOfSound, samplingRate, numFrameSamples, numBands, numBeamsPerHemifield, angleRes);
	hilbertTransform    = new Filters::HilbertTransform(numBands * numBeams, numFrameDownSamples);
	butterworthFilter   = new Filters::ButterworthFilter(samplingRate / downSampEnv, bandPassWidth);


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
	yInfo( "\t Number of Beams Per Hemifield    : %d",       numBeamsPerHemifield                );
	yInfo( "\t Number of Front Field Beams      : %d",       numBeams                            );
	yInfo( "\t Number of Front Field Angles     : %d",       numFrontFieldAngles                 );
	yInfo( "\t Number of Full Field Angles      : %d",       numFullFieldAngles                  );
	yInfo( "\t Exit Early                       : %s",       exitEarly ? "ENABLED"  : "DISABLED" );
	yInfo( "\t Band Pass Frequency              : %.2f Hz",  bandPassFreq                        );
	yInfo( "\t Band Pass Width                  : %.2f",     bandPassWidth                       );
	yInfo( "\t Pre-envelope Matrix Down Sample  : %d",       downSampEnv                         );
	yInfo( "\t Number Down Samples per Frame    : %d",       numFrameDownSamples                 );
	#ifdef WITH_OMP
	yInfo( "\t Number of OpenMP Threads         : %d",       numOmpThreads                       );
	#else
	yInfo( "\t Number of OpenMP Threads         : DISABLED"                                      );
	#endif
	yInfo( " " );
	yInfo( "\t                    [OTHER]                   "                                    );
	yInfo( "\t ============================================ "                                    );
	yInfo( "\t Matrix Visualisation Down Sample : %d",       downSampVis                         );
	yInfo( "\t Directory for Matrix Output      : %s",       processAll ? saveMatrices.c_str() : "DISABLED");
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

	if (!exitEarly) {

		if (!outHilbertEnvelopePort.open(getName("/hilbertEnvelope:o").c_str())) {
			yError("Unable to open port for sending the Hilbert Envelope of the Filter bank.");
			return false;
		}

		if (!outBandPassedEnvelopePort.open(getName("/bandPassedEnvelope:o").c_str())) {
			yError("Unable to open port for sending the band passed Hilbert Envelope.");
			return false;
		}

		if (!outBandPassedRmsEnvelopePort.open(getName("/bandPassedRmsEnvelope:o").c_str())) {
			yError("Unable to open port for sending the root mean square of band passed Hilbert Envelope.");
			return false;
		}

		if (!outBandPassedRmsPowerPort.open(getName("/bandPassedRmsPower:o").c_str())) {
			yError("Unable to open port for sending the power of bands of the root mean square of band passed envelope.");
			return false;
		}

		if (!outAllocentricEnvelopePort.open(getName("/allocentricEnvelope:o").c_str())) {
			yError("Unable to open port for sending the allocentric envelope map.");
			return false;
		}

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
	outBeamformedAudioPort.interrupt();
	outBeamformedRmsAudioPort.interrupt();
	outBeamformedRmsPowerPort.interrupt();
	outAllocentricAudioPort.interrupt();
	outHilbertEnvelopePort.interrupt();
	outBandPassedEnvelopePort.interrupt();
	outBandPassedRmsEnvelopePort.interrupt();
	outBandPassedRmsPowerPort.interrupt();
	outAllocentricEnvelopePort.interrupt();
	
	//-- Close the threads.
	inRawAudioPort.close();
	inHeadAnglePort.close();
	outGammatoneFilteredAudioPort.close();
	outGammatoneFilteredPowerPort.close();
	outBeamformedAudioPort.close();
	outBeamformedRmsAudioPort.close();
	outBeamformedRmsPowerPort.close();
	outAllocentricAudioPort.close();
	outHilbertEnvelopePort.close();
	outBandPassedEnvelopePort.close();
	outBandPassedRmsEnvelopePort.close();
	outBandPassedRmsPowerPort.close();
	outAllocentricEnvelopePort.close();
	
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

		AudioUtil::makeTimeStamp(totalDelay, timeDelay, startTime, stopTime);
		
		//-- Get Input.
		inputSound = inRawAudioPort.read(true);
		inRawAudioPort.getEnvelope(timeStamp);

		//-- Get head position.
		headOffset = 0.0;
		if (inHeadAnglePort.getInputCount()) {
			headAngleBottle = inHeadAnglePort.read(true);
			headOffset += headAngleBottle->get(panAngle).asDouble();
		}

		AudioUtil::makeTimeStamp(totalReading, timeReading, startTime, stopTime);

		//-- Main Loop.
		result = processing();

		AudioUtil::makeTimeStamp(totalProcessing, timeProcessing, startTime, stopTime);

		//-- Write data to outgoing ports.
		publishOutPorts();

		//-- If saving output was set, save all matrices.
		if (processAll) { saveOutPorts(); }

		AudioUtil::makeTimeStamp(totalTransmission, timeTransmission, startTime, stopTime);

		//-- Give time stats to the user.
		timeTotal  = timeDelay + timeReading + timeProcessing + timeTransmission;
		totalTime += timeTotal;
		totalIterations++;
		yInfo("End of Loop %d, TS %d:  Offset  %.2f  |  Delay  %f  |  Reading  %f  |  Processing  %f  |  Transmission  %f  |  Total  %f  |", totalIterations, timeStamp.getCount(), headOffset, timeDelay, timeReading, timeProcessing, timeTransmission, timeTotal);
	}
}


bool AudioPreprocessorPeriodicThread::processing() {

	#ifdef WITH_OMP
	//-- Ensure we are always using the 
	//-- specified number of threads.
	omp_set_num_threads(numOmpThreads);
	#endif

	//-- Separate the sound into left and right channels.	
	AudioUtil::SoundToMatrix(inputSound, RawAudioMatrix);


	/* ===========================================================================
	 *  Apply a 4th order gammatone filter onto the raw audio for the 
	 *    specified number of bands. Also returns a bank of envelopes.
	 * =========================================================================== */
	gammatoneFilterBank->getGammatoneFilteredAudio (
		/* Raw Audio Source = */ RawAudioMatrix,
		/* Basilar Memb Res = */ GammatoneFilteredAudioMatrix
	);


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute the 
	 *    RMS power of the filter banks bands.
	 * =========================================================================== */
	if (processAll || outGammatoneFilteredPowerPort.getOutputCount()) {
		gammatoneFilterBank->getGammatoneFilteredPower (
			/* Basilar Memb Res  = */ GammatoneFilteredAudioMatrix,
			/* Power of the Bank = */ GammatoneFilteredPowerMatrix
		);
	}

	
	/* ===========================================================================
	 *  Apply a delay and sum beamformer with RMS applied 
	 *    across the frame onto the filtered left and right channel. 
	 * =========================================================================== */
	beamformer->getBeamformedAudio (
		/* Basilar Memb Res = */ GammatoneFilteredAudioMatrix,
		/* Beamformered     = */ BeamformedAudioMatrix
	);


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute the 
	 *    RMS power of the beamformed bands.
	 * =========================================================================== */
	if (processAll || outBeamformedRmsAudioPort.getOutputCount() || outBeamformedRmsPowerPort.getOutputCount() || outAllocentricAudioPort.getOutputCount()) {
		
		AudioUtil::RootMeanSquareMatrix (
			/* Source = */ BeamformedAudioMatrix,
			/* Target = */ BeamformedRmsAudioMatrix,
			/* Dim X  = */ numBands,
			/* Dim Y  = */ numBeams
		);

		if (processAll || outBeamformedRmsPowerPort.getOutputCount()) {
			beamformer->getBeamformedRmsPower (
				/* RMS Beamformer = */ BeamformedRmsAudioMatrix,
				/* Power of Beams = */ BeamformedRmsPowerMatrix
			);
		}

		if (processAll || outAllocentricAudioPort.getOutputCount()) {
			/* ===========================================================================
			 *  Interpolate over, and mirror the front field auditory scene
			 * =========================================================================== */
			beamformer->getAngleNormalAudioMap (
				/* Source = */ BeamformedRmsAudioMatrix,
				/* Target = */ AllocentricAudioMatrix,
				/* Offset = */ headOffset
			);
		}
	}

	if (exitEarly) {
		//-- Leave the processing loop early to avoid processing hilbert transform.
		return true;
	}

	/* ===========================================================================
	 *  Down sample the matrix by some factor to make the computation faster.
	 * =========================================================================== */
	AudioUtil::downSampleMatrix(
		/* Source = */ BeamformedAudioMatrix,
		/* Target = */ BeamformedDownSampAudioMatrix,
		/* Reduce = */ downSampEnv
	);


	/* ===========================================================================
	 *  Compute the hilbert envelope.
	 * =========================================================================== */
	hilbertTransform->getHilbertEnvelope (
		/* Source = */ BeamformedDownSampAudioMatrix,
		/* Target = */ HilbertEnvelopeMatrix
	);


	/* ===========================================================================
	 *  Isolate for the specified envelope (typically 5 Hz).
	 * =========================================================================== */
	butterworthFilter->getBandPassedAudio (
		/* Source = */ HilbertEnvelopeMatrix,
		/* Target = */ BandPassedEnvelopeMatrix,
		/* CFreq  = */ bandPassFreq
	);


	/* ===========================================================================
	 *  Collapse accross the frames.
	 * =========================================================================== */
	AudioUtil::RootMeanSquareMatrix (
		/* Source = */ BandPassedEnvelopeMatrix,
		/* Target = */ BandPassedRmsEnvelopeMatrix,
		/* Dim X  = */ numBands,
		/* Dim Y  = */ numBeams
	);


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute the 
	 *    RMS power of the band-passed beamformed-envelope bands.
	 *    ( We can reuse the function for getting beamformed rms power,  
	 *      as these matrices share the same shape ).
	 * =========================================================================== */
	if (processAll || outBandPassedRmsPowerPort.getOutputCount()) {
		beamformer->getBeamformedRmsPower (
			/* RMS Envelope = */ BandPassedRmsEnvelopeMatrix,
			/* Power of Env = */ BandPassedRmsPowerMatrix
		);
	}


	/* ===========================================================================
	 *  Interpolate over, and mirror the front field auditory scene
	 * =========================================================================== */
	beamformer->getAngleNormalAudioMap (
		/* Source = */ BandPassedRmsEnvelopeMatrix,
		/* Target = */ AllocentricEnvelopeMatrix,
		/* Offset = */ headOffset
	);

	return true;
}


void AudioPreprocessorPeriodicThread::publishOutPorts() {
	
	//-- Write to Active Ports.
	if (outGammatoneFilteredAudioPort.getOutputCount()) {

		//-- This Matrix can be very big. Down sample if enabled.
		AudioUtil::downSampleMatrix(GammatoneFilteredAudioMatrix, outGammatoneFilteredAudioPort.prepare(), downSampVis);
		outGammatoneFilteredAudioPort.setEnvelope(timeStamp);
		outGammatoneFilteredAudioPort.write();

	}

	if (outGammatoneFilteredPowerPort.getOutputCount()) {

		outGammatoneFilteredPowerPort.prepare() = GammatoneFilteredPowerMatrix;
		outGammatoneFilteredPowerPort.setEnvelope(timeStamp);
		outGammatoneFilteredPowerPort.write();

	}

	if (outBeamformedAudioPort.getOutputCount()) {
		
		//-- This Matrix can be very big. Down sample if enabled.
		AudioUtil::downSampleMatrix(BeamformedAudioMatrix, outBeamformedAudioPort.prepare(), downSampVis);
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

		outAllocentricAudioPort.prepare() = AllocentricAudioMatrix;
		outAllocentricAudioPort.setEnvelope(timeStamp);
		outAllocentricAudioPort.write();
		
	}

	if (outHilbertEnvelopePort.getOutputCount()) {

		//-- This Matrix can be very big. Down sample if enabled.
		AudioUtil::downSampleMatrix(HilbertEnvelopeMatrix, outHilbertEnvelopePort.prepare(), downSampVis);
		outHilbertEnvelopePort.setEnvelope(timeStamp);
		outHilbertEnvelopePort.write();

	}

	if (outBandPassedEnvelopePort.getOutputCount()) {

		//-- This Matrix can be very big. Down sample if enabled.
		AudioUtil::downSampleMatrix(BandPassedEnvelopeMatrix, outBandPassedEnvelopePort.prepare(), downSampVis);
		outBandPassedEnvelopePort.setEnvelope(timeStamp);
		outBandPassedEnvelopePort.write();

	}

	if (outBandPassedRmsEnvelopePort.getOutputCount()) {

		outBandPassedRmsEnvelopePort.prepare() = BandPassedRmsEnvelopeMatrix;
		outBandPassedRmsEnvelopePort.setEnvelope(timeStamp);
		outBandPassedRmsEnvelopePort.write();

	}

	if (outBandPassedRmsPowerPort.getOutputCount()) {

		outBandPassedRmsPowerPort.prepare() = BandPassedRmsPowerMatrix;
		outBandPassedRmsPowerPort.setEnvelope(timeStamp);
		outBandPassedRmsPowerPort.write();

	}

	if (outAllocentricEnvelopePort.getOutputCount()) {

		outAllocentricEnvelopePort.prepare() = AllocentricEnvelopeMatrix;
		outAllocentricEnvelopePort.setEnvelope(timeStamp);
		outAllocentricEnvelopePort.write();
		
	}
}


void AudioPreprocessorPeriodicThread::saveOutPorts() {

	AudioUtil::MatrixToFile(
		AudioUtil::downSampleMatrix(GammatoneFilteredAudioMatrix, downSampVis),
		saveMatrices + "GammatoneFilteredAudio_" +AudioUtil::leadingZeros(totalIterations,4)+".data"
	);
	AudioUtil::MatrixToFile(GammatoneFilteredPowerMatrix,    saveMatrices + "GammatoneFilteredPower_" +AudioUtil::leadingZeros(totalIterations,4)+".data");
	AudioUtil::MatrixToFile(
		AudioUtil::downSampleMatrix(BeamformedAudioMatrix, downSampVis),           
		saveMatrices + "BeamformedAudio_"+AudioUtil::leadingZeros(totalIterations,4)+".data"
	);
	AudioUtil::MatrixToFile(BeamformedRmsAudioMatrix,        saveMatrices + "BeamformedRmsAudio_"     +AudioUtil::leadingZeros(totalIterations,4)+".data");
	AudioUtil::MatrixToFile(AllocentricAudioMatrix,          saveMatrices + "AllocentricAudio_"       +AudioUtil::leadingZeros(totalIterations,4)+".data");
	if (!exitEarly) {
		AudioUtil::MatrixToFile(HilbertEnvelopeMatrix,       saveMatrices + "HilbertEnvelope_"        +AudioUtil::leadingZeros(totalIterations,4)+".data");
		AudioUtil::MatrixToFile(BandPassedEnvelopeMatrix,    saveMatrices + "BandPassedEnvelope_"     +AudioUtil::leadingZeros(totalIterations,4)+".data");
		AudioUtil::MatrixToFile(BandPassedRmsEnvelopeMatrix, saveMatrices + "BandPassedRmsEnvelope_"  +AudioUtil::leadingZeros(totalIterations,4)+".data");
		AudioUtil::MatrixToFile(BandPassedRmsPowerMatrix,    saveMatrices + "BandPassedRmsPower_"     +AudioUtil::leadingZeros(totalIterations,4)+".data");
		AudioUtil::MatrixToFile(AllocentricEnvelopeMatrix,   saveMatrices + "AllocentricEnvelope_"    +AudioUtil::leadingZeros(totalIterations,4)+".data");
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
