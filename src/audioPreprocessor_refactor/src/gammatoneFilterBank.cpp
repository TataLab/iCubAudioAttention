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

#define myMod(x,y)     ( ( x ) - ( y ) * floor ( ( x ) / ( y ) ) )

GammatoneFilterBank::GammatoneFilterBank(int mics, int samples, int frames, int bands, double lcf, double hcf, bool hrec, bool erbs) : 
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
	 *    the lowest to highest. 
	 *    - The length of this table is specified by numBands. 
	 *    - The type of spacing is specified by erbSpaced.
	 * =========================================================================== */
	if (erbSpaced) {
		makeErbCFs();
	} else {
		makeLinearCFs();
	}
	
	_proxyBasilarMembrane.resize(numMics * numBands, numFrameSamples);
	_proxyEnvelope.resize(numMics * numBands, numFrameSamples);
	_proxyPhase.resize(numMics * numBands, numFrameSamples);

	tpt = (_pi + _pi) / samplingRate;
}


GammatoneFilterBank::~GammatoneFilterBank() {
	
}


void GammatoneFilterBank::getGammatoneFilteredAudio(const yarp::sig::Matrix& RawAudio, yarp::sig::Matrix& FilterBank) {

	//-- Ensure space is allocated for the filtered audio.
	FilterBank.resize(numMics * numBands, numFrameSamples);
	_proxyEnvelope.resize(numMics * numBands, numFrameSamples);

	//-- Loop variabes.
	int itrBandMic;
	int numBandMic = numMics * numBands;
	
	#ifdef WITH_OMP
	#pragma omp parallel \
  	 shared  (RawAudio, FilterBank, _proxyEnvelope) \
  	 private (itrBandMic)
	#pragma omp for schedule(guided)
	#endif
	for (itrBandMic = 0; itrBandMic < numBandMic; itrBandMic++) {

		singleGammatoneFilter (
			/* Channel of Audio = */ RawAudio       [itrBandMic / numBands],
			/* Basilar Membrane = */ FilterBank     [itrBandMic],
			/* Hilbert Envelope = */ _proxyEnvelope [itrBandMic],
			/* Center Frequency = */ cfs            [itrBandMic % numBands],
			/* Include Envelope = */ false
		);
	}	
}


void GammatoneFilterBank::getGammatoneFilteredAudio(const yarp::sig::Matrix& RawAudio, yarp::sig::Matrix& FilterBank, yarp::sig::Matrix& EnvelopeBank) {

	//-- Ensure space is allocated for the filtered audio.
	FilterBank.resize(numMics * numBands, numFrameSamples);
	EnvelopeBank.resize(numMics * numBands, numFrameSamples);

	//-- Loop variabes.
	int itrBandMic;
	int numBandMic = numMics * numBands;
	
	#ifdef WITH_OMP
	#pragma omp parallel \
  	 shared  (RawAudio, FilterBank, EnvelopeBank) \
  	 private (itrBandMic)
	#pragma omp for schedule(guided)
	#endif
	for (itrBandMic = 0; itrBandMic < numBandMic; itrBandMic++) {

		singleGammatoneFilter (
			/* Channel of Audio = */ RawAudio     [itrBandMic / numBands],
			/* Basilar Membrane = */ FilterBank   [itrBandMic],
			/* Hilbert Envelope = */ EnvelopeBank [itrBandMic],
			/* Center Frequency = */ cfs          [itrBandMic % numBands],
			/* Include Envelope = */ true
		);
	}	
}


void GammatoneFilterBank::getGammatoneFilteredPower(const yarp::sig::Matrix& FilterBank, yarp::sig::Matrix& BankPower) {

	//-- Ensure space is allocated for the powermap.
	BankPower.resize(numBands, numMics);

	for (int mic = 0; mic < numMics; mic++) {

		for (int band = 0; band < numBands; band++) {

			//-- Reduce computation by storing this once.
			const int itrband  = band + (mic * numBands);

			//-- Set the current band to 0 before beginning.
			double bandSum = 0.0;

			//-- Sum the squares of the provided filter bank.
			for (int sample = 0; sample < numFrameSamples; sample++) {
				bandSum += pow(FilterBank[itrband][sample], 2.0);
			}

			//-- Take the root of the mean.
			BankPower[band][mic] = sqrt( bandSum / (double) numFrameSamples );
		}
	}
}

#include <iostream>
void GammatoneFilterBank::getBandPassedAudio(const yarp::sig::Matrix& AudioBank, yarp::sig::Matrix& BandPassedBank, const double BandFreq) {

	//-- Ensure space is allocated for the band passed audio.
	BandPassedBank.resize(numMics * numBands, numFrameSamples);

	//-- Loop variables.
	int itrBandMic;
	int numBandMic = numMics * numBands;

	#ifdef WITH_OMP
	#pragma omp parallel \
	 shared  (AudioBank, BandPassedBank) \
	 private (itrBandMic)
	#pragma omp for schedule(guided)
	#endif
	for (itrBandMic = 0; itrBandMic < numBandMic; itrBandMic++) {

		singleBandPass (
			/* Source    = */ AudioBank[itrBandMic],
			/* Target    = */ BandPassedBank[itrBandMic],
			/* Band Pass = */ BandFreq
		);
	}
	
	////////////////////////////////////////////////////////
	////// REMOVE //////////////////////////////////////////
	////////////////////////////////////////////////////////

	double min =  999;
	double max = -999;

	for (itrBandMic = 0; itrBandMic < numBandMic; itrBandMic++) {

		for (int i = 0; i < numFrameSamples; i++) {
			if (BandPassedBank[itrBandMic][i] > max) {
				max = BandPassedBank[itrBandMic][i];
			}
			if (BandPassedBank[itrBandMic][i] < min) {
				min = BandPassedBank[itrBandMic][i];
			}
		}
	}

	std::cerr << "Min: " << min << " Max: " << max << std::endl;

	//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\/
	//\\\\ REMOVE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\/
	//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\/
}


void GammatoneFilterBank::singleGammatoneFilter(const double* RawAudio, double* BasilarMembrane, double* Envelope, const double CenterFrequency, const bool IncludeEnvelope) {

	/* ===================================================================
	*  Initialize all variables in this scope so 
	*    that they are not shared amongst threads.
	* =================================================================== */
	const double tptbw    = tpt * HzToErb(CenterFrequency) * BW_CORRECTION;

	//-- Based on integral of impulse response.
	const double gain = (tptbw*tptbw*tptbw*tptbw) / 3;
	const double a    = exp(-tptbw);

	//-- Update filter coefficients.
	const double a1 =  4.0 * a; 
	const double a2 = -6.0 * a * a; 
	const double a3 =  4.0 * a * a * a; 
	const double a4 = -a   * a * a * a; 
	const double a5 =  a   * a;

	double p0r = 0.0, p1r = 0.0, p2r = 0.0, p3r = 0.0, p4r = 0.0, u0r = 0.0;
	double p0i = 0.0, p1i = 0.0, p2i = 0.0, p3i = 0.0, p4i = 0.0, u0i = 0.0;

	double coscf  = cos(tpt * CenterFrequency);
	double sincf  = sin(tpt * CenterFrequency);
	double qcos   = 1.0; 
	double oldcs  = 0.0;
	double qsin   = 0.0;
	double pPhase = 0.0;
	double dp     = 0.0;
	double dps    = 0.0;

	/* ===================================================================
	 *  Begin running the single filter on the provided raw audio.
	 *  The resulting basilar membrane displacement is stored in
	 *    the provided filter bank matrix.
	 * =================================================================== */

	for (int sample = 0; sample < numFrameSamples; sample++) {
		
		//-- Filter part 1 & shift down to d.c.
		p0r = qcos*RawAudio[sample] + a1*p1r + a2*p2r + a3*p3r + a4*p4r;
		p0i = qsin*RawAudio[sample] + a1*p1i + a2*p2i + a3*p3i + a4*p4i;

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


		/* ===============================================================
		 *  Calculate the Basilar Membrane Response.
		 * =============================================================== */
		BasilarMembrane[sample] = (u0r * qcos + u0i * qsin) * gain;
		
		//-- Apply Half-Wave Rectifying if enabled.
		if (halfRec && BasilarMembrane[sample] < 0.0) {
			BasilarMembrane[sample] = 0.0;
		}


		/* ===============================================================
		 *  Calculate the Instantaneous Hilbert Envelope.
		 *    env = abs(u) * gain
		 * =============================================================== */
		if (IncludeEnvelope) {
			Envelope[sample] = sqrt( u0r*u0r + u0i*u0i ) * gain;
		}


		/* ===============================================================
		 *  Calculate the Instantaneous Phase.
		 *    ph = unwrap(angle(u))
		 * =============================================================== */
		//if (IncludePhase) {
		//	
		//	//-- This opperation is very slow. Revisit this later.
		//	Phase[sample] = atan2(u0i, u0r);
		//	
		//  //*
		//	// Unwrap Version 1
		//	dp = Phase[sample] - pPhase;
		//	if (fabs(dp) > _pi) {
		//		dps = myMod(dp + _pi, 2 * _pi) - _pi;
		//		if (dps == -_pi && dp > 0) {
		//			dps = _pi;
		//		}
		//		Phase[sample] = Phase[sample] + dps - dp;
		//	}
		//	pPhase = Phase[sample];
		//	//*/
		//
		//  /*
		//	// Unwrap Version 2
		//	dp = Phase[sample] - pPhase;
		//	dp = dp > _pi ? dp - 2.0 * _pi : (dp < -_pi ? dp + 2.0 * _pi : dp);
		//	Phase[sample] = pPhase + dp;
		//	pPhase = Phase[sample];
		//	*/
		//}
		
		//-- Update coefficients.
		qcos = coscf * (oldcs = qcos) + sincf * qsin;
		qsin = coscf * qsin - sincf * oldcs;
	}
}


void GammatoneFilterBank::singleBandPass(const double* Audio, double* BandPass, const double BandFreq)  {

	/* ===================================================================
	*  Initialize all variables in this scope so
	*    that they are not shared amongst threads.
	* =================================================================== */

	const double gain = 100000.0;
	const double Q  = 82.0;
	const double BW = BandFreq * BW_CORRECTION / Q;
	const double f0 = samplingRate / 2.0;
	const double C  = 1.0 / tan( _pi * (BW / samplingRate) );
	const double D  = 2.0 * cos( tpt * f0 );

	//-- Update Filter coefficients.
	const double a0 =  1.0 / (1.0 + C);
	const double a1 =  0.0;
	const double a2 = -a0;
	const double b1 = -a0 * C * D;
	const double b2 =  a0 * (C - 1.0);

	double p0i = 0.0, p1i = 0.0, p2i = 0.0;
	double p0o = 0.0, p1o = 0.0, p2o = 0.0;

	/* ===================================================================
	 *  Begin running the single butterworth band pass on the provided input audio.
	 *  The resulting band passed audio is stored in the provided band pass matrix.
	 * =================================================================== */

	for (int sample = 0; sample < numFrameSamples; sample++) {
		
		//-- Filter out all except the specified frequency band.
		p0i = gain * Audio[sample];
		p0o = (a0*p0i + a1*p1i + a2*p2i) - (b2*p2o + b1*p1o);

		//-- Clip coefficients to stop them from becoming too close to zero.
		if (fabs(p0o) < VERY_SMALL_NUMBER) {
			p0o = 0.0F;
		}
		if (fabs(p0i) < VERY_SMALL_NUMBER) {
			p0i = 0.0F;
		}

		//-- Update filter results.
		p2i = p1i; p1i = p0i;
		p2o = p1o; p1o = p0o;

		//-- Store the results.
		BandPass[sample] = p0o;
	}
}


void GammatoneFilterBank::makeErbCFs() {

	//-- Make sure space is allocated for the 
	//-- center frequency table.
	cfs.resize(numBands);

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
	cfs.resize(numBands);

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
