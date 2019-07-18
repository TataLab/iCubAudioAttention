// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Department of Neuroscience - University of Lethbridge
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
 * @file  butterworthFilter.cpp
 * @brief Implementation of the butterworth filter (see header file).
 * =========================================================================== */

#include <iCub/filters/butterworthFilter.h>

using namespace Filters;

ButterworthFilter::ButterworthFilter(int rate, double q) :
    samplingRate(rate),
    q_scale(q) {

    CenterFreq_lp = -1; //-- Ensure we calculate 
    CenterFreq_hp = -1; //-- coefficients on the 
    CenterFreq_bp = -1; //-- first call of each
    CenterFreq_bn = -1; //-- filter type.
}

ButterworthFilter::~ButterworthFilter() {

}

void ButterworthFilter::getLowPassedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency) {

    if (CenterFreq_lp != CenterFrequency) {
        updateLowPassCoefficients(CenterFrequency);
    }

    //-- Find the dimensions of the audio source.
    const size_t RowSize = AudioSource.rows();
    const size_t ColSize = AudioSource.cols();
    
    //-- Ensure space is allocated for the target.
    AudioTarget.resize(RowSize, ColSize);

    //-- Loop variable.
    size_t idx;

    #ifdef WITH_OMP
	#pragma omp parallel \
	 private (idx)
	#pragma omp for schedule(guided)
	#endif
    for (idx = 0; idx < RowSize; idx++) {

        bilinearTransformation (
            AudioSource[idx],
            AudioTarget[idx],
            ColSize,
            lp_a0, lp_a1, lp_a2,
            lp_b1, lp_b2
        );
    }
}


void ButterworthFilter::getHighPassedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency) {

    if (CenterFreq_hp != CenterFrequency) {
        updateHighPassCoefficients(CenterFrequency);
    }

    //-- Find the dimensions of the audio source.
    const size_t RowSize = AudioSource.rows();
    const size_t ColSize = AudioSource.cols();
    
    //-- Ensure space is allocated for the target.
    AudioTarget.resize(RowSize, ColSize);

    //-- Loop variable.
    size_t idx;

    #ifdef WITH_OMP
	#pragma omp parallel \
	 private (idx)
	#pragma omp for schedule(guided)
	#endif
    for (idx = 0; idx < RowSize; idx++) {

        bilinearTransformation (
            AudioSource[idx],
            AudioTarget[idx],
            ColSize,
            hp_a0, hp_a1, hp_a2,
            hp_b1, hp_b2
        );
    }
}


void ButterworthFilter::getBandPassedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency) {

    if (CenterFreq_bp != CenterFrequency) {
        updateBandPassCoefficients(CenterFrequency);
    }

    //-- Find the dimensions of the audio source.
    const size_t RowSize = AudioSource.rows();
    const size_t ColSize = AudioSource.cols();
    
    //-- Ensure space is allocated for the target.
    AudioTarget.resize(RowSize, ColSize);

    //-- Loop variable.
    size_t idx;

    #ifdef WITH_OMP
	#pragma omp parallel \
	 private (idx)
	#pragma omp for schedule(guided)
	#endif
    for (idx = 0; idx < RowSize; idx++) {

        bilinearTransformation (
            AudioSource[idx],
            AudioTarget[idx],
            ColSize,
            bp_a0, bp_a1, bp_a2,
            bp_b1, bp_b2
        );
    }
}


void ButterworthFilter::getBandNochedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency) {

    if (CenterFreq_bn != CenterFrequency) {
        updateBandNochCoefficients(CenterFrequency);
    }

    //-- Find the dimensions of the audio source.
    const size_t RowSize = AudioSource.rows();
    const size_t ColSize = AudioSource.cols();
    
    //-- Ensure space is allocated for the target.
    AudioTarget.resize(RowSize, ColSize);

    //-- Loop variable.
    size_t idx;

    #ifdef WITH_OMP
	#pragma omp parallel \
	 private (idx)
	#pragma omp for schedule(guided)
	#endif
    for (idx = 0; idx < RowSize; idx++) {

        bilinearTransformation (
            AudioSource[idx],
            AudioTarget[idx],
            ColSize,
            bn_a0, bn_a1, bn_a2,
            bn_b1, bn_b2
        );
    }
}


void ButterworthFilter::setQscale(const double new_q) {

    q_scale = new_q;

    CenterFreq_lp = -1; //-- Ensure we calculate 
    CenterFreq_hp = -1; //-- coefficients on the 
    CenterFreq_bp = -1; //-- first call of each
    CenterFreq_bn = -1; //-- filter type.
}


void ButterworthFilter::bilinearTransformation(const double* AudioSource, double* AudioTarget, const size_t numSamples, const double a0, const double a1, const double a2, const double b1, const double b2) {

    //-- Filter buffer.
	double p0i = 0.0, p1i = 0.0, p2i = 0.0;
	double p0o = 0.0, p1o = 0.0, p2o = 0.0;

	/* ===================================================================
	 *  Begin running the single bilinear transform on the provided input audio.
	 *  The resulting filtered audio is stored in the provided double array matrix.
	 * =================================================================== */

	for (size_t sample = 0; sample < numSamples; sample++) {

		p0i = AudioSource[sample];
		p0o = (a0*p0i + a1*p1i + a2*p2i) - (b2*p2o + b1*p1o);

		p2i = p1i; p1i = p0i;
		p2o = p1o; p1o = p0o;

		AudioTarget[sample] = p0o;
	}
}


void ButterworthFilter::updateLowPassCoefficients(const double CenterFrequency) {
    
    //-- Compute the coeff for the given center frequency.
    lp_c  = 1.0 / tan(_pi * CenterFrequency / samplingRate);
    lp_cc = lp_c * lp_c;
    lp_a0 = 1.0 / (1.0 + (_sqrt2 * lp_c) + lp_cc);
    lp_a1 = lp_a0 * 2.0;
    lp_a2 = lp_a0;
    lp_b1 = lp_a0 * 2.0 * (1.0 - lp_cc);
    lp_b2 = lp_a0 * (1.0 - (_sqrt2 * lp_c) + lp_cc);

    //-- Store this center frequency, 
    //-- so that it is not recomputed.
    CenterFreq_lp = CenterFrequency;
}


void ButterworthFilter::updateHighPassCoefficients(const double CenterFrequency) {
    
    //-- Compute the coeff for the given center frequency.
    hp_c  =  tan(_pi * CenterFrequency / samplingRate);
    hp_cc =  hp_c * hp_c;
    hp_a0 =  1.0 / (1.0 + (_sqrt2 * hp_c) + hp_cc);
    hp_a1 = -hp_a0 * 2.0;
    hp_a2 =  hp_a0;
    hp_b1 =  hp_a0 * 2.0 * (hp_cc - 1.0);
    hp_b2 =  hp_a0 * (1.0 - (_sqrt2 * hp_c) + hp_cc);

    //-- Store this center frequency, 
    //-- so that it is not recomputed.
    CenterFreq_hp = CenterFrequency;
}


void ButterworthFilter::updateBandPassCoefficients(const double CenterFrequency) {

    //-- Compute the coeff for the given center frequency.
    bp_bw =  q_scale;
    bp_c  =  1.0 / tan( _pi * (bp_bw / samplingRate) );
    bp_d  =  2.0 * cos( 2.0 * _pi * (CenterFrequency / samplingRate) );
    bp_a0 =  1.0 / (bp_c + 1.0);
    bp_a1 =  0.0;
    bp_a2 = -bp_a0;
    bp_b1 = -bp_a0 * bp_c * bp_d;
    bp_b2 =  bp_a0 * (bp_c - 1.0);

    //-- Store this center frequency, 
    //-- so that it is not recomputed.
    CenterFreq_bp = CenterFrequency;
}


void ButterworthFilter::updateBandNochCoefficients(const double CenterFrequency) {

    //-- Compute the coeff for the given center frequency.
    bn_bw =  q_scale;
    bn_c  =  tan( _pi * (bn_bw / samplingRate) );
    bn_d  =  2.0 * cos( 2.0 * _pi * (CenterFrequency / samplingRate) );
    bn_a0 =  1.0 / (1.0 + bn_c);
    bn_a1 = -bn_a0 * bn_d;
    bn_a2 =  bn_a0;
    bn_b1 = -bn_a0 * bn_d;
    bn_b2 =  bn_a0 * (1.0 - bn_c);

    //-- Store this center frequency, 
    //-- so that it is not recomputed.
    CenterFreq_bn = CenterFrequency;
}

