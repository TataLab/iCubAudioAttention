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

#ifndef _BEAM_FORMER_H_
#define _BEAM_FORMER_H_

#include "gammatoneFilter.h"


#include <iostream>
#include <vector>
#include <thread>

class BeamFormer {

public:
	/**
	 *	constructor
	 *	
	 *	Pre-allocates the data structures that will hold the inputAudio as well as the filteredAudio
	 *   
	 *	@param          numBands : number of frequency bands
	 *	@param          nSamples : sampling rate 
	 *	@param           numMics : number of microphones
	 *	@param numBeamsHemifield : number of beams in hemifield
	 */
	BeamFormer(int numBands, int nSamples, int numMics, int numBeamsHemifield);


	/**
	 *	destructor
	 */
	~BeamFormer();


	/**
	 *	inputAudio
	 *
	 *	Sets the audio file that is passed into this file as the audio to be filtered
	 *
	 *	TODO make an Error check function that will check if the input audio is valid
	 *
	 *	@param  inAudio : An array which contains audio data. The left channel are all 
	 *					  the even indexs and right channels are all the odd indexs.
	 */
	void inputAudio(std::vector< float* > inAudio);


	/**
	 *	getBeamAudio
	 *
	 *	Function that will run multithread function that will calculate the Beamformed 
	 *	Audio from information that is stored in inputSignal.
	 *
	 *	@return Beamformed data that is stored in vector of size(totalBeams) containing a vector of 
	 * 			size(number of Bands) that contains a array of float with a size(samples in frame).
	 */
	//std::vector<std::vector<float*> > getBeamAudio();
	 std::vector<std::vector<std::vector<float> > > getBeamAudio();

	/**
	 *	getReducedBeamAudio
	 *
	 *	Function that will run multithread function that will calculate the 
	 *	Beamformed Audio from information that is stored in inputSignal.
	 *	This Function then compresses the data in the time frame by running RMS(root mean square).
	 *
	 *	@return Compressed beamformed data that is stored in vector of size(totalBeams) containing 
	 *	        a vector of size(number of Bands) that contains the average power in each frame.
	 */
	std::vector<std::vector<double> > getReducedBeamAudio();

private:

	/**
	 *	reducedAudioMultiThreadingLoop
	 *
	 *	A helper function used with the C++11 multithreading to create the reducedAudioMap.
	 *
	 *	@param i : the beam that work will be done on
	 */
	void reducedAudioMultiThreadingLoop(int i);


	/**
	 *	audioMultiThreadingLoop
	 *
	 *	A helper function used with the C++11 multithreading to create the audioMap.
	 *
	 *	@param i : the beam that work will be done on
	 */
	void audioMultiThreadingLoop(int i);


	int nMics;						// Number of Microphones that is used
	int frameSamples;				// The number of samples in each frame of audio data
	int nBands;						// The maximum number of bands for the given spacing of the mics

	int getNBeamsPerHemifield;		// The maximum number of bands in each hemifield
	int totalBeams;					// Total number of beams that is used

	std::vector < float* > inputSignal;											 // The input audio signal
	std::vector < std::vector < std::vector < float > > > beamFormedAudioVector; // The uncompressed beamformed data if getBeamAudio() was called
	std::vector < std::vector < double > > reducedBeamFormedAudioVector;		 // The compressed beamformed data if getReducedBeamAudio() was called
};

#endif
