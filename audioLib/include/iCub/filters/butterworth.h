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
 * @file  butterworth.h
 * @brief Definition of a butterworth filter that receives raw audio data 
 *          from and applies different butterworth filters to it.
 * =========================================================================== */

#ifndef _BUTTERWORTH_FILTER_H_
#define _BUTTERWORTH_FILTER_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/Log.h>

#ifdef WITH_OMP
#include <omp.h>
#endif

typedef yarp::sig::Matrix yMatrix;

namespace Filters {
    class Butterworth;
} 


class Filters::Butterworth {

  private:
	
	/* ===========================================================================
	 *  Variables received from the constructor.
	 * =========================================================================== */
	int samplingRate;
    int q_scale;
	
    double CenterFreq_lp;
    double CenterFreq_hp;
    double CenterFreq_bp;
    double CenterFreq_bn;

	
	/* ===========================================================================
	 *  Constant variables.
	 * =========================================================================== */
	const double _pi    = 2 * acos(0.0); //-- High precision pi.
    const double _sqrt2 = sqrt(2.0);     //-- High precision sqrt(2.0)

    /* ===========================================================================
	 *  Filter Coefficients.
	 * =========================================================================== */
    double lp_c, lp_cc, lp_a0, lp_a1, lp_a2, lp_b1, lp_b2;       //-- Low-Pass.
    double hp_c, hp_cc, hp_a0, hp_a1, hp_a2, hp_b1, hp_b2;       //-- High-Pass.
    double bp_bw, bp_c, bp_d, bp_a0, bp_a1, bp_a2, bp_b1, bp_b2; //-- Band-Pass.
    double bn_bw, bn_c, bn_d, bn_a0, bn_a1, bn_a2, bn_b1, bn_b2; //-- Band-Noch.


  public:

	/* ===========================================================================
	 *  Default Constructor.
	 * =========================================================================== */
	Butterworth(int rate, int q);


    /* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~Butterworth();


    /* ===========================================================================
     *  Apply a low pass filter on the provided audio source, for some
     *    specified center frequency.
     * 
     * @param AudioSource     : Source for the input audio. 
     * @param AudioTarget     : Target for the low passed audio.
     * @param CenterFrequency : Center frequency for the low pass.
     * =========================================================================== */
    void getLowPassedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency);


    /* ===========================================================================
     *  Apply a high pass filter on the provided audio source, for some
     *    specified center frequency.
     * 
     * @param AudioSource     : Source for the input audio. 
     * @param AudioTarget     : Target for the high passed audio.
     * @param CenterFrequency : Center frequency for the high pass.
     * =========================================================================== */
    void getHighPassedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency);


    /* ===========================================================================
     *   Apply a band pass filter on the provided audio source, for some
     *    specified center frequency.
     * 
     * @param AudioSource     : Source for the input audio. 
     * @param AudioTarget     : Target for the band passed audio.
     * @param CenterFrequency : Center frequency for the band pass.
     * =========================================================================== */
    void getBandPassedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency);


    /* ===========================================================================
     *  Apply a band noch filter on the provided audio source, for some
     *    specified center frequency.
     * 
     * @param AudioSource     : Source for the input audio. 
     * @param AudioTarget     : Target for the band noched audio.
     * @param CenterFrequency : Center frequency for the band noch.
     * =========================================================================== */
    void getBandNochedAudio(const yMatrix& AudioSource, yMatrix& AudioTarget, const double CenterFrequency);


  private:

    /* ===========================================================================
     *  Perform a generic bilinear transformation on the audio source, the 
     *    coefficients are provided as input so this function may be generally
     *    used among low pass, high pass, band pass and band noch filters.
     * 
     * @param AudioSource        : Source of input audio.
     * @param AudioTarget        : Target for the filtered audio.
     * @param numSamples         : Lenght of the source and target.
     * @param a0, a1, a2, b1, b2 : Filter coefficients of caller filter.
     * =========================================================================== */
    void bilinearTransformation(const double* AudioSource, double* AudioTarget, const size_t numSamples, const double a0, const double a1, const double a2, const double b1, const double b2);


    /* ===========================================================================
     *  Update the internally stored coefficients for a low pass filter 
     *    based on the provided center frequency.
     * =========================================================================== */
    void updateLowPassCoefficients(const double CenterFrequency);


    /* ===========================================================================
     *  Update the internally stored coefficients for a high pass filter 
     *    based on the provided center frequency.
     * =========================================================================== */
    void updateHighPassCoefficients(const double CenterFrequency);


    /* ===========================================================================
     *  Update the internally stored coefficients for a band pass filter 
     *    based on the provided center frequency.
     * =========================================================================== */
    void updateBandPassCoefficients(const double CenterFrequency);


    /* ===========================================================================
     *  Update the internally stored coefficients for a band noch filter 
     *    based on the provided center frequency.
     * =========================================================================== */
    void updateBandNochCoefficients(const double CenterFrequency);
};


#endif  //_BUTTERWORTH_FILTER_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
