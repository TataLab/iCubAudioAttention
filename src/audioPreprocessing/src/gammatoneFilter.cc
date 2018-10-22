// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Marko Ilievski, Austin Kothig, Francesco Rea
  * email: m.ilievski@uleth.ca, matthew.tata@uleth.ca, kothiga@ueth.ca, francesco.rea@iit.it
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
 * @file  gammatoneFilter.cc
 * @brief Implementation of the gammatone filter bank.
 */

#include "gammatoneFilter.h"

#define erb(x)         ( 24.7 * ( 4.37e-3 * ( x ) + 1.0 ) )

inline float myAbs(float x) {
	return (x >= 0.00001f) ? x : -1.f * x;
}


GammatoneFilter::GammatoneFilter(int SampleRate, int lowCF, int highCF, int numBands, int nSamples, int numMics, bool hr) :
samplingRate(SampleRate), lowerCF(lowCF), higherCF(highCF), nBands(numBands), frameSamples(nSamples), nMics(numMics),  hrect(hr) {

	// Calls a function that will be used to create the ERB center
	// frequencies between the lowest and highest frequency and the number
	// of bands all specified in the configuration file.
	makeErbCFs();

	// Uncomment this if you would like Liner Center Frequencies
	//makeLinerCFs();

	tempFilteredAudio = new float[frameSamples];

	for (int i = 0; i < nMics; i++) {

		float* currentMicArray = new float[frameSamples];

		for (int j = 0; j < frameSamples; j++)
			currentMicArray[j] = 0.f;

		inputSplitAudio.push_back(currentMicArray);
	}

	for (int i = 0; i < nBands * nMics; i++) {

		float *temp = new float[frameSamples];
		filteredAudio.push_back(temp);
	}

	for (int i = 0; i < nBands; i++) {
		powerAudio.push_back(0.f);
	}

	tpt = (M_PI + M_PI) / samplingRate;
}


GammatoneFilter::~GammatoneFilter() {

	for (int i = 0; i < inputSplitAudio.size(); i++)
		delete[] inputSplitAudio[i];

	delete[] tempFilteredAudio;
}


void GammatoneFilter::gammatoneFilterBank(float *inAudio) {

	for (int i = 0; i < nMics; i++)
		for (int j = 0; j < frameSamples; j++)
			inputSplitAudio[i][j] = inAudio[j*nMics + i];


	int itrband;
	for (int i = 0; i < nMics; i++)
		for (int ch = 0; ch < nBands; ch++) {

            float *p = singleFilter(inputSplitAudio[i], cfs[ch]);

			itrband = ch + (i * nBands);
            for (int frame = 0; frame < frameSamples; frame++) {
    			filteredAudio[itrband][frame] = p[frame];
            }
        }
}


float* GammatoneFilter::reverseFilterBank(std::vector< float* >& inAudio) {

	float* result = new float[frameSamples*nMics];

	for (int i=0; i < frameSamples * nMics; i++)
		result[i] = 0;

	for (int i = 0; i < nMics;i++) {

		for (int ch = 0; ch < nBands; ch++)	{

			float reverse[frameSamples*nMics];

			// reverse
			for (int j = 0; j < frameSamples; j++)
				reverse[frameSamples-j-1] = inAudio[ch + (i * nBands)][j];

			// run through filter again
			float* rereverse = singleFilter(reverse, cfs[ch]);

			// reverse again back to normal order
			for (int j = 0; j < frameSamples; j++)
				result[(frameSamples-j-1)*nMics+i] += rereverse[j];

			delete[] rereverse;
		}
	}

	return result;
}


float* GammatoneFilter::reverseFilterBank(std::vector< float* >& inAudio,  std::vector< double > maskWeights) {

	float* result = new float[frameSamples*nMics];

	for (int i = 0; i < frameSamples * nMics; i++)
		result[i] = 0;

	for (int i = 0; i < nMics; i++) {
		for (int ch = 0; ch < nBands; ch++) {

			float reverse[frameSamples*nMics];

			// reverse
			for (int j = 0; j < frameSamples; j++)
				reverse[frameSamples-j-1] = inAudio[ch + (i * nBands)][j];

			// run through filter again
			float* rereverse = singleFilter(reverse, cfs[ch]);

			// reverse again back to normal order
			for (int j = 0; j < frameSamples; j++)
				result[(frameSamples-j-1)*nMics+i] += rereverse[j] * maskWeights[ch];

			delete[] rereverse;
		}
	}

	return result;
}


void GammatoneFilter::makeErbCFs() {

	// Calculates the lower bound in ERB space
	double minERB = HzToErbRate(lowerCF);

	// Calculates the upper bound in ERB space
	double highERB = HzToErbRate(higherCF);

	// Calculates the incrementing amount
	double incAmount = (highERB - minERB) / (nBands - 1);

	for (int i = 0; i < nBands; i++)
		cfs.push_back(ErbRateToHz(incAmount * i + minERB));
}


void GammatoneFilter::makeLinerCFs() {

	double incAmount = (higherCF - lowerCF) / (nBands - 1);

	for (int i = 0; i < nBands; i++)
		cfs.push_back(incAmount * i + lowerCF);
}


inline double GammatoneFilter::HzToErbRate(double Hz) {
	return (21.4 * log10(0.00437 * Hz + 1));
}


inline double GammatoneFilter::ErbRateToHz(double Erb) {
	return (pow(10, Erb / 21.4) - 1) / 0.00437;
}


std::vector< float* > GammatoneFilter::getFilteredAudio() {
	return filteredAudio;
}


float* GammatoneFilter::singleFilter(float* input, double centerFreqency) {

	p0r = 0, p1r = 0, p2r = 0, p3r = 0, p4r = 0, p0i = 0, p1i = 0, p2i = 0, p3i = 0, p4i = 0;

	double tptbw = tpt * erb(centerFreqency) * BW_CORRECTION;
	double a = exp(-tptbw);
	double gain = (tptbw * tptbw * tptbw * tptbw) / 3;

	double a1 =  4.0 * a;
	double a2 = -6.0 * a * a;
	double a3 =  4.0 * a * a * a;
	double a4 = -a   * a * a * a;
	double a5 =  a   * a;

	double coscf = cos(tpt * centerFreqency);
	double sincf = sin(tpt * centerFreqency);
	double qcos = 1;
	double qsin = 0;

	for (int t = 0; t < (frameSamples); t++) {

		float currentInput = input[t];
		p0r = qcos * currentInput + a1 * p1r + a2 * p2r + a3 * p3r + a4 * p4r;
		p0i = qsin * currentInput + a1 * p1i + a2 * p2i + a3 * p3i + a4 * p4i;

		if (fabs(p0r) < VERY_SMALL_NUMBER)
			p0r = 0.0F;

		if (fabs(p0i) < VERY_SMALL_NUMBER)
			p0i = 0.0F;

		double u0r = p0r + a1 * p1r + a5 * p2r;
		double u0i = p0i + a1 * p1i + a5 * p2i;

		p4r = p3r; p3r = p2r; p2r = p1r; p1r = p0r;
		p4i = p3i; p3i = p2i; p2i = p1i; p1i = p0i;

		tempFilteredAudio[t] = (u0r * qcos + u0i * qsin) * gain;

		if (hrect && tempFilteredAudio[t] < 0)
			tempFilteredAudio[t] = 0;

		qcos = coscf * (oldcs = qcos) + sincf * qsin;
		qsin = coscf * qsin - sincf * oldcs;
	}

	return tempFilteredAudio;
}


std::vector< float > GammatoneFilter::getPowerAudio() {

	//-- Take the average of power at each 
	//-- framesample, for each band.
	int itrBand;
	float currentBand;
	for (int band = 0; band < nBands; band++) {
		currentBand = 0.f;

		//-- Go through each mic that makes up the filtered audio.
		for (int mic = 0; mic < nMics; mic++) {

			//-- Set the current index of band
			//-- that is being iterated over.
			itrBand = (mic * nBands) + band;

			//-- add this sample to the running sum.
			for (int frame = 0; frame < frameSamples; frame++) {
				currentBand += myAbs(filteredAudio[itrBand][frame]);
			}
		}

		//-- Take the average of power for the current band.
		currentBand /= (nMics * frameSamples);
		
		//-- Append this the the power map.
		powerAudio[band] = currentBand;
	}

	return powerAudio;
}
