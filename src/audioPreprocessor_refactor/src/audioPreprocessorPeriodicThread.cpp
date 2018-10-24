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

#define THPERIOD 0.1 // seconds.

AudioPreprocessorPeriodicThread::AudioPreprocessorPeriodicThread() : 
	PeriodicThread(THPERIOD) {

	robot = "icub";        
}


AudioPreprocessorPeriodicThread::AudioPreprocessorPeriodicThread(std::string _robot, std::string _configFile) : 
	PeriodicThread(THPERIOD) {

	robot = _robot;
	configFile = _configFile;
}


AudioPreprocessorPeriodicThread::~AudioPreprocessorPeriodicThread() {
	delete gammatoneFilterBank;
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

	numBands  = rf.findGroup("processing").check("numBands",  yarp::os::Value(128),   "number of frequency bands (int)"              ).asInt();
	lowCf     = rf.findGroup("processing").check("lowCf",     yarp::os::Value(380),   "lowest center frequency (int)"                ).asInt();
	highCf    = rf.findGroup("processing").check("highCf",    yarp::os::Value(2800),  "highest center frequency (int)"               ).asInt();
	halfRec   = rf.findGroup("processing").check("halfRec",   yarp::os::Value(false), "half wave rectifying (boolean)"               ).asBool();
	erbSpaced = rf.findGroup("processing").check("erbSpaced", yarp::os::Value(true),  "ERB spaced centre frequencies (boolean)"      ).asBool();
	degreeRes = rf.findGroup("processing").check("degreeRes", yarp::os::Value(1),     "degree resolution for a single position (int)").asInt();
	
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


	/* =========================================================================== 
	 *  Print the resulting variables to the console.
	 * =========================================================================== */
	yInfo("\n\t               [ROBOT SPECIFIC]               "                                                   );
	yInfo(  "\t ============================================ "                                                   );
	yInfo(  "\t Index of Pan Joint            : %d", panAngle                            );
	yInfo(  "\t Number of Microphones         : %d", numMics                             );
	yInfo(  "\t Microphone Distance           : %f", micDistance                         );
	yInfo("\n\t                  [SAMPLING]                  "                                                      );
	yInfo(  "\t ============================================ "                                                   );
	yInfo(  "\t Speed of Sound                : %f", speedOfSound                        );
	yInfo(  "\t Sampling Rate                 : %d", samplingRate                        );
	yInfo(  "\t Number of Frames Samples      : %d", numFrameSamples                     );
	yInfo("\n\t                 [PROCESSING]                 "                                                     );
	yInfo(  "\t ============================================ "                                                   );
	yInfo(  "\t Number of Frequency Bands     : %d", numBands                            );
	yInfo(  "\t Lowest Center Frequency       : %d", lowCf                               );
	yInfo(  "\t Highest Center Frequency      : %d", highCf                              );
	yInfo(  "\t Half-Wave Rectifying          : %s", halfRec   ? "ENABLED"  : "DISABLED" );
	yInfo(  "\t Center Frequency Spacing      : %s", erbSpaced ? "ERB-Rate" : "Linear"   );
	yInfo(  "\t Number of Beams Per Hemifield : %d", numBeamsPerHemifield                );
	yInfo(  "\t Number of Front Field Beams   : %d", numBeams                            );

	/* ===========================================================================
	 *  Initialize the matrices used for data processing.
	 * =========================================================================== */
	RawAudioMatrix.resize(numMics, numFrameSamples);
	RawAudioMatrix.zero();

	GammatoneFilteredAudioMatrix.resize(numMics * numBands, numFrameSamples);
	GammatoneFilteredAudioMatrix.zero();

	GammatoneFilteredPowerMatrix.resize(numBands, numMics);
	GammatoneFilteredPowerMatrix.zero();

	BeamformedAudioMatrix.resize(numBeams * numBands, numFrameSamples);
	BeamformedAudioMatrix.zero();

	BeamformedRmsAudioMatrix.resize(numBands, numBeams);
	BeamformedRmsAudioMatrix.zero();

	BeamformedRmsPowerMatrix.resize(numBands, 1);
	BeamformedRmsPowerMatrix.zero();

	/* ===========================================================================
	 *  Initialize the processing objects.
	 * =========================================================================== */
	gammatoneFilterBank = new GammatoneFilterBank(numMics, samplingRate, numFrameSamples, numBands, lowCf, highCf, halfRec, erbSpaced);
	interauralCues      = new InterauralCues();

	return true;
}


bool AudioPreprocessorPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *   let RFModule know initialization was unsuccessful.
	 * =========================================================================== */
	if (!inRawAudioPort.open(getName("/rawAudio:i").c_str())) {
		yError("Unable to open port for receiving raw audio from head.");
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

	if (!outBeamformedRmsPowerPort.open(getName("/beamformedRmsPowerPort:o").c_str())) {
		yError("Unable to open port for sending the power of bands of the root mean square of beamformed audio.");
		return false;
	}

	yInfo("Initialization of the processing thread correctly ended.");

	return true;
}


void AudioPreprocessorPeriodicThread::threadRelease() {

	//-- Stop all threads.
	inRawAudioPort.interrupt();
	outGammatoneFilteredAudioPort.interrupt();
	outGammatoneFilteredPowerPort.interrupt();
	outBeamformedAudioPort.interrupt();
	outBeamformedRmsAudioPort.interrupt();
	outBeamformedRmsPowerPort.interrupt();

	//-- Close the threads.
	inRawAudioPort.close();
	outGammatoneFilteredAudioPort.close();
	outGammatoneFilteredPowerPort.close();
	outBeamformedAudioPort.close();
	outBeamformedRmsAudioPort.close();
	outBeamformedRmsPowerPort.close();

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
		
		inputSound = inRawAudioPort.read(true);
		inRawAudioPort.getEnvelope(timeStamp);
		result = processing();

		//-- TODO: This may be better moved into processing loop.
		if (outGammatoneFilteredAudioPort.getOutputCount()) {
			outGammatoneFilteredAudioPort.prepare() = GammatoneFilteredAudioMatrix;
			outGammatoneFilteredAudioPort.write();
		}

		if (outGammatoneFilteredPowerPort.getOutputCount()) {
			outGammatoneFilteredPowerPort.prepare() = GammatoneFilteredPowerMatrix;
			outGammatoneFilteredPowerPort.write();
		}

	}



	//if (outputPort.getOutputCount()) {
	//	*outputImage = outputPort.prepare();
	//	outputImage->resize(inputImage->width(), inputImage->height());
	//	outputPort.write();
	//}
}


bool AudioPreprocessorPeriodicThread::processing() {
	
	//-- Separate the sound into left and right channels.	
	for (int sample = 0; sample < numFrameSamples; sample++) {
		for (int channel = 0; channel < numMics; channel++) {
			RawAudioMatrix[channel][sample] = inputSound->get(sample, channel) / _norm;
		}
	}

	/* ===========================================================================
	 *  Apply a 4th order gammatone filter onto the
	 *   raw audio for the specified number of bands.
	 * =========================================================================== */
	gammatoneFilterBank->getGammatoneFilteredAudio(
		GammatoneFilteredAudioMatrix,  //-- Target.
		RawAudioMatrix                 //-- Source.
	);

	//-- If a port is connected, compute the RMS
	//--  power of the filter banks beams.
	if (outGammatoneFilteredPowerPort.getOutputCount()) {
		gammatoneFilterBank->getGammatoneFilteredPower(
			GammatoneFilteredPowerMatrix,
			GammatoneFilteredAudioMatrix
		);
	}

	
	


	return true;
}


