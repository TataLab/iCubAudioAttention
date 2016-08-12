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
	*	Pre Allocates the data structures that will hold the inputAudio as well as the filteredAudio
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
	*	@param  inAudio An array which contains audio data. The left channel are all the even indexs and right channels are all the odd indexs.
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
	*	 Generates the gammaton filter
	*/
	void generatFilter(int MicNumber);

	/**
	*
	*/
	void makeErbCFs();

	/**
	*
	*/
	void makeLinerCFs();
	/**
	*	loadFile
	*	Accesses the loadFile.xml that is found in the root directory of this
	*	module and load all required parameters for the gammatone filter bank.
	*/
	void loadFile();

	/**
	*
	*/
	double HzToErbRate(int Hz);

	/**
	*
	*/
	double HzToErbRate(double Hz);

	/**
	*
	*/
	double ErbRateToHz(int Hz);

	/**
	*
	*/
	double ErbRateToHz(double Hz);




	int nBands;
	//filtered audio
	std::vector< float* > filteredAudio;

	//input audio
	const float *inputSignal;

	//Sampling frequency is calculated in Hz
	int samplingRate;
	int centerFrequency;

	//Peramiters to calculate the center freqeuncies that the gammaton filter bank will use
	std::vector<double> cfs;
	int lowerCF;
	int higherCF;

	float *P;

	//Name of the file containing all defealt peramiters
	std::string fileName;

	int frameSamples;
	bool align;

	int nMics;
	bool hrect;
	std::ofstream myfile;

	double p0r, p1r, p2r, p3r, p4r, p0i, p1i, p2i, p3i, p4i;
};

#endif
