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
*	This function will create a filter bank which brakes the auditory signal into
*/

#ifndef _GAMMATONE_FILTER_H_
#define _GAMMATONE_FILTER_H_

#define BW_CORRECTION      1.0190
#define VERY_SMALL_NUMBER  1e-100
#ifndef M_PI
#define M_PI               3.14159265358979323846
#endif

#include <vector>
#include <math.h>
#include <fstream>

class GammatoneFilter {

public:
	/**
	*	Default constructor
	*	Calls loadFile()
	*	Calls makeErbCFs()
	*	Pre-allocates the data structures that will hold the inputAudio as well as the filteredAudio
	*	@param SampleRate
	*	@param lowCF
	*	@param highCF
	*	@param numBands
	*	@param nSamples
	*	@param numMics
	*	@param al
	*	@param hr
	*/
	GammatoneFilter(int SampleRate, int lowCF, int highCF, int numBands, int nSamples, int numMics, bool al, bool hr);
	
	/**
	*	 Default De-constructor
	*/
	~GammatoneFilter();

	/**
	*	getFilteredAudio
	*	Return a vector containing the filtered audio
	*	@return      The a vector of arrays in which the first half of the vector is left channels the second half of the vector is the right channel
	*/
	std::vector< float* > getFilteredAudio();

	/**
	*	gammatoneFilterBank
	*	Filters the passed in audio file thorugh the gammatone filter bank.
	*	@param  inAudio An array which contains audio data. The left channel are all the even index's and right channels are all the odd indexs.
	*/
	void gammatoneFilterBank(float *inAudio);

	/**
	*	reverseFilterBank
	*	Filters the passed in audio file thorugh the gammatone filter bank.
	*	@param  inAudio An array which contains audio data. The left channel are all the even index's and right channels are all the odd indexs.
	*/
	float* reverseFilterBank(std::vector< float* >& inAudio);

	/**
	*	reverseFilterBank
	*	Filters the passed in audio file thorugh the gammatone filter bank.
	*	@param  inAudio An array which contains audio data. The left channel are all the even index's and right channels are all the odd indexs.
	*	@param  maskWeights
	*/
	float* reverseFilterBank(std::vector< float* >& inAudio, std::vector< double > maskWeights);


private:

	/**
	*	makeErbCFs
	*	Fills the vector called cfs using the ERB spaced centered frequencies.
	*/
	void makeErbCFs();

	/**
	*	makeLinerCFs
	*	Fills the vector called cfs using the Liner spaced centered frequencies.
	*/
	void makeLinerCFs();

	/**
	*	HzToErbRate
	*	Takes a Hz input and translate it into ERB
	*	@param The Hz frequency to be converted. 
	*	@return The converted Erb centered frequency.
	*/
	double HzToErbRate(double Hz);

	/**
	*	ErbRateToHz
	*	Takes a Erb input and translate it into Hz
	*	@param Erb
	*	@return Hz 
	*/
	double ErbRateToHz(double Erb);

	float* singleFilter(float* input, double centerFreqency);




	int nBands;
	std::vector< float* > filteredAudio;						//filtered audio 

	std::vector< float* > inputSplitAudio;

	const float *inputSignal;									//input audio

	int samplingRate;											//Sampling frequency is calculated in Hz

	std::vector<double> cfs;									//Perimeters to calculate the center frequencies that the gammaton filter bank will use
	int lowerCF;												//Lowest center frequency to use in the gammaton filter
	int higherCF;												//Highest center frequency to use in the gammaton filter

	int frameSamples;											//The number of samples per audio frame
	bool align;											
	int nMics;													//The number of microphones used
	
	bool hrect;
	double p0r, p1r, p2r, p3r, p4r, p0i, p1i, p2i, p3i, p4i;
	float *P;

	double oldcs;
	double tpt;

};

#endif
