// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2018 Department of Neuroscience - University of Lethbridge
 * Author: Austin Kothig, Francesco Rea, Marko Ilievski, Matt Tata
 * email: kothiga@uleth.ca, francesco.reak@iit.it, marko.ilievski@uwaterloo.ca, matthew.tata@uleth.ca
 * 
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

/* ===========================================================================
 * @file  gammatoneFilterBank.cpp
 * @brief Implementation of the gammatone filter bank (see header file).
 * =========================================================================== */

#include <iCub/gammatoneFilterBank.h>

GammatoneFilterBank::GammatoneFilterBank(int mics, int samples, int frames, int bands, int lcf, int hcf, bool hrec, bool erbs) : 
	numMics(mics), 
	samplingRate(samples), 
	numFrameSamples(frames), 
	numBands(bands),
	lowCf(lcf),
	highCf(hcf),
	halfRec(hrec),
	erbSpaced(erbs) {

	/* ===========================================================================
	 *  Create a table with spaced center frequencies ranging from 
	 *   the lowest to highest. 
	 *   - The length of this table is specified by numBands. 
	 *   - The type of spacing is specified by erbSpaced.
	 * =========================================================================== */
	if (erbSpaced) {
		makeErbCFs();
	} else {
		makeLinearCFs();
	}
	
	tpt = (_pi + _pi) / samplingRate;
}


GammatoneFilterBank::~GammatoneFilterBank() {
	
}


void GammatoneFilterBank::getGammatoneFilteredAudio(yarp::sig::Matrix& FilterBank, const yarp::sig::Matrix& RawAudio) {

	//-- Ensure space is allocated for the filtered audio.
	FilterBank.resize(numMics * numBands, numFrameSamples);

	//-- Loop variabes.
	int mic, band, sample;

	//--
	//-- TODO: OpenMP here.
	//--
	//-- Ensure this is include above.
	//-- #include <omp.h>
	//--
	//-- Number of threads should be specified in ini.
	//--
	//-- Potentially add an additional version of this function
	//--  that is strictly serial.
	//--
	//-- Ensure this is called at some point. 
	//--  omp_set_num_threads(t);
	//--
	//# pragma omp parallel	  	\
  	//shared (filterBank, RawAudio, numMics, numBands, numFrameSamples)	 \
  	//private (mic, band, sample)
	for (mic = 0; mic < numMics; mic++) {
	
		//-- 
		//-- TODO: Uncomment.
		//--
		//# pragma omp for schedule(guided)
		for (band = 0; band < numBands; band++) {
			
			//--
			//-- TODO: Move this block into a function once validated.
			//--

			/* ===================================================================
			 *  Store a constant iterating position,
			 *   to avoid unnecessary computation.
			 *  Initialize all variables in this scope so
			 *   that they are not shared amongst threads.
			 * =================================================================== */
			const int itrband  = band + (mic * numBands);
			double    oldphase = 0.0;
			double    tptbw    = tpt * HzToErb(cfs[band]) * BW_CORRECTION;
			
			//-- Based on integral of impulse response.
			double gain = (tptbw*tptbw*tptbw*tptbw) / 3;
			double a    = exp(-tptbw);

			//-- Update filter coefficients.
			double a1 =  4.0 * a; 
			double a2 = -6.0 * a * a; 
			double a3 =  4.0 * a * a * a; 
			double a4 = -a   * a * a * a; 
			double a5 =  a   * a;

			double p0r = 0.0, p1r = 0.0, p2r = 0.0, p3r = 0.0, p4r = 0.0, u0r = 0.0;
			double p0i = 0.0, p1i = 0.0, p2i = 0.0, p3i = 0.0, p4i = 0.0, u0i = 0.0;

			double coscf = cos(tpt * cfs[band]);
			double sincf = sin(tpt * cfs[band]);
			double qcos  = 1; 
			double oldcs = 0;
			double qsin  = 0;
			
			/* ===================================================================
			 *  Begin running the single filter on the provided raw audio.
			 *  The resulting basilar membrane displacement is stored in
			 *   the provided filter bank matrix.
			 * =================================================================== */

			for (sample = 0; sample < numFrameSamples; sample++) {
				
				//-- Filter part 1 & shift down to d.c.
				p0r = qcos*RawAudio[mic][sample] + a1*p1r + a2*p2r + a3*p3r + a4*p4r;
				p0i = qsin*RawAudio[mic][sample] + a1*p1i + a2*p2i + a3*p3i + a4*p4i;

				//-- Clip coefficients to stop them from becoming too close to zero.
				if (fabs(p0r) < VERY_SMALL_NUMBER) {
					p0r = 0.0F;
				}
				if (fabs(p0i) < VERY_SMALL_NUMBER) {
					p0i = 0.0F;
				}

				//-- Filter part 2.
				u0r = p0r + a1*p1r + a5*p2r;
				u0i = p0i + a1*p1i + a5*p2i;

				//-- Update filter results.
				p4r = p3r; p3r = p2r; p2r = p1r; p1r = p0r;
				p4i = p3i; p3i = p2i; p2i = p1i; p1i = p0i;

				//-- Find the Basilar Membrane Response.
				FilterBank[itrband][sample] = (u0r * qcos + u0i * qsin) * gain;
				
				//-- Apply Half-Wave Rectifying if enabled.
				if (halfRec && FilterBank[itrband][sample] < 0.0) {
					FilterBank[itrband][sample] = 0.0;
				}

				//-- Update coefficients.
				qcos = coscf * (oldcs = qcos) + sincf * qsin;
				qsin = coscf * qsin - sincf * oldcs;
			}
		}
	}
}


void GammatoneFilterBank::getGammatoneFilteredPower(yarp::sig::Matrix& BankPower, const yarp::sig::Matrix& FilterBank) {

	//-- Ensure space is allocated for this. Clear it to zeros.
	BankPower.resize(numMics, numBands);
	BankPower.zero();

	for (int mic = 0; mic < numMics; mic++) {

		//--
		//-- TODO: parallelize this loop if needed. 
		//--

		for (int band = 0; band < numBands; band++) {

			//-- Reduce computation by storing this once.
			const int itrband  = band + (mic * numBands);

			//-- Sum the squares of the provided filter bank.
			for (int sample = 0; sample < numFrameSamples; sample++) {
				BankPower[mic][band] += pow(FilterBank[itrband][sample], 2.0);
			}

			//-- Take the root of the mean.
			BankPower[mic][band] = sqrt( BankPower[mic][band] / (double) numFrameSamples );
		}
	}
}


void GammatoneFilterBank::makeErbCFs() {

	//-- Make sure space is allocated for the 
	//-- center frequency table.
	cfs.resize(numBands, 0.0);

	//-- Calculates the lower bound in ERB space.
	double lowERB = HzToErbRate(lowCf);

	//-- Calculates the upper bound in ERB space.
	double highERB = HzToErbRate(highCf);

	//-- Calculates the incrementing amount.
	double linspace_step = (highERB - lowERB) / (numBands - 1.0);
	double current_step  = lowERB;

	for (int band = 0; band < numBands; band++) {
		cfs[band] = ErbRateToHz(current_step);
		current_step += linspace_step;
	}
}


void GammatoneFilterBank::makeLinearCFs() {
	
	//-- Make sure space is allocated for the 
	//-- center frequency table.
	cfs.resize(numBands, 0.0);

	//-- Calculates the incrementing amount.
	double linspace_step = (highCf - lowCf) / (numBands - 1.0);
	double current_step  = lowCf;

	for (int band = 0; band < numBands; band++) {
		cfs[band] = current_step;
		current_step += linspace_step;
	}
}

inline double GammatoneFilterBank::HzToErb(double Hz) {
	return ( 24.7 * ( 0.00437 * ( Hz ) + 1.0 ) );
}

inline double GammatoneFilterBank::HzToErbRate(double Hz) {
	return ( 21.4 * log10( 0.00437 * Hz + 1.0 ) );
}

inline double GammatoneFilterBank::ErbRateToHz(double Erb) {
	return (pow(10, Erb / 21.4) - 1) / 0.00437;
}
