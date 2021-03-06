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

	azimuthIndex = rf.findGroup("robotspec").check("azimuthIndex",    yarp::os::Value(2), "index of azimuth (or pan joint) (int)").asInt();
	numMics      = rf.findGroup("robotspec").check("numMics",     yarp::os::Value(2),     "number of mics (int)"                 ).asInt();
	micDistance  = rf.findGroup("robotspec").check("micDistance", yarp::os::Value(0.145), "distance between the mics (double)"   ).asDouble();

	speedOfSound     = rf.findGroup("sampling").check("speedOfSound",     yarp::os::Value(336.628), "speed of sound (double)"                ).asDouble();
	samplingRate     = rf.findGroup("sampling").check("samplingRate",     yarp::os::Value(48000),   "sampling rate of mics (int)"            ).asInt();
	numFrameSamples  = rf.findGroup("sampling").check("numFrameSamples",  yarp::os::Value(4096),    "number of frame samples received (int)" ).asInt();
	sampleNormaliser = rf.findGroup("sampling").check("sampleNormaliser", yarp::os::Value(1024.0),  "normalisation value for samples (double").asDouble();

	numBands        = rf.findGroup("processing").check("numBands",        yarp::os::Value(128),    "number of frequency bands (int)"                    ).asInt();
	lowCf           = rf.findGroup("processing").check("lowCf",           yarp::os::Value(380.0),  "lowest center frequency (double)"                   ).asDouble();
	highCf          = rf.findGroup("processing").check("highCf",          yarp::os::Value(7800.0), "highest center frequency (double)"                  ).asDouble();
	erbSpaced       = rf.findGroup("processing").check("erbSpaced",       yarp::os::Value(true),   "ERB spaced centre frequencies (boolean)"            ).asBool();
	halfRec         = rf.findGroup("processing").check("halfRec",         yarp::os::Value(false),  "half wave rectifying (boolean)"                     ).asBool();
	computeEnvelope = rf.findGroup("processing").check("computeEnvelope", yarp::os::Value(false),  "should envelope computation be done (boolean)"      ).asBool();
	downSampEnv     = rf.findGroup("processing").check("downSampEnv",     yarp::os::Value(4),      "rate to down sample pre-envelope mat by (int)"      ).asInt();
	downSampMethod  = rf.findGroup("processing").check("downSampMethod",  yarp::os::Value(""),     "method for down sampling (string)"                  ).asString();
	bandPassFreq    = rf.findGroup("processing").check("bandPassFreq",    yarp::os::Value(5.0),    "frequency to use in band pass filter (double)"      ).asDouble();
	bandPassWidth   = rf.findGroup("processing").check("bandPassWidth",   yarp::os::Value(0.5),    "bandwidth allowed for band pass filter (double)"    ).asDouble();
	angleRes        = rf.findGroup("processing").check("angleRes",        yarp::os::Value(1),      "degree resolution for a single position (int)"      ).asInt();
	windowLength    = rf.findGroup("processing").check("windowLength",    yarp::os::Value(128),    "number of samples in a single window (int)"         ).asInt();
	hopLength       = rf.findGroup("processing").check("hopLength",       yarp::os::Value(128),    "number of samples to jump for the next window (int)").asInt();
	windowMethod    = rf.findGroup("processing").check("windowMethod",    yarp::os::Value("rms"),  "operation to do over the samples in window (string)").asString();
	downSampOut     = rf.findGroup("processing").check("downSampOut",     yarp::os::Value(1),      "rate to down sample visualisation by (int)"         ).asInt();
	numOmpThreads   = rf.findGroup("processing").check("numOmpThreads",   yarp::os::Value(4),      "if enabled, the number of omp threads (int)"        ).asInt();
	
	/* ===========================================================================
	 *  Derive additional variables given the ones above.
	 * =========================================================================== */
	
	//-- Take the ceiling of of (D/C)/Rate.
	//--   ceiling = (x + y - 1) / y
	numBeamsPerHemifield = ((micDistance * samplingRate) + speedOfSound - 1.0) / speedOfSound;

	//-- Take the one less the floor of (D/C)*Rate.
	//numBeamsPerHemifield = int((micDistance / speedOfSound) * samplingRate) - 1; //-- For a different number of beams.
	
	//-- Find the total number of beams for the front field.
	numBeams = 2 * numBeamsPerHemifield + 1;

	//-- Using the provided angle resolution, find number of 
	//-- front and full field angle positions we will use.
	numFrontFieldAngles = _baseAngles * angleRes + 1;
	numFullFieldAngles  = _baseAngles * angleRes * 2;

	numFrameDownSamples = numFrameSamples / downSampEnv;
	

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
	yInfo( "\t Index of Pan Joint               : %d",       azimuthIndex                        );
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
	yInfo( "\t Center Frequency Spacing         : %s",       erbSpaced ? "ERB-Rate" :  "Linear"  );
	yInfo( "\t Half-Wave Rectifying             : %s",       halfRec   ? "ENABLED"  : "DISABLED" );
	yInfo( "\t Compute Envelope of Beamformer   : %s",       computeEnvelope ? "ENABLED"  : "DISABLED" );
	yInfo( "\t Number of Beams Per Hemifield    : %d",       numBeamsPerHemifield                );
	yInfo( "\t Number of Front Field Beams      : %d",       numBeams                            );
	if (computeEnvelope) {
	yInfo( "\t Pre-envelope Matrix Down Sample  : %d",       downSampEnv                         );
	yInfo( "\t Number Down Samples per Frame    : %d",       numFrameDownSamples                 );
	yInfo( "\t Down Sampling Method             : %s",       downSampMethod == "" ? "drop" : downSampMethod.c_str());
	yInfo( "\t Band Pass Frequency              : %.2f Hz",  bandPassFreq                        );
	yInfo( "\t Band Pass Width                  : %.2f",     bandPassWidth                       );
	}
	yInfo( "\t Number of Front Field Angles     : %d",       numFrontFieldAngles                 );
	yInfo( "\t Number of Full Field Angles      : %d",       numFullFieldAngles                  );
	yInfo( "\t Length of Window                 : %d",       windowLength                        );	
	yInfo( "\t Hop Length of Window             : %d",       hopLength                           );	
	yInfo( "\t Window processing method         : %s",       windowMethod.c_str()                );
	yInfo( "\t Matrix Visualisation Down Sample : %d",       downSampOut                         );
	yInfo( "\t Number Down Samples per Frame    : %d",       numFrameSamples / downSampOut       );
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

	if (!outGammatoneFilteredWindowPort.open(getName("/gammatoneFilteredWindow:o").c_str())) {
		yError("Unable to open port for sending the windowed gammatone filter bank.");
		return false;
	}

	if (!outBeamformedAudioPort.open(getName("/beamformedAudio:o").c_str())) {
		yError("Unable to open port for sending the beamformed audio.");
		return false;
	}

	if (!outBeamformedWindowPort.open(getName("/beamformedWindow:o").c_str())) {
		yError("Unable to open port for sending the windowed beamformed audio.");
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

	if (computeEnvelope) {

		if (!outHilbertEnvelopePort.open(getName("/hilbertEnvelope:o").c_str())) {
			yError("Unable to open port for sending the Hilbert Envelope of the Filter bank.");
			return false;
		}

		if (!outHilbertEnvelopeWindowPort.open(getName("/hilbertEnvelopeWindow:o").c_str())) {
			yError("Unable to open port for sending the Windowd Hilbert Envelope of the Filter bank.");
			return false;
		}

		if (!outBandPassedEnvelopePort.open(getName("/bandPassedEnvelope:o").c_str())) {
			yError("Unable to open port for sending the band passed Hilbert Envelope.");
			return false;
		}

		if (!outBandPassedEnvelopeWindowPort.open(getName("/bandPassedEnvelopeWindow:o").c_str())) {
			yError("Unable to open port for sending the Windowed band passed Hilbert Envelope.");
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
	outGammatoneFilteredWindowPort.interrupt();
	outBeamformedAudioPort.interrupt();
	outBeamformedWindowPort.interrupt();
	outBeamformedRmsAudioPort.interrupt();
	outBeamformedRmsPowerPort.interrupt();
	outAllocentricAudioPort.interrupt();
	outHilbertEnvelopePort.interrupt();
	outHilbertEnvelopeWindowPort.interrupt();
	outBandPassedEnvelopePort.interrupt();
	outBandPassedEnvelopeWindowPort.interrupt();
	outBandPassedRmsEnvelopePort.interrupt();
	outBandPassedRmsPowerPort.interrupt();
	outAllocentricEnvelopePort.interrupt();
	
	//-- Close the threads.
	inRawAudioPort.close();
	inHeadAnglePort.close();
	outGammatoneFilteredAudioPort.close();
	outGammatoneFilteredPowerPort.close();
	outGammatoneFilteredWindowPort.close();
	outBeamformedAudioPort.close();
	outBeamformedWindowPort.close();
	outBeamformedRmsAudioPort.close();
	outBeamformedRmsPowerPort.close();
	outAllocentricAudioPort.close();
	outHilbertEnvelopePort.close();
	outHilbertEnvelopeWindowPort.close();
	outBandPassedEnvelopePort.close();
	outBandPassedEnvelopeWindowPort.close();
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
			headOffset += headAngleBottle->get(azimuthIndex).asDouble();
		}

		AudioUtil::makeTimeStamp(totalReading, timeReading, startTime, stopTime);

		//-- Main Loop.
		result = processing();

		AudioUtil::makeTimeStamp(totalProcessing, timeProcessing, startTime, stopTime);

		//-- Write data to outgoing ports.
		publishOutPorts();

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
	AudioUtil::SoundToMatrix(inputSound, RawAudioMatrix, sampleNormaliser);

	// std::cout << RawAudioMatrix.toString() << "\n\n" << std::endl;

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
	if (outGammatoneFilteredPowerPort.getOutputCount()) {
		gammatoneFilterBank->getGammatoneFilteredPower (
			/* Basilar Memb Res  = */ GammatoneFilteredAudioMatrix,
			/* Power of the Bank = */ GammatoneFilteredPowerMatrix
		);
	}


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute a sliding window
	 *    over the filter bank. 
	 * =========================================================================== */
	if (outGammatoneFilteredWindowPort.getOutputCount()) {
		AudioUtil::WindowMatrix (
			/* Source = */ GammatoneFilteredAudioMatrix,
			/* Target = */ GammatoneFilteredWindowMatrix,
			/* Window = */ windowLength,
			/* Hop    = */ hopLength,
			/* Method = */ windowMethod
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
	 *  OPTIONAL: If a port is connected, compute a sliding window
	 *    over the beamformed audio. 
	 * =========================================================================== */
	if (outBeamformedWindowPort.getOutputCount()) {
		AudioUtil::WindowMatrix (
			/* Source = */ BeamformedAudioMatrix,
			/* Target = */ BeamformedWindowMatrix,
			/* Window = */ windowLength,
			/* Hop    = */ hopLength,
			/* Method = */ windowMethod
		);
	}


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute the 
	 *    RMS power of the beamformed bands.
	 * =========================================================================== */
	if (outBeamformedRmsAudioPort.getOutputCount() || outBeamformedRmsPowerPort.getOutputCount() || outAllocentricAudioPort.getOutputCount()) {
		
		AudioUtil::RootMeanSquareMatrix (
			/* Source = */ BeamformedAudioMatrix,
			/* Target = */ BeamformedRmsAudioMatrix,
			/* Dim X  = */ numBands,
			/* Dim Y  = */ numBeams
		);

		if (outBeamformedRmsPowerPort.getOutputCount()) {
			beamformer->getBeamformedRmsPower (
				/* RMS Beamformer = */ BeamformedRmsAudioMatrix,
				/* Power of Beams = */ BeamformedRmsPowerMatrix
			);
		}

		if (outAllocentricAudioPort.getOutputCount()) {
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

	if (!computeEnvelope) {
		//-- Leave the processing loop early to avoid processing hilbert transform.
		return true;
	}

	/* ===========================================================================
	 *  Down sample the matrix by some factor to make the computation faster.
	 * =========================================================================== */
	AudioUtil::downSampleMatrix(
		/* Source = */ BeamformedAudioMatrix,
		/* Target = */ BeamformedDownSampAudioMatrix,
		/* Reduce = */ downSampEnv,
		/* Method = */ downSampMethod
	);


	/* ===========================================================================
	 *  Compute the hilbert envelope.
	 * =========================================================================== */
	hilbertTransform->getHilbertEnvelope (
		/* Source = */ BeamformedDownSampAudioMatrix,
		/* Target = */ HilbertEnvelopeMatrix
	);


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute a sliding window
	 *    over the hilbert transformed audio. 
	 * =========================================================================== */
	if (outHilbertEnvelopeWindowPort.getOutputCount()) {
		AudioUtil::WindowMatrix (
			/* Source = */ HilbertEnvelopeMatrix,
			/* Target = */ HilbertEnvelopeWindowMatrix,
			/* Window = */ windowLength,
			/* Hop    = */ hopLength,
			/* Method = */ windowMethod
		);
	}


	/* ===========================================================================
	 *  Isolate for the specified envelope (typically 5 Hz).
	 * =========================================================================== */
	butterworthFilter->getBandPassedAudio (
		/* Source = */ HilbertEnvelopeMatrix,
		/* Target = */ BandPassedEnvelopeMatrix,
		/* CFreq  = */ bandPassFreq
	);


	/* ===========================================================================
	 *  OPTIONAL: If a port is connected, compute a sliding window
	 *    over the hilbert transformed audio. 
	 * =========================================================================== */
	if (outBandPassedEnvelopeWindowPort.getOutputCount()) {
		AudioUtil::WindowMatrix (
			/* Source = */ BandPassedEnvelopeMatrix,
			/* Target = */ BandPassedEnvelopeWindowMatrix,
			/* Window = */ windowLength,
			/* Hop    = */ hopLength,
			/* Method = */ windowMethod
		);
	}


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
	if (outBandPassedRmsPowerPort.getOutputCount()) {
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
		AudioUtil::downSampleMatrix(GammatoneFilteredAudioMatrix, outGammatoneFilteredAudioPort.prepare(), downSampOut); //, downSampMethod);
		outGammatoneFilteredAudioPort.setEnvelope(timeStamp);
		outGammatoneFilteredAudioPort.write();

	}

	if (outGammatoneFilteredPowerPort.getOutputCount()) {

		outGammatoneFilteredPowerPort.prepare() = GammatoneFilteredPowerMatrix;
		outGammatoneFilteredPowerPort.setEnvelope(timeStamp);
		outGammatoneFilteredPowerPort.write();

	}

	if (outGammatoneFilteredWindowPort.getOutputCount()) {

		outGammatoneFilteredWindowPort.prepare() = GammatoneFilteredWindowMatrix;
		outGammatoneFilteredWindowPort.setEnvelope(timeStamp);
		outGammatoneFilteredWindowPort.write();
	}

	if (outBeamformedAudioPort.getOutputCount()) {
		
		//-- This Matrix can be very big. Down sample if enabled.
		AudioUtil::downSampleMatrix(BeamformedAudioMatrix, outBeamformedAudioPort.prepare(), downSampOut); //, downSampMethod);
		outBeamformedAudioPort.setEnvelope(timeStamp);
		outBeamformedAudioPort.write();

	}

	if (outBeamformedWindowPort.getOutputCount()) {

		outBeamformedWindowPort.prepare() = BeamformedWindowMatrix;
		outBeamformedWindowPort.setEnvelope(timeStamp);
		outBeamformedWindowPort.write();
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
		AudioUtil::downSampleMatrix(HilbertEnvelopeMatrix, outHilbertEnvelopePort.prepare(), downSampOut); //, downSampMethod);
		outHilbertEnvelopePort.setEnvelope(timeStamp);
		outHilbertEnvelopePort.write();

	}

	if (outHilbertEnvelopeWindowPort.getOutputCount()) {

		outHilbertEnvelopeWindowPort.prepare() = HilbertEnvelopeWindowMatrix;
		outHilbertEnvelopeWindowPort.setEnvelope(timeStamp);
		outHilbertEnvelopeWindowPort.write();
	}

	if (outBandPassedEnvelopePort.getOutputCount()) {

		//-- This Matrix can be very big. Down sample if enabled.
		AudioUtil::downSampleMatrix(BandPassedEnvelopeMatrix, outBandPassedEnvelopePort.prepare(), downSampOut); //, downSampMethod);
		outBandPassedEnvelopePort.setEnvelope(timeStamp);
		outBandPassedEnvelopePort.write();

	}

	if (outBandPassedEnvelopeWindowPort.getOutputCount()) {

		outBandPassedEnvelopeWindowPort.prepare() = BandPassedEnvelopeWindowMatrix;
		outBandPassedEnvelopeWindowPort.setEnvelope(timeStamp);
		outBandPassedEnvelopeWindowPort.write();
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
