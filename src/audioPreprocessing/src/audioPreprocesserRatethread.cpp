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
	delete outBeamFormedAudioPort;
	delete outReducedBeamFormedAudioPort;
	delete outAudioMapEgoPort;
	delete outAudioMap;
	delete outGammaToneFilteredAudioMap;
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


	// output port for sending BeamFormed Audio
	outBeamFormedAudioPort= new yarp::os::Port();
	if (!outBeamFormedAudioPort->open("/iCubAudioAttention/BeamFormedAudio:o")) {
		yError("unable to open port to send Beamformed Audio");
		return false;
	}


	// output port for sending BeamFormed Audio
	outReducedBeamFormedAudioPort= new yarp::os::Port();
	if (!outReducedBeamFormedAudioPort->open("/iCubAudioAttention/ReducedBeamFormedAudio:o")){
		yError("unable to open port to send Reduced Beamformed Audio");
		return false;
	}


	// output port for sending the Audio Map
	outAudioMapEgoPort = new yarp::os::Port();
	if (!outAudioMapEgoPort->open("/iCubAudioAttention/AudioMapEgo:o")){
		yError("unable to open port to send Audio Map");
		return false;
	}


	// error checking
	if (yarp::os::Network::exists("/iCubAudioAttention/AudioPreprocesser:i")) {
		if (yarp::os::Network::connect("/sender", "/iCubAudioAttention/AudioPreprocesser:i") == false) {
			yError("Could not make connection to /sender. \nExiting. \n");
			return false;
		}
	}

	// inPort failed to open. quit.
	else {
		return false;
	}

	// prepare GammatoneFilter object
	gammatoneAudioFilter = new GammatoneFilter(samplingRate, lowCf, highCf, nBands, frameSamples, nMics, false);

	// prepare BeamFormer object
	beamForm = new BeamFormer(nBands, frameSamples, nMics, nBeamsPerHemi);

	// prepare other memory structures
	rawAudio = new float[(frameSamples * nMics)];

	// initialize a two dimentianl vector that
	// is 2 * interpolateNSamples x nBands
	for (int i = 0; i < interpolateNSamples * 2; i++) {

		std::vector<double> tempvector(nBands, 0);

		highResolutionAudioMap.push_back(tempvector);
	}

	// construct a yarp Matrix for sending an Audio Map
	outAudioMap = new yarp::sig::Matrix(nBands, interpolateNSamples * 2);

	// construct a yarp Matrix for sending GammaTone Filtered Audio
	outGammaToneFilteredAudioMap = new yarp::sig::Matrix(nBands*2, frameSamples);

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

	if (ts.getCount() != lastframe + 1) {

	}


	// initialize rawAudio to be usable
	for (int col = 0 ; col < frameSamples; col++)
		for (int micLoop = 0; micLoop < nMics; micLoop++) 
	  		rawAudio[col*nMics + micLoop] = s->get(col, micLoop) / normDivid;


	// run the Filter Bank on the raw Audio
	gammatoneAudioFilter->gammatoneFilterBank(rawAudio);

	if (outGammaToneAudioPort->getOutputCount()) {
		// format the gammatone filtered audio into
		// a sendable format, set the envelope,
		// then publish to the network
		sendGammatoneFilteredAudio(gammatoneAudioFilter->getFilteredAudio());
	}

	// set the beamformers audio to be the filtered audio
	beamForm->inputAudio(gammatoneAudioFilter->getFilteredAudio());

	// run the beamformer on the set audio
	// beamFormedAudioVector = beamForm->getBeamAudio();

	if (outBeamFormedAudioPort->getOutputCount()) {
		// format the beam formed audio into
		// a sendable format, set the envelope,
		// then publish to the network
		// sendBeamFormedAudio(beamFormedAudioVector);
	}

	// run the reduced-beamformer on the set audio
	reducedBeamFormedAudioVector = beamForm->getReducedBeamAudio();

	if (outReducedBeamFormedAudioPort->getOutputCount()) {
		// format the reduced beam formed audio into
		// a sendable format, set the envelope,
		// then publish to the network
		sendReducedBeamFormedAudio(reducedBeamFormedAudioVector);
	}

	// do an interpolate on the reducedBeamFormedAudioVector
	// to produce a highResolutionAudioMap
	linerInterpolate();

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
	outBeamFormedAudioPort->interrupt();
	outReducedBeamFormedAudioPort->interrupt();
	outAudioMapEgoPort->interrupt();

	// release all ports
	inPort->close();
	outGammaToneAudioPort->close();
	outBeamFormedAudioPort->close();
	outReducedBeamFormedAudioPort->close();
	outAudioMapEgoPort->close();
}


void AudioPreprocesserRatethread::loadFile(yarp::os::ResourceFinder &rf) {

	// import all relevant data fron the .ini file
	yInfo("loading configuration file");
	try {
		frameSamples        =  rf.check("frameSamples",
										Value("4096"),
										"frame samples (int)").asInt();

		nBands              =  rf.check("nBands",
										Value("128"),
										"numberBands (int)").asInt();

		nMics               =  rf.check("nMics",
										Value("2"),
										"number mics (int)").asInt();

		interpolateNSamples =  rf.check("interpolateNSamples",
										Value("180"),
										"interpellate N samples (int)").asInt();

		micDistance         =  rf.check("micDistance",
										Value("0.145"),
										"micDistance (double)").asDouble();

		C                   =  rf.check("C",
										Value("338"),
										"C speed of sound (int)").asInt();

		samplingRate        =  rf.check("samplingRate",
										Value("48000"),
										"sampling rate of mics (int)").asInt();

		lowCf               =  rf.check("lowCf",
										Value("1000"),
										"lowest center frequency(int)").asInt();

		highCf              =  rf.check("highCf",
										Value("3000"),
										"highest center frequency(int)").asInt();

		// print information from rf to the console
		yInfo("micDistance = %f", micDistance);
		nBeamsPerHemi  = (int)((micDistance / C) * samplingRate) - 1;

		yInfo("_beamsPerHemi %d = %f / %d * %d", nBeamsPerHemi, micDistance, C, samplingRate);
		totalBeams = nBeamsPerHemi * 2 + 1;

		yInfo("frameSamples = %d", frameSamples);
		yInfo("nBands = %d", nBands);
		yInfo("nMics = %d", nMics);
		yInfo("interpolateNSamples = %d", interpolateNSamples );
		yInfo("total beams = %d",totalBeams);
	}

	catch (int a) {
		yError("Error in the loading of file");
	}

	yInfo("file successfully load");
}


void AudioPreprocesserRatethread::sendGammatoneFilteredAudio(const std::vector<float*> &gammatoneAudio) {

	// fill the yarp Matrix with gammatone filtered audio
	for (int i = 0; i < nBands; i++) {
		yarp::sig::Vector tempV(frameSamples);
		for (int j = 0; j < frameSamples; j++) {
			tempV[j] = gammatoneAudio[i][j];
		}
		outGammaToneFilteredAudioMap->setRow(i, tempV);
	}

	// set the envelope for the Gammatone Filtered Audio port
	outGammaToneAudioPort->setEnvelope(ts);

	// publish the map onto the network
	outGammaToneAudioPort->write(*outGammaToneFilteredAudioMap);
}


void AudioPreprocesserRatethread::sendBeamFormedAudio(const std::vector<std::vector<float*> > &beamFormedAudio) {

	// fill the yarp Matrix with uncompressed beamformed audio
	for (int i = 0; i < nBands; i++) {
		yarp::sig::Vector tempV(totalBeams);
		for (int j = 0; j < totalBeams; j++) {
			//tempV[j] = beamFormedAudio[i][j];
		}
		outBeamFormedAudioMap->setRow(i, tempV);
	}

	// set the envelope for the Beam Formed Audio port
	outBeamFormedAudioPort->setEnvelope(ts);
	
	// publish the map onto the network
	outBeamFormedAudioPort->write(*outBeamFormedAudioMap);
}


void AudioPreprocesserRatethread::sendReducedBeamFormedAudio(const std::vector<std::vector<double> > &reducedBeamFormedAudio) {

	// fill the yarp Matrix with reduced beamformed audio
	for (int i = 0; i < nBands; i++) {
		yarp::sig::Vector tempV(totalBeams);
		for (int j = 0; j < totalBeams; j++) {
			tempV[j] = reducedBeamFormedAudio[i][j];
		}
		outReducedBeamFormedAudioMap->setRow(i, tempV);
	}

	// set the envelope for the Reduced Beam Formed Audio port
	outReducedBeamFormedAudioPort->setEnvelope(ts);

	// publish the map onto the network
	outReducedBeamFormedAudioPort->write(*outReducedBeamFormedAudioMap);
}


void AudioPreprocesserRatethread::sendAudioMap() {

	// fill the yarp Matrix with interpolated beamformed audio
	for (int i = 0; i < nBands; i++) {
		yarp::sig::Vector tempV(interpolateNSamples * 2);
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			tempV[j] = highResolutionAudioMap[j][i];
		}
		outAudioMap->setRow(i, tempV);
	}

	// set the envelope for the Audio Map port
	outAudioMapEgoPort->setEnvelope(ts);

	// publish the map onto the network
	outAudioMapEgoPort->write(*outAudioMap);
}


inline double AudioPreprocesserRatethread::linerApproximation(int x, int x1, double y1, int x2, double y2) {
	return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
}


void AudioPreprocesserRatethread::linerInterpolate() {

	double offset = (interpolateNSamples / (double)totalBeams);

	for (int i = 0; i < nBands; i++) {

		int k = 0;
		double curroffset = 0;

		// interpolation for the first half
		for (int j = 0; j < interpolateNSamples; j++) {

			if (j == (int)curroffset && k < (totalBeams - 1)) {
				curroffset += offset;
				k++;
			}

			highResolutionAudioMap[j][i] = linerApproximation(	j,										// x
																curroffset - offset,					// x1
																reducedBeamFormedAudioVector[k-1][i],	// y2
																curroffset,								// x2
																reducedBeamFormedAudioVector[k][i]);	// y2
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

			highResolutionAudioMap[j][i] = linerApproximation(	j,										// x
																curroffset - offset,					// x1
																reducedBeamFormedAudioVector[k][i],		// y1
																curroffset,								// x2
																reducedBeamFormedAudioVector[k-1][i]);	// y2
		}
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

	double offset = (interpolateNSamples / (double)totalBeams);

	for (int i = 0; i < nBands; i++) {

		int k = 0;
		double curroffset = 0;

		// interpolation for the first half
		for (int j = 0; j < interpolateNSamples; j++) {
			if (j == (int)curroffset && k < (totalBeams - 1)) {
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
