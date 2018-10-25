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
 * @file  beamFormer.cc
 * @brief Implementation of the beamformer.
 */

#include "beamFormer.h"


//inline int myMod(int a, int b) { return  a >= 0 ? a % b : (a % b) + b; }
inline int myMod(int a, int b) { return  a >= 0 ? a % b : ((a % b) + b) % b; }


BeamFormer::BeamFormer(int numBands, int numSamples, int numMics, int numBeamsHemifield) :
 nBands(numBands), frameSamples(numSamples), nMics(numMics), nBeamsPerHemi(numBeamsHemifield) {
 
	nBeams = (nBeamsPerHemi*2) + 1;

	// Allocating the required vectors to create the beamFormed audio
	for (int i = 0; i < nBands; i++) {

		std::vector<std::vector<float> > tempvector;

		for (int j = 0; j < nBeams; j++) {
			std::vector<float> temparray(frameSamples, 0.f);
			tempvector.push_back(temparray);
		}

		beamFormedAudioVector.push_back(tempvector);

		std::vector<double> tempvec(nBeams, 0.0);
		reducedBeamFormedAudioVector.push_back(tempvec);
	}

	// Allocating the required vectors to create the reduced beamFormed audio
	/*
	for (int i = 0; i < nBeams; i++) {

		std::vector<double> tempvector(nBands, 0);

		reducedBeamFormedAudioVector.push_back(tempvector);
	}
	*/

	// Allocate space for the powerAudio vector.
	for (int i = 0; i < nBands; i++) {
		powerAudio.push_back(0.0);
	}
}


BeamFormer::~BeamFormer() {
	for(int i = 0; i < inputSignal.size();i++)
		delete[] inputSignal[i];
}


void BeamFormer::inputAudio(std::vector< float* > inAudio) {
	// Sets the input audio passed into this function as inAudio.
	inputSignal = inAudio;
}


std::vector<std::vector<std::vector<float> > > BeamFormer::getBeamAudio() {

	// init
	int band = 0, lowerBand = 0, limit = std::thread::hardware_concurrency();

	// construct the threads once
	std::vector<std::thread> myThread(limit);

	// run beamforming on all indices
	while (band < nBands) {
		for ( ; band < nBands; band++) {

			// break if at thread limit
			if (band-lowerBand+1 > limit) break;

			// begin thread
			myThread[band-lowerBand] = std::thread(&BeamFormer::audioMultiThreadingLoop, this, band);
		}

		// join the finished threads so
		// that they may be reassigned
		for (int thread_id = 0; lowerBand < band; lowerBand++, thread_id++)
			myThread[thread_id].join();
	}

	return beamFormedAudioVector;
}


std::vector<std::vector<double> > BeamFormer::getReducedBeamAudio() {

	// init
	int band = 0, lowerBand = 0, limit = std::thread::hardware_concurrency();

	// construct the threads once
	std::vector<std::thread> myThread(limit);

	// run beamforming on all indices
	while (band < nBands) {
		for ( ; band < nBands; band++) {

			// break if at thread limit
			if (band-lowerBand+1 > limit) break;

			// begin thread
			myThread[band-lowerBand] = std::thread(&BeamFormer::reducedAudioMultiThreadingLoop, this, band);
		}

		// join the finished threads so
		// that they may be reassigned
		for (int thread_id = 0; lowerBand < band; lowerBand++, thread_id++)
			myThread[thread_id].join();
	}

	return reducedBeamFormedAudioVector;
}


void BeamFormer::audioMultiThreadingLoop(int band) {

	for (int beam = 0; beam < nBeams; beam++) {
		for (int frame = 0; frame < frameSamples; frame++) {
			beamFormedAudioVector[band][beam][frame] = (inputSignal[band][frame] + inputSignal[band + nBands][myMod(frame + (nBeamsPerHemi - beam), frameSamples)]);
		}
	}
}


void BeamFormer::reducedAudioMultiThreadingLoop(int band) {

	for (int beam = 0; beam < nBeams; beam++) {

		double beamSum = 0.0;
		for (int frame = 0; frame < frameSamples; frame++) {
			beamSum += pow((inputSignal[band][frame] + inputSignal[band + nBands][myMod(frame + (nBeamsPerHemi - beam), frameSamples)]), 2);
		}
		
		reducedBeamFormedAudioVector[band][beam] = sqrt(beamSum / ((double)frameSamples));
	}
}


std::vector< double > BeamFormer::getPowerAudio() {
	
	for (int band = 0; band < nBands; band++) {

		//-- Clear out the previous powerAudio vector.
		powerAudio[band] = 0.0;
	
		//-- Take the average of power at each
		//-- beam and average it.
		for (int beam = 0; beam < nBeams; beam++) {
			powerAudio[band] += reducedBeamFormedAudioVector[band][beam];
		}

		//-- Go through now and find the mean at each index of powerAudio.
		powerAudio[band] /= nBeams;
	}

	return powerAudio;
}