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

#include "beamFormer.h"

inline int myMod(int a, int b) {
	return  a >= 0 ? a % b : (a % b) + b;
}


BeamFormer::BeamFormer(int numBands, int nSamples, int numMics, int numBeamsHemifield):
nMics(numMics), frameSamples(nSamples), nBands(numBands), getNBeamsPerHemifield(numBeamsHemifield) {
	
	totalBeams = (getNBeamsPerHemifield*2) + 1;

	//Allocating the required vectors to create the beamFormed audio 
	for (int i = 0; i < totalBeams; i++) {

		std::vector<std::vector<float> > tempvector;

		for (int j = 0; j < nBands; j++) {
			std::vector<float> temparray(frameSamples, 0);
			tempvector.push_back(temparray);
		}

		beamFormedAudioVector.push_back(tempvector);
	}

	//Allocating the required vectors to create the reduced beamFormed audio 
	for (int i = 0; i < totalBeams; i++) {

		std::vector<double> tempvector(nBands, 0);
		
		reducedBeamFormedAudioVector.push_back(tempvector);
	}
}


BeamFormer::~BeamFormer() {

	for(int i = 0; i < inputSignal.size();i++) {
		delete[] inputSignal[i];
	}
}


void BeamFormer::inputAudio(std::vector< float* > inAudio) {
	//Sets the input audio passed into this function as inAudio.
	inputSignal = inAudio;
}


std::vector<std::vector<std::vector<float> > > BeamFormer::getBeamAudio() {

	int i = 0, j = 0, limit = std::thread::hardware_concurrency() * 4;

	std::vector<std::thread> myThread(limit);
	
	while (i < totalBeams) {
		for ( ; i < totalBeams; i++) {

			if (i-j > limit) break;

			myThread[i-j] = std::thread(&BeamFormer::audioMultiThreadingLoop, this, i);
		}
		
		for (int k = 0; j < i; j++, k++) 
			myThread[k].join();
	}

	return beamFormedAudioVector;
}


std::vector<std::vector<double> > BeamFormer::getReducedBeamAudio() {


	int i = 0, j = 0, limit = std::thread::hardware_concurrency() * 4;

	std::vector<std::thread> myThread(limit);
	
	while (i < totalBeams) {
		for ( ; i < totalBeams; i++) {

			if (i-j > limit) break;

			myThread[i-j] = std::thread(&BeamFormer::reducedAudioMultiThreadingLoop, this, i);
		}

		
		for (int k = 0; j < i; j++, k++) 
			myThread[k].join();
	}

	return reducedBeamFormedAudioVector;
}


void BeamFormer::audioMultiThreadingLoop(int i) {


	for (int j = 0; j < nBands; j++) {
		for (int k = 0; k < frameSamples; k++) {
			beamFormedAudioVector[i][j][k] = (inputSignal[j][k] + inputSignal[j + nBands][myMod(k + ((getNBeamsPerHemifield) - i), frameSamples)]);
		}
	}
}


void BeamFormer::reducedAudioMultiThreadingLoop(int i) {

	for (int j = 0; j < nBands; j++) {
		for (int k = 0; k < frameSamples; k++) {
			reducedBeamFormedAudioVector[i][j] += pow((inputSignal[j][k] + inputSignal[j + nBands][myMod(k + ((getNBeamsPerHemifield) - i), frameSamples)]), 2);
		}
		reducedBeamFormedAudioVector[i][j] = sqrt((static_cast<double>(1) / frameSamples) * (reducedBeamFormedAudioVector[i][j]));
	}
}
