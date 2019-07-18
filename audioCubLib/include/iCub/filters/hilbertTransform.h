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
 * @file  hilbertTransform.h
 * @brief Object for processing the hilbert transform for 
 *         finding the analytical signal.
 * =========================================================================== */

#ifndef _HILBERT_TRANSFORM_H_
#define _HILBERT_TRANSFORM_H_

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>

#ifdef WITH_OMP
#include <omp.h>
#endif

#include <fftw3.h>
#include <vector>

#include <iCub/util/audioUtil.h>

namespace Filters {
    class HilbertTransform;
} 


class Filters::HilbertTransform { 

  private:
    
	/* ===========================================================================
	 *  Variables set in the constructor. Should not be changed for life of object.
	 * =========================================================================== */
	int numRows;
	int numFrameSamples;

	/* ===========================================================================
	 *  Constant variables.
	 * =========================================================================== */
    const double _pi = 2 * acos(0.0); //-- High precision pi.
	const int    REAL=0,   IMAG=1;    //-- Enumerators.

	/* ===========================================================================
	 *  FFTW Plans and Objects.
	 * =========================================================================== */
	std::vector< fftw_plan > plan_forward;
	std::vector< fftw_plan > plan_backward;
	fftw_complex**           fft_input;
	fftw_complex**           fft_output;


  public:

	/* ===========================================================================
	 *  Main Constructor.
	 * =========================================================================== */
	HilbertTransform(int rows, int samples);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~HilbertTransform();
	

    /* ===========================================================================
	 *  Find the hilbert envelope by performing a fast fourier transformation, 
     *    phase shifting, inverse fast fourier transformation, and finally taking
     *    the absolute value to get the real signal, which is the envelope.
	 * 
	 * @param source : Real siganl. 
	 * @param target : Real envelope of the signal.
	 * =========================================================================== */
    void getHilbertEnvelope(const yarp::sig::Matrix& source, yarp::sig::Matrix& target);

	//void getAnalyticalSignal(const yarp::sig::Matrix& source, fftw_complex** target);

	
  private:
	
	//void getAnalyticalSignal(const yarp::sig::Matrix& source);
	void singleAnalyticalSignal(const fftw_complex* source, fftw_complex* target, const size_t numSamples, fftw_plan& forward, fftw_plan& backward);
	//void singleAnalyticalSignal(const fftw_complex* source, fftw_complex* target, const size_t numSamples);

	void RealToComplex(const double* source, fftw_complex* target, const size_t numSamples);
	void ComplexToReal(const fftw_complex* source, double* target, const size_t numSamples);
	double c2r(const fftw_complex x);
	

};

#endif //_HILBERT_TRANSFORM_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
