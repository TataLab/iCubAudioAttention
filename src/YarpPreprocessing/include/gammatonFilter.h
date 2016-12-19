// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
*	This function will create a filter bank which brakes the auditory signal into
*/

#ifndef _GAMMATON_FILTER_H_
#define _GAMMATON_FILTER_H_

#define BW_CORRECTION      1.0190
#define VERY_SMALL_NUMBER  1e-100
#ifndef M_PI
#define M_PI               3.14159265358979323846
#endif



#include "../../Configuration/ConfigParser.h"

#include <vector>
#include <string>
#include <stdio.h>
#include <math.h>
#include <string>
#include <thread>
#include <fstream>

class GammatonFilter {

public:
	/**
	*	Default constructor
	*	Calls loadFile()
	*	Calls makeErbCFs()
	*	Pre-allocates the data structures that will hold the inputAudio as well as the filteredAudio
	*/
	GammatonFilter(std::string file);
	
	/**
	*	 Default De-constructor
	*/
	~GammatonFilter();

	/**
	*	inputAudio
	*	Sets the audio file that is passed into this file as the audio to be filtered
	*	TODO make an Error check function that will check if the input audio is valid
	*	Calls generatFilter()
	*	@param  inAudio An array which contains audio data. The left channel are all the even index's and right channels are all the odd indexs.
	*/
	void inputAudio(float *inAudio);
	
	/**
	*	getFilteredAudio
	*	Return a vector containing the filtered audio
	*	(first half of the vector is left channels the second half of the vector is the right channel)
	*	@return      The a vector of arrays which
	*/
	std::vector< float* > getFilteredAudio();

private:
	/**
	*	 Generates the gammaton filter.
	*	@param MicNumber which mic is being used to generate the current filter.
	*/
	void generatFilter(int MicNumber);

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
	*	loadFile
	*	Accesses the loadFile.xml that is found in the Configuration directory
	*	load all required parameters for the gammatone filter bank.
	*/
	void loadFile();

	/**
	*	HzToErbRate
	*	Takes a Hz input and translate it into ERB
	*	@param Hz 
	*	@return Erb
	*/
	double HzToErbRate(double Hz);

	/**
	*	ErbRateToHz
	*	Takes a Erb input and translate it into Hz
	*	@param Erb
	*	@return Hz 
	*/
	double ErbRateToHz(double Erb);




	int nBands;
	std::vector< float* > filteredAudio;						//filtered audio 

	const float *inputSignal;									//input audio

	int samplingRate;											//Sampling frequency is calculated in Hz

	std::vector<double> cfs;									//Perimeters to calculate the center frequencies that the gammaton filter bank will use
	int lowerCF;												//Lowest center frequency to use in the gammaton filter
	int higherCF;												//Highest center frequency to use in the gammaton filter

	std::string fileName;										//Path and name of the file containing all defeat perimeters

	int frameSamples;											//The number of samples per audio frame
	bool align;													//If 
	int nMics;													//The number of microphones used
	
	bool hrect;
	double p0r, p1r, p2r, p3r, p4r, p0i, p1i, p2i, p3i, p4i;
	float *P;

};

#endif
