// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author: Matt Tata, Marko Ilievski, Austin Kothig, Francesco Rea
  * email: m.ilievski@uleth.ca, matthew.tata@uleth.ca, kothiga@uleth.ca, francesco.rea@iit.it
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
 * @file  gammatoneFilter.h
 * @brief Header file of the gammatone filter class.
 * 		  This function will filter given audio input using a 
 * 		  gammatone filter bank into any number of frequency bands.
 */

#ifndef _GAMMATONE_FILTER_H_
#define _GAMMATONE_FILTER_H_

#define BW_CORRECTION      1.0190
#define VERY_SMALL_NUMBER  1e-100
#ifndef M_PI
#define M_PI               3.14159265358979323846
#endif

#include <math.h>
#include <vector>

class GammatoneFilter {

 public:
	/**
	 *	constructor
	 *
	 *	Calls loadFile()
	 *	Calls makeErbCFs()
	 *
	 *	Pre-allocates the data structures that will hold the inputAudio as well as the filteredAudio
	 *
	 *	@param sampleRate : The sampling rate at which the audio is captured.
	 *	@param      lowCF : The lower frequency bound that the gammatone filter bank will use.
	 *	@param     highCF : The highest frequency bound that the gammatone filter bank will use.
	 *	@param   numBands : The number of bands that should be created by the filter bank.
	 *	@param 	 nSamples : The number of recorded samples that are in the current audio frame.
	 *	@param 	  numMics : The number of microphones used to record the audio
	 *	@param 	       hr : Determines if the signal is clipped at 0
	 */
	GammatoneFilter(int SampleRate, int lowCF, int highCF, int numBands, int nSamples, int numMics, bool hr);
	

	/**
	 *	destructor
	 */
	~GammatoneFilter();


	/**
	 *	getFilteredAudio
	 *
	 *	Returns a vector containing the filtered audio, of which the first half of the
	 * 	vector is the left channel, and the second half of the vector is the right channel
	 *
	 *	@return filtered audio
	 */
	std::vector< float* > getFilteredAudio();


	/**
	 *  getPowerAudio
	 * 
	 *  Returns a vector containing the power of the filtered audio, at each band across framesamples.
	 * 
	 *  @return power of filtered audio
	 */
	std::vector< float > getPowerAudio();


	/**
	 *	gammatoneFilterBank
	 *
	 *	Filters the passed in audio through the gammatone filter bank using the parameters given in the constructor.
	 *
	 *	@param inAudio : An array which contains audio data. All of the even index's are the left
	 *                   audio channel and all of the odd index's are the right audio channel
	 */
	void gammatoneFilterBank(float *inAudio);


	/**
	 *	reverseFilterBank
	 *
	 *	Takes a vector of gammatone filtered audio and reverses the filter bank returning the audio into its previous state.
	 *
	 *	@param inAudio : An array which contains filtered audio data.
	 *
	 *	@return unfiltered audio
	 */
	float* reverseFilterBank(std::vector< float* >& inAudio);


	/**
	 *	reverseFilterBank
	 *
	 *	Takes a vector of gammatone filtered audio and a given mask and reverses the filter bank 
	 *	using the mask weight to recreate the audio stream.
	 *
	 *	@param     inAudio : An array which contains audio data. All of the even index's are the left
	 *                       audio channel and all of the odd index's are the right audio channel
	 *	@param maskWeights : a mask that will be multiplied by the reverse filtered audio
	 *
	 *	@return unfiltered audio
	 */
	float* reverseFilterBank(std::vector< float* >& inAudio, std::vector< double > maskWeights);


 private:
	/**
	 *	makeErbCFs
	 *
	 *	Fills the vector called cfs using the ERB spaced centered frequencies.
	 */
	void makeErbCFs();


	/**
	 *	makeLinerCFs
	 *
	 *	Fills the vector called cfs using the Liner spaced centered frequencies.
	 */
	void makeLinerCFs();


	/**
	 *	HzToErbRate
	 *
	 *	Takes a Hz input and translate it into ERB
	 *
	 *	@param Hz : frequency to be converted. 
	 *
	 *	@return The converted Erb centered frequency.
	 */
	double HzToErbRate(double Hz);


	/**
	 *	ErbRateToHz
	 *
	 *	Takes a Erb input and translate it into Hz
	 *
	 *	@param Erb : input to be converted.
	 *
	 *	@return The converted Hz frequency. 
	 */
	double ErbRateToHz(double Erb);


	/**
	 *	singleFilter
	 *
	 *	Function that takes any input steam and applies a gammatone filter centered at a given center frequency. 
	 *	This is the "meat" of this class.
	 *
	 *	@param	        input : Currently audio stream that is being processed.
	 *	@param centerFreqency : The center frequency around which the gammatone filter will be applied.
	 *
	 *	@return The gammatone filtered audio. 
	 */
	float* singleFilter(float* input, double centerFreqency);


	// 
	// constructor variables
	//
	int samplingRate;	// Sampling frequency is calculated in Hz
	int lowerCF;		// Lowest center frequency to use in the gammatone filter
	int higherCF;		// Highest center frequency to use in the gammatone filter
	int nBands;			// The number of bands the filter bank should process
	int frameSamples;	// The number of samples per audio frame
	int nMics;			// The number of microphones used							
	bool hrect;			// Determines if the signal is clipped at 0


	// 
	// containers for proccessed data
	//
	std::vector< float* > inputSplitAudio;	// Splits the input 
	std::vector< float* > filteredAudio;	// filtered audio 
	std::vector< float  > powerAudio;       // power of filtered audio
	std::vector< double > cfs;				// Perimeters to calculate the center frequencies that the gammatone filter bank will use
	
	const float *inputSignal;				// input audio
		  float *tempFilteredAudio; 		// temp container for filter calculations


	// 
	// calculated variables in
	// function singleFilter
	//
	double p0r;
	double p1r;
	double p2r;
	double p3r;
	double p4r;
	double p0i;
	double p1i;
	double p2i;
	double p3i;
	double p4i;
	double oldcs;
	double tpt;
};

#endif  //_GAMMATONE_FILTER_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
