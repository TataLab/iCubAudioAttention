// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Marko Ilievski
  * email: m.ilievski@uleth.ca, matthew.tata@uleth.ca, francesco.rea@iit.it
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

/**
 * @file  audioPreprocesserRatethread.cpp
 * @brief Implementation of the processing ratethread.
 *        This is where the processing happens.
 */

#include "audioPreprocesserRatethread.h"

using namespace yarp::os;

#define THRATE 80 //ms


AudioPreprocesserRatethread::AudioPreprocesserRatethread() : RateThread(THRATE) {
	robot = "icub";
}


AudioPreprocesserRatethread::AudioPreprocesserRatethread(std::string _robot, std::string _configFile, yarp::os::ResourceFinder &rf) : RateThread(THRATE) {
	robot = _robot;
	configFile = _configFile;
	loadFile(rf);
}


AudioPreprocesserRatethread::~AudioPreprocesserRatethread() {
	delete inPort;
	delete outGammaToneAudioPort;
	delete outGammaTonePowerAudioPort;
	delete outBeamFormedAudioPort;
	delete outReducedBeamFormedAudioPort;
	delete outBeamFormedPowerAudioPort;
	delete outLowResolutionAudioMapPort;
	delete outAudioMapEgoPort;
	delete outGammatoneFilterVisPort;

	delete outAudioMap;
	delete outGammaToneFilteredAudioMap;
	delete outGammaTonePowerAudioMap;
	delete outBeamFormedAudioMap;
	delete outReducedBeamFormedAudioMap;
	delete outBeamFormedPowerAudioMap;

	delete gammatoneAudioFilter;
	delete beamForm;

	delete[] rawAudio;
}


bool AudioPreprocesserRatethread::threadInit() {

	// input port for receiving raw audio
	inPort = new yarp::os::BufferedPort<yarp::sig::Sound>();
	if (!inPort->open("/iCubAudioAttention/AudioPreprocesser:i")) {
		yError("unable to open port to receive input");
		return false;
	}


	// output port for sending GammaTone Filtered Audio
	outGammaToneAudioPort = new yarp::os::Port();
	if (!outGammaToneAudioPort->open("/iCubAudioAttention/GammaToneFilteredAudio:o")) {
		yError("unable to open port to send Gammatone Filtered Audio");
		return false;
	}


	// output port for sending the power of GammaTone Filtered Audio
	outGammaTonePowerAudioPort = new yarp::os::Port();
	if (!outGammaTonePowerAudioPort->open("/iCubAudioAttention/GammaTonePowerAudio:o")) {
		yError("unable to open port to send Power of Gammatone Filtered Audio");
		return false;
	}

	outGammatoneFilterVisPort = new yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >();
	if (!outGammatoneFilterVisPort->open("/iCubAudioAttention/GammatoneVis/img:o")) { 
		yError("unable to open port to send visualization of the Gammatone Filtered Audio");
		return false;
	}


	// output port for sending BeamFormed Audio
	outBeamFormedAudioPort = new yarp::os::Port();
	if (!outBeamFormedAudioPort->open("/iCubAudioAttention/BeamFormedAudio:o")) {
		yError("unable to open port to send Beamformed Audio");
		return false;
	}


	// output port for sending BeamFormed Audio
	outReducedBeamFormedAudioPort = new yarp::os::Port();
	if (!outReducedBeamFormedAudioPort->open("/iCubAudioAttention/ReducedBeamFormedAudio:o")) {
		yError("unable to open port to send Reduced Beamformed Audio");
		return false;
	}


	// output port for sending the power of the BeamFormed Audio
	//outBeamFormedPowerAudioPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
	outBeamFormedPowerAudioPort = new yarp::os::Port();
	if (!outBeamFormedPowerAudioPort->open("/iCubAudioAttention/BeamFormedPowerAudio:o")) {
		yError("unable to open port to send the power of the BeamFormed Audio");
		return false;
	}

	outLowResolutionAudioMapPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
	if (!outLowResolutionAudioMapPort->open("/iCubAudioAttention/LowResolutionAudioMap:o")) {
		yError("unable to open port to send the low resolution audio map");
		return false;
	}


	// output port for sending the Audio Map
	outAudioMapEgoPort = new yarp::os::Port();
	if (!outAudioMapEgoPort->open("/iCubAudioAttention/AudioMapEgo:o")){
		yError("unable to open port to send Audio Map");
		return false;
	}


	// error checking
	//if (yarp::os::Network::exists("/iCubAudioAttention/AudioPreprocesser:i")) {
	//	if (yarp::os::Network::connect("/sender", "/iCubAudioAttention/AudioPreprocesser:i") == false) {
	//		yError("Could not make connection to /sender. \nExiting. \n");
	//		return false;
	//	}
	//} else {
	//	// inPort failed to open. quit.
	//	return false;
	//}

	// prepare GammatoneFilter object
	//gammatoneAudioFilter = new GammatoneFilter(samplingRate, lowCf, highCf, nBands, frameSamples, nMics, false);
	gammatoneAudioFilter = new GammatoneFilter(samplingRate, lowCf, highCf, nBands, frameSamples, nMics, true);

	// prepare BeamFormer object
	beamForm = new BeamFormer(nBands, frameSamples, nMics, nBeamsPerHemi);

	// prepare other memory structures
	rawAudio = new float[(frameSamples * nMics)];

	egoSpaceMap.resize(nBands, 360);

	//-- Find the Spacing of the beams, for proper interpolation.
	setAngleSpacing();

	//--
	//-- TODO: Change if works.
	//--
	setSampleDelay();

	//-- Allocate space for the low resolution audio map,
	//-- then clear it to all zeros.
	lowResolutionAudioMap.resize(nBands, nMicAngles);
	lowResolutionAudioMap.zero();


	// initialize a two dimentianl vector that
	// is 2 * interpolateNSamples x nBands
	for (int i = 0; i < interpolateNSamples * 2; i++) {

		std::vector<double> tempvector(nBands, 0);

		highResolutionAudioMap.push_back(tempvector);
	}

	// construct a yarp Matrix for sending an Audio Map
	outAudioMap = new yarp::sig::Matrix(nBands, interpolateNSamples * 2);

	//TODO: comeback
	// construct a yarp Matrix for sending GammaTone Filtered Audio
	outGammaToneFilteredAudioMap = new yarp::sig::Matrix(nBands*2, frameSamples);

	// construct a yarp Matrix for sending the Power of GammaTone Filtered Audio.
	outGammaTonePowerAudioMap = new yarp::sig::Matrix(1, nBands);

	// construct a yarp Matrix for sending Beam Formed Audio
  	outBeamFormedAudioMap = new yarp::sig::Matrix(nBeams * nBands, frameSamples);

  	// construct a yarp Matrix for sending Reduced Beam Formed Audio
  	outReducedBeamFormedAudioMap = new yarp::sig::Matrix(nBands, nBeams);

	// construct a yarp Matrix for sending the power of the Beam Formed Audio
	outBeamFormedPowerAudioMap = new yarp::sig::Matrix(1, nBands);

	// initializing completed successfully
	yInfo("Initialization of the processing thread correctly ended");

	return true;
}


void AudioPreprocesserRatethread::setName(std::string str) {
	this->name=str;
}


std::string AudioPreprocesserRatethread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void AudioPreprocesserRatethread::setInputPortName(std::string InpPort) {

}


void AudioPreprocesserRatethread::run() {

	// read in raw audio
	s = inPort->read(true);

	// set ts to be the envelope
	// count for the inPort
	inPort->getEnvelope(ts);

	// initialize rawAudio to be usable
	for (int col = 0 ; col < frameSamples; col++) {
		for (int micLoop = 0; micLoop < nMics; micLoop++) {
            rawAudio[col*nMics + micLoop] = s->get(col, micLoop) / normDivid; //double check casting
        }
    }

	// run the Filter Bank on the raw Audio
	gammatoneAudioFilter->gammatoneFilterBank(rawAudio);

	if (outGammaToneAudioPort->getOutputCount()) {
		// format the gammatone filtered audio into
		// a sendable format, set the envelope,
		// then publish to the network
		sendGammatoneFilteredAudio(gammatoneAudioFilter->getFilteredAudio());
	}
	
	if (outGammaTonePowerAudioPort->getOutputCount()) {
		// format the power of gammatone filtered audio into 
		// a sendable format, set the envelope,
		// then publish to the network.
		sendGammatonePowerAudio(gammatoneAudioFilter->getPowerAudio());
	}

	if (outGammatoneFilterVisPort->getOutputCount()) {
		sendGammatoneFilteredAudioVis(gammatoneAudioFilter->getFilteredAudio());
	}






	// set the beamformers audio to be the filtered audio
	beamForm->inputAudio(gammatoneAudioFilter->getFilteredAudio());

	if (outBeamFormedAudioPort->getOutputCount()) {
        // run the beamformer on the set audio
	    //beamFormedAudioVector = beamForm->getBeamAudio();

		// format the beam formed audio into
		// a sendable format, set the envelope,
		// then publish to the network
		//sendBeamFormedAudio(beamFormedAudioVector);
	}

	// run the reduced-beamformer on the set audio
	reducedBeamFormedAudioVector = beamForm->getReducedBeamAudio();

	if (outReducedBeamFormedAudioPort->getOutputCount()) {
		// format the reduced beam formed audio into
		// a sendable format, set the envelope,
		// then publish to the network
		sendReducedBeamFormedAudio(reducedBeamFormedAudioVector);
	}

	if (outBeamFormedPowerAudioPort->getOutputCount()) {
		// format the power of the beam formed audio into 
		// a sendable format, set the envelope,
		// then publish to the network.
		sendBeamFormedPowerAudio(beamForm->getPowerAudio());
	}

	//setLowResolutionMap();


	//if (outLowResolutionAudioMapPort->getOutputCount()) {
	//	sendLowResolutionAudioMap();
	//}


	// do an interpolate on the lowResolutionAudioMap
	// to produce a highResolutionAudioMap
	linearInterpolate();
    //splineInterpolate();

	if (outAudioMapEgoPort->getOutputCount()) {
		// format the highResolutionAudioMap into
		// a sendable format, set the envelope,
		// then publish to the network
		sendAudioMap();
	}

	// timing how long the module took
	lastframe = ts.getCount();
	stopTime=yarp::os::Time::now();
	yInfo("elapsed time = %f\n",stopTime-startTime);
	startTime=stopTime;
}


bool AudioPreprocesserRatethread::processing() {
	// here goes the processing...
	return true;
}


void AudioPreprocesserRatethread::threadRelease() {
	// stop all ports
	inPort->interrupt();
	outGammaToneAudioPort->interrupt();
	outGammaTonePowerAudioPort->interrupt();
	outBeamFormedAudioPort->interrupt();
	outReducedBeamFormedAudioPort->interrupt();
	outBeamFormedPowerAudioPort->interrupt();
	outLowResolutionAudioMapPort->interrupt();
	outAudioMapEgoPort->interrupt();

	// release all ports
	inPort->close();
	outGammaToneAudioPort->close();
	outGammaTonePowerAudioPort->close();
	outBeamFormedAudioPort->close();
	outReducedBeamFormedAudioPort->close();
	outBeamFormedPowerAudioPort->close();
	outLowResolutionAudioMapPort->close();
	outAudioMapEgoPort->close();
}


void AudioPreprocesserRatethread::loadFile(yarp::os::ResourceFinder &rf) {

	// import all relevant data fron the .ini file
	yInfo("Loading Configuration File.");
	try {
		C            = rf.findGroup("sampling").check("C",            Value(336.628), "C speed of sound (double)").asDouble();
		nMics        = rf.findGroup("sampling").check("nMics",        Value(2),       "number mics (int)").asInt();
		micDistance  = rf.findGroup("sampling").check("micDistance",  Value(0.145),   "micDistance (double)").asDouble();
		frameSamples = rf.findGroup("sampling").check("frameSamples", Value(4096),    "frame samples (int)").asInt();
		samplingRate = rf.findGroup("sampling").check("samplingRate", Value(48000),   "sampling rate of mics (int)").asInt();
		
		nBands              = rf.findGroup("preprocessing").check("nBands",              Value(32),   "numberBands (int)").asInt();
		lowCf               = rf.findGroup("preprocessing").check("lowCf",               Value(80),   "lowest center frequency (int)").asInt();
		highCf              = rf.findGroup("preprocessing").check("highCf",              Value(8000), "highest center frequency (int)").asInt();
		interpolateNSamples = rf.findGroup("preprocessing").check("interpolateNSamples", Value(180),  "interpellate N samples (int)").asInt();
		radialRes_degrees   = rf.findGroup("preprocessing").check("radialRes_degrees",   Value(1),    "Degrees Per Possible Head Direction (int)").asInt();


		//-- Take the ceiling of of (D/C)/Rate.
		//--   ceiling = (x + y - 1) / y
		nBeamsPerHemi     = ((micDistance * samplingRate) + C - 1.0) / C;
		//nBeamsPerHemi     = int((micDistance / C) * samplingRate) - 1;
		nBeams            = 2 * nBeamsPerHemi + 1;
	    nMicAngles        = nBeams * 2 - 2;
		radialRes_radians = _pi / 180.0 * radialRes_degrees;
		nSpaceAngles      = 360 / radialRes_degrees;
		nNormalAngles     = nSpaceAngles / 2;

		//-- Print information from rf to the console.
		yInfo("\t nMics                           = %d", nMics);
		yInfo("\t micDistance                     = %f", micDistance);
		yInfo("\t frameSamples                    = %d", frameSamples);
		yInfo("\t nBands                          = %d", nBands);
    	yInfo("\t low Cutting frequency           = %d", lowCf);
    	yInfo("\t high Cutting frequency          = %d", highCf);
		yInfo("\t Num Beams Per Hemifield      %d = ceil(%f / %f * %d)", nBeamsPerHemi, micDistance, C, samplingRate);
		yInfo("\t Total Num Beams                 = %d", nBeams);
		yInfo("\t nMicAngles                      = %d", nMicAngles);
		yInfo("\t interpolateNSamples             = %d", interpolateNSamples );
		yInfo("\t Radial Resolution (Degrees)     = %d", radialRes_degrees);
		yInfo("\t Radial Resolution (Radians)     = %f", radialRes_radians);
		yInfo("\t Num Space Angles                = %d", nSpaceAngles);
		yInfo("\t Num Normal Angles               = %d", nNormalAngles);
	}

	catch (int a) {
		yError("Error in the loading of file");
	}

	yInfo("file successfully load");
}


void AudioPreprocesserRatethread::sendGammatoneFilteredAudio(const std::vector<float*> &gammatoneAudio) {
    for (int channel = 0; channel < nMics; channel++) {
	    // fill the yarp Matrix with gammatone filtered audio
	    for (int i = 0; i < nBands; i++) {
		    yarp::sig::Vector tempV(frameSamples); //this is potentially requiring loads of time
		    for (int j = 0; j < frameSamples; j++) {
			    tempV[j] = gammatoneAudio[i + channel * nBands][j];
		    }
		    outGammaToneFilteredAudioMap->setRow(i + channel * nBands, tempV);
	    }
    }

	// set the envelope for the Gammatone Filtered Audio port
	outGammaToneAudioPort->setEnvelope(ts);
    //yDebug("sending matrix %d %d", outGammaToneFilteredAudioMap->rows(), outGammaToneFilteredAudioMap->cols());
	// publish the map onto the network
	outGammaToneAudioPort->write(*outGammaToneFilteredAudioMap);
}


void AudioPreprocesserRatethread::sendGammatonePowerAudio(const std::vector<float> &gammatonePower) {
	
	//-- Fill the yarp matrix with the power of gammatone filtered audio.
	yarp::sig::Vector tempV(nBands);

	for (int band = 0; band < nBands; band++) {
		tempV[band] = gammatonePower[band];
	}

	outGammaTonePowerAudioMap->setRow(0, tempV);

	//-- Set the envelope for the Gammatone Power Audio Port.
	outGammaTonePowerAudioPort->setEnvelope(ts);

	//-- Publish the map onto the yarp network.
	outGammaTonePowerAudioPort->write(*outGammaTonePowerAudioMap);
}


void AudioPreprocesserRatethread::sendGammatoneFilteredAudioVis(const std::vector<float*> &gammatoneAudio) {

	yarp::sig::ImageOf<yarp::sig::PixelRgb> &img = outGammatoneFilterVisPort->prepare();
	img.resize(frameSamples, nBands);

	unsigned char* pImage = img.getRawImage();
	int padding = img.getPadding();
	
	float value = 0.f;
	int currentBand = 0;

	for (int band = 0; band < nBands; band++) {
		for (int frame = 0; frame < frameSamples; frame++) {
			
			for (int pos = 0; pos < 3; pos++) {
				
				if (pos == 2) {
					//-- First Half Channel. Blue
                    currentBand = band;
				} else if (pos == 1) {
					//-- Second Half Channel. Green.
					currentBand = band + nBands;
				} else { 
                    //--- Ignore Red Channel.
                    pImage++; continue;

				}

				//-- Grab the current value from the filtered audio.
				value = gammatoneAudio[currentBand][frame];

				if (value <= 0.f) {
					//-- Ignore values below zero.
					value = 0.f;
				}

				//-- Normalize the values in range of 1.
				value /= 1.26f;

				//-- Set the color value.
				*pImage = value * 255.f;

				//-- Move to next pixel.
				pImage++;
			}
		}
		pImage += padding;
	}

	outGammatoneFilterVisPort->setEnvelope(ts);
	outGammatoneFilterVisPort->write();
}


void AudioPreprocesserRatethread::sendBeamFormedAudio(const std::vector<std::vector<std::vector<float> > > &beamFormedAudio) {

	// fill the yarp Matrix with uncompressed beamformed audio
    for (int beam = 0; beam < nBeams; beam++) {
        for (int band = 0; band < nBands; band++) {
            yarp::sig::Vector tempV(frameSamples);
            for (int frame = 0; frame < frameSamples; frame++) {

                tempV[frame] = beamFormedAudio[beam][band][frame];
            }

        	outBeamFormedAudioMap->setRow(band + beam * nBands, tempV);
        }
    }

    /*
	for (int i = 0; i < nBands; i++) {
		yarp::sig::Vector tempV(nBeams);
		for (int j = 0; j < nBeams; j++) {
			tempV[j] = beamFormedAudio[i][j];
		}
		outBeamFormedAudioMap->setRow(i, tempV);
	}
    */
    //yDebug("after sending matrix %d %d", outBeamFormedAudioMap->rows(), outBeamFormedAudioMap->cols());
	// set the envelope for the Beam Formed Audio port
	outBeamFormedAudioPort->setEnvelope(ts);

	// publish the map onto the network
	outBeamFormedAudioPort->write(*outBeamFormedAudioMap);
}


void AudioPreprocesserRatethread::sendReducedBeamFormedAudio(const std::vector<std::vector<double> > &reducedBeamFormedAudio) {

    //yDebug("size of reduced beam former %d %d \n", reducedBeamFormedAudio.size(), reducedBeamFormedAudio[0].size());

    /********************************************************************
	// fill the yarp Matrix with reduced beamformed audio
	for (int i = 0; i < nBeams; i++) {
		yarp::sig::Vector tempV(nBands);
		for (int j = 0; j < nBands; j++) {
			tempV[j] = reducedBeamFormedAudio[i][j];
		}
		outReducedBeamFormedAudioMap->setRow(i, tempV);
	}
    //********************************************************************/

    // fill the yarp Matrix with reduced beamformed audio
	for (int i = 0; i < nBands; i++) {
		yarp::sig::Vector tempV(nBeams);
		for (int j = 0; j < nBeams; j++) {
			tempV[j] = reducedBeamFormedAudio[i][j];
		}
		outReducedBeamFormedAudioMap->setRow(i, tempV);
	}

	// set the envelope for the Reduced Beam Formed Audio port
	outReducedBeamFormedAudioPort->setEnvelope(ts);

	// publish the map onto the network
	outReducedBeamFormedAudioPort->write(*outReducedBeamFormedAudioMap);
}


void AudioPreprocesserRatethread::sendBeamFormedPowerAudio(const std::vector< double > &beamFormedPower) {

	/*
	//-- Init a matrix to publish on.
	yarp::sig::Matrix& m = outBeamFormedPowerAudioPort->prepare();
	m.resize(1, nBands);

	//-- Fill the yarp matrix with the power of the beams formed audio.
	yarp::sig::Vector tempV(nBands);

	for (int band = 0; band < nBands; band++) {
		tempV[band] = beamFormedPower[band];
	}

	//-- Add this to the buffered port.
	m.setRow(0, tempV);

	//-- Set the envelope for the Beamformed Power Audio Port.
	outBeamFormedPowerAudioPort->setEnvelope(ts);

	//-- Publish the map onto the yarp network.
	outBeamFormedPowerAudioPort->write();
	*/

	//-- Fill the yarp matrix with the power of the beams formed audio.
	yarp::sig::Vector tempV(nBands);
	
	for (int band = 0; band < nBands; band++) {
		tempV[band] = beamFormedPower[band];
	}

	outBeamFormedPowerAudioMap->setRow(0, tempV);

	//-- Set the envelope for the Beamformed Power Audio Port.
	outBeamFormedPowerAudioPort->setEnvelope(ts);
	
	//-- Publish the map onto the yarp network.
	outBeamFormedPowerAudioPort->write(*outBeamFormedPowerAudioMap); 
}


void AudioPreprocesserRatethread::sendLowResolutionAudioMap() {

	//-- Init a matrix to publish on.
	yarp::sig::Matrix& m = outLowResolutionAudioMapPort->prepare();
	m.resize(nBands, nMicAngles);

	//-- Fill the yarp matrix with the power of the beams formed audio.
	yarp::sig::Vector tempV(nMicAngles);

	for (int band = 0; band < nBands; band++) {
		for (int angle = 0; angle < nMicAngles; angle++) {
			tempV[angle] = lowResolutionAudioMap[band][angle];
		}
		//-- Add this to the buffered port.
		m.setRow(band, tempV);
	}

	//-- Set the envelope for the Beamformed Power Audio Port.
	outLowResolutionAudioMapPort->setEnvelope(ts);

	//-- Publish the map onto the yarp network.
	outLowResolutionAudioMapPort->write();
}

void AudioPreprocesserRatethread::sendAudioMap() {

	// fill the yarp Matrix with interpolated beamformed audio
	yarp::sig::Vector tempV(interpolateNSamples * 2);

	for (int i = 0; i < nBands; i++) {	
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			tempV[j] = egoSpaceMap[i][j];
		}
		outAudioMap->setRow(i, tempV);
	}

	// set the envelope for the Audio Map port
	outAudioMapEgoPort->setEnvelope(ts);

	// publish the map onto the network
	outAudioMapEgoPort->write(*outAudioMap);
}


void AudioPreprocesserRatethread::frontFieldMirror(yarp::sig::Vector &target, const yarp::sig::Vector &source) {
	
	/*
	//-- Make sure space is allocated.
	target.resize(source.size() * 2, 0.0);

	//-- Get the length of the source vector.
	int full_length      = source.size();
	int half_length = full_length / 2;

	//-- Use this for continuous iteration.
	int current_position = 0;

	//-- Mirror First Quarter with Second.
	for (int index = full_length - half_length - 1; index > -1; index--) {
		target[current_position++] = source[index];
	}

	//-- Set Second and Third Quarter as normal.
	for (int index = 0; index < full_length; index++) {
		target[current_position++] = source[index];
	}

	//-- Mirror Fourth Quarter with Third.
	for (int index = full_length - 1; index > full_length - half_length - 1; index--) {
		target[current_position++] = source[index];
	}
	
	*/
	
	// Alternative Method for mirroring front field.

	//-- Make sure space is allocated.
	target.resize(source.size() * 2, 0.0);

	//-- Get the length of the source vector.
	int full_length  = source.size();
	int half_length  = full_length / 2;
	int two_and_half = full_length + full_length + half_length - 1;

	for (int index = 0; index < half_length; index++) {
		target[half_length + index]     = source[index];
		target[half_length - index - 1] = source[index];
	}

	for (int index = half_length; index < full_length; index++) {
		target[half_length  + index] = source[index];
		target[two_and_half - index] = source[index];
	}
	
}


void AudioPreprocesserRatethread::setAngleSpacing() {

	//-- Generate the angles at which each beam is pointed.
	angles.resize(nBeams, 0.0);
	for (int beam = 0; beam < nBeams; beam++) {
		angles[beam] = (1.0 / micDistance) * (-nBeamsPerHemi + beam) * (C / samplingRate);
		angles[beam] = (angles[beam] <= -1.0) ? -1.0 : angles[beam];  //-- Make sure we are in 
		angles[beam] = (angles[beam] >=  1.0) ?  1.0 : angles[beam];  //-- range to avoid NAN.
		angles[beam] = asin(angles[beam]);
	}


	//-- Generate a lineaer distribution of the angles in space.
	//double linspace_step = ((_pi - radialRes_radians) - (-_pi)) / (nSpaceAngles - 1.0);
	//double current_step  = -_pi;
	//
	//spaceAngles.resize(nSpaceAngles, 0.0);
	//
	//for (int angle = 0; angle < nSpaceAngles; angle++) {
	//	spaceAngles[angle] = current_step;
	//	current_step += linspace_step;
	//}

	//-- Generate a linear distribution of the angles in the front hemisphere.
	double linspace_step = (((_pi/ 2) - radialRes_radians) - (-_pi / 2)) / (180.0 - 1.0);
	double current_step  = (-_pi / 2);

	normalAngles.resize(180, 0.0);

	for (int angle = 0; angle < 180; angle++) {
		normalAngles[angle] = current_step; 
		current_step += linspace_step;
	}


	//-- Generate the non-linear distribution of where all beams are pointed.
	//int current_mic_pos = 0;
	//
	//micAngles.resize(nMicAngles, 0.0);
	//
	////-- First section is the last half of angles + pi.
	//for (int beam = nBeams-nBeamsPerHemi-1; beam < nBeams-1; beam++) {
	//	micAngles[current_mic_pos++] = angles[beam] + _pi;
	//}
	//
	////-- Second section is just the entire angles.
	//for (int beam = 0; beam < nBeams; beam++) {
	//	micAngles[current_mic_pos++] = angles[beam];
	//}
	//
	////-- Third section is the first half of angles + pi.
	//for (int beam = 1; beam < nBeams-nBeamsPerHemi-1; beam++) {
	//	micAngles[current_mic_pos++] = angles[beam] + _pi;
	//}
	//
	////-- Unwrap the micAngles by changing deltas between values to 2*pi complement.
	//for (int angle = 1; angle < nMicAngles; angle++) {		
	//	double delta = micAngles[angle] - micAngles[angle-1];
	//	delta = (delta > _pi) ? (delta - 2 * _pi) : ( (delta < -_pi) ? (delta + 2 * _pi) : (delta) );
	//	micAngles[angle] = micAngles[angle-1] + delta;
	//}
	//
	////-- Normalize all microphone positions to +/- pi.
	//for (int angle = 0; angle < nMicAngles; angle++) {
	//	micAngles[angle] -= (2 * _pi);
	//}
}


void AudioPreprocesserRatethread::setSampleDelay() {

	//-- TODO: Move the angle res to ini.
	int angle_resolution = 180;
	int startAngle_index =   0 * (angle_resolution / 180);
	int endAngle_index   = 180 * (angle_resolution / 180);

	//-- Allocate space.
	angle_index.resize(angle_resolution, 0.0);

	for (int angle = startAngle_index; angle < endAngle_index; angle++) {
		angle_index[angle] = 180.0 * angle / (angle_resolution-1) * M_PI / 180.0;
		angle_index[angle] = micDistance * samplingRate * (-cos(angle_index[angle])) / C;
		angle_index[angle] = myRound(angle_index[angle]);
		if (angle) { angle_index[angle] -= angle_index[startAngle_index]; }   //-- Normalize based on first index.
	} angle_index[startAngle_index] -= angle_index[startAngle_index];         //-- Adjust first index last.
}


void AudioPreprocesserRatethread::setLowResolutionMap() {

	for (int band = 0; band < nBands; band++) {

		int current_beam = 0;
		for (int beam = nBeams-nBeamsPerHemi-1; beam > 0; beam--) {
			lowResolutionAudioMap[band][current_beam++] = reducedBeamFormedAudioVector[band][beam];
		}

		for (int beam = 0; beam < nBeams; beam++) {
			lowResolutionAudioMap[band][current_beam++] = reducedBeamFormedAudioVector[band][beam];
		}

		for (int beam = nBeams-2; beam > nBeams-nBeamsPerHemi-1; beam--) {
			lowResolutionAudioMap[band][current_beam++] = reducedBeamFormedAudioVector[band][beam];
		}
	}
}



inline double AudioPreprocesserRatethread::linearApproximation(double x, double x1, double y1, double x2, double y2) {
	return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
}


void AudioPreprocesserRatethread::linearInterpolate() {

	/*  TODO : REMOVE THIS BLOCK.

	double offset = (interpolateNSamples / (double)nBeams);

	for (int i = 0; i < nBands; i++) {

		int k = 0;
		double curroffset = 0;

		// interpolation for the first half
		for (int j = 0; j < interpolateNSamples; j++) {

			if (j == (int)curroffset && k < (nBeams - 1)) {
				curroffset += offset;
				k++;
			}

			highResolutionAudioMap[j][i] = linerApproximation (	
				j,										// x
				curroffset - offset,					// x1
				reducedBeamFormedAudioVector[k-1][i],	// y1
				curroffset,								// x2
				reducedBeamFormedAudioVector[k][i]      // y2
			);	
		}

		curroffset = interpolateNSamples;

		// interpolation for the second half
		for (int j = interpolateNSamples; j < interpolateNSamples * 2; j++) {
			if (j == (int)curroffset && k > 1) {
				if (j != interpolateNSamples) {
					k--;
				}

				curroffset += offset;
			}

			highResolutionAudioMap[j][i] = linerApproximation (	
				j,										// x
				curroffset - offset,					// x1
				reducedBeamFormedAudioVector[k][i],		// y1
				curroffset,								// x2
				reducedBeamFormedAudioVector[k-1][i]    // y2
			);	
		}
	}
	*/ 

	/*
	for (int band = 0; band < nBands; band++) {
		
		int mAngle = 1;
		for (int sAngle = 0; sAngle < nSpaceAngles; sAngle++) {
			
			//-- If the offset angle is smaller than the normal angle, 
			//-- move to the next offset angle.
			if (spaceAngles[sAngle] > micAngles[mAngle] && mAngle < nMicAngles-1) {
				mAngle++;
			}

			//-- Interpolate for this position.
			highResolutionAudioMap[sAngle][band] = linearApproximation(
				spaceAngles[sAngle],
				micAngles[mAngle-1],
				lowResolutionAudioMap[band][mAngle-1],
				micAngles[mAngle],
				lowResolutionAudioMap[band][mAngle]
			);
		}
	}
	*/

	int idx0, idx1;
	yarp::sig::Vector source(180, 0.0);
	yarp::sig::Vector target(360, 0.0);

	for (int band = 0; band < nBands; band++) {

		for (int angle = 0; angle < 180; angle++) {
			
			if (normalAngles[angle] < angles[angle_index[angle]]) {
				idx0 = angle_index[angle] - 1;
				idx1 = angle_index[angle];
			} else {
				idx0 = angle_index[angle];
				idx1 = angle_index[angle] + 1;
			}

			source[angle] = linearApproximation (
				normalAngles[angle],
				angles[idx0],
				reducedBeamFormedAudioVector[band][idx0],
				angles[idx1],
				reducedBeamFormedAudioVector[band][idx1]
			);
		}

		frontFieldMirror(target, source);
		egoSpaceMap.setRow(band, target);
	}
}


double AudioPreprocesserRatethread::splineApproximation(double x, double x1, double y1, double x2, double y2, double x3, double y3) {

	// init
	double xs1, xs2, ys1, ys2, ks1, ks2;

	// generate the knot values
	knotValues k = calcSplineKnots(x1, y1, x2, y2, x3, y3);

	// determine which side
	// of the center x is
	if (x > x2) {
		xs1 = x2; ys1 = y2; ks1 = k.k1;
		xs2 = x3; ys2 = y3; ks2 = k.k2;
	}

	else        {
		xs1 = x1; ys1 = y1; ks1 = k.k0;
		xs2 = x2; ys2 = y2; ks2 = k.k1;
	}

	// calculate the value of the y
	double t = (x - xs1) / (xs2 - xs1);
	double a =  ks1 * (xs2 - xs1) - (ys2 - ys1);
	double b = -ks2 * (xs2 - xs1) + (ys2 - ys1);
	double y = (1 - t) * ys1 + t * ys2 + t * (1 - t) * (a * (1 - t) + b * t);
	return y;
}


knotValues AudioPreprocesserRatethread::calcSplineKnots(double x1, double y1, double x2, double y2, double x3, double y3) {

	// init
	int matSize = 3;
	yarp::sig::Matrix a(matSize,matSize), aI(matSize,matSize);
	yarp::sig::Vector b(matSize);

	// get the tridiagonal linear matrix a
	a[0][0] = 2 / (x2 - x1);
	a[0][1] = 1 / (x2 - x1);
	a[0][2] = 0;
	a[1][0] = 1 / (x2 - x1);
	a[1][1] = 2 * ( (1 / (x2 - x1)) + (1 / (x3 - x2)) );
	a[1][2] = 1 / (x3 - x2);
	a[2][0] = 0;
	a[2][1] = 1 / (x3 - x2);
	a[2][2] = 2 / (x3 - x2);

	// get the inverse of matrix a
	aI = yarp::math::luinv(a);

	// get matrix b
	b[0] = 3 * ( (y2 - y1) / ( (x2 - x1) * (x2 - x1) ) );
	b[1] = 3 * ( (y2 - y1) / ( (x2 - x1) * (x2 - x1) )
	           + (y3 - y2) / ( (x3 - x2) * (x3 - x2) ) );
	b[2] = 3 * ( (y3 - y2) / ( (x3 - x2) * (x3 - x2) ) );

	// matrix ks being the knot values for
	// the spline which is matrix aI * b
	knotValues knots;
	knots.k0 = ( aI[0][0] * b[0] ) + ( aI[0][1] * b[1] ) + ( aI[0][2] * b[2] );
	knots.k1 = ( aI[1][0] * b[0] ) + ( aI[1][1] * b[1] ) + ( aI[1][2] * b[2] );
	knots.k2 = ( aI[2][0] * b[0] ) + ( aI[2][1] * b[1] ) + ( aI[2][2] * b[2] );

	return knots;
}


void AudioPreprocesserRatethread::splineInterpolate() {

	double offset = (interpolateNSamples / (double)nBeams);

	for (int i = 0; i < nBands; i++) {

		int k = 0;
		double curroffset = 0;

		// interpolation for the first half
		for (int j = 0; j < interpolateNSamples; j++) {
			if (j == (int)curroffset && k < (nBeams - 1)) {
				curroffset += offset;
				k++;
			}

			highResolutionAudioMap[j][i] = splineApproximation(	j,										// x
																curroffset - offset,					// x1
																reducedBeamFormedAudioVector[k-1][i],	// y1
																curroffset,								// x2
																reducedBeamFormedAudioVector[k][i],		// y2
																curroffset + offset,					// x3
																reducedBeamFormedAudioVector[k+1][i]);	// y3
		}

		curroffset = interpolateNSamples;

		// interpolation for the second half
		for (int j = interpolateNSamples; j < interpolateNSamples * 2; j++) {

			if (j == (int)curroffset && k > 1) {

				if (j != interpolateNSamples) {
					k--;
				}

				curroffset += offset;
			}

			highResolutionAudioMap[j][i] = splineApproximation(	j,										// x
																curroffset - offset,					// x1
																reducedBeamFormedAudioVector[k][i],		// y1
																curroffset,								// x2
																reducedBeamFormedAudioVector[k-1][i],	// y2
																curroffset + offset,					// x3
																reducedBeamFormedAudioVector[k-2][i]);	// y3
		}
	}
}
