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
 * @file  hilbertTransform.cpp
 * @brief Implementation of the hilbert transform (see header file).
 * =========================================================================== */

#include <iCub/hilbertTransform.h>


HilbertTransform::HilbertTransform(int rows, int samples) :
    numRows(rows),
    numFrameSamples(samples) {

    //-- Allocate a matrix of fftw complex.
    fft_input = (fftw_complex **) fftw_malloc(numRows * sizeof(fftw_complex *));
    for (int idx = 0; idx < numRows; idx++) {
        fft_input[idx] = (fftw_complex *) fftw_malloc(numFrameSamples * sizeof(fftw_complex));
    }

    fft_output = (fftw_complex **) fftw_malloc(numRows * sizeof(fftw_complex *));
    for (int idx = 0; idx < numRows; idx++) {
        fft_output[idx] = (fftw_complex *) fftw_malloc(numFrameSamples * sizeof(fftw_complex));
    }

    //-- Allocate the plans as a vector so they are reused.
    for (int idx = 0; idx < numRows; idx++) {
        plan_forward.push_back ( fftw_plan_dft_1d(numFrameSamples, fft_input[idx],  fft_output[idx], FFTW_FORWARD,  FFTW_ESTIMATE) );
        plan_backward.push_back( fftw_plan_dft_1d(numFrameSamples, fft_output[idx], fft_output[idx], FFTW_BACKWARD, FFTW_ESTIMATE) );
    }
}

HilbertTransform::~HilbertTransform() {

    //-- Free the memory for the complex objects.
    for (int idx = 0; idx < numRows; idx++) {
        fftw_free(fft_input[idx]);
        fftw_free(fft_output[idx]);
    }

    fftw_free(fft_input);
    fftw_free(fft_output);

    //-- Destroy the fftw plans.
    for (int idx = 0; idx < numRows; idx++) {
        fftw_destroy_plan(plan_forward[idx]);
        fftw_destroy_plan(plan_backward[idx]);
    }

    //-- General clean up.
    fftw_cleanup();
}


void HilbertTransform::getHilbertEnvelope(const yarp::sig::Matrix& source, yarp::sig::Matrix& target) {

    //-- Get the size of the source.
    const size_t numMatRows = source.rows(); //assert(numMatRows == numRows);         //TODO: USE ASSERTS??
    const size_t numMatCols = source.cols(); //assert(numMatCols == numFrameSamples); //TODO: USE ASSERTS??

    //-- Reshape the target to ensure space is allocated.
    target.resize(numMatRows, numMatCols);

    size_t row;

    #ifdef WITH_OMP
    #pragma omp parallel \
     private (row)
    #pragma omp for schedule(guided)
    #endif
    for (row = 0; row < numMatRows; row++) {

        RealToComplex(source[row], fft_input[row], numMatCols);

        singleAnalyticalSignal(fft_input[row], fft_output[row], numMatCols, plan_forward[row], plan_backward[row]);

        ComplexToReal(fft_output[row], target[row], numMatCols);
    } 
}


void HilbertTransform::singleAnalyticalSignal(const fftw_complex* source, fftw_complex* target, const size_t numSamples, fftw_plan& forward, fftw_plan& backward) {

    //-- Get some information of the signals length.
    size_t halfNumSamples = numSamples >> 1;
    size_t numRemaining   = halfNumSamples;

    //-- Perform fft on the source.
    fftw_execute(forward);

    //-- First half of the signals imaginary and real 
    //-- parts are doubled. 
    for (size_t idx = 1; idx < halfNumSamples; idx++) {
        target[idx][REAL] *= 2.0;
        target[idx][IMAG] *= 2.0;
    }

    //-- One less remainder when even number of samples.
    if (numSamples % 2 == 0) {
        numRemaining--;
    }

    //-- If the number of samples was odd, and 
    //-- greator than one, double the half position too.
    else if (numSamples > 1) {
        target[halfNumSamples][REAL] *= 2.0;
        target[halfNumSamples][IMAG] *= 2.0;
    }

    //-- Drop the second half of the signal.
    memset(&target[halfNumSamples+1][REAL], 0.0, numRemaining * sizeof(fftw_complex));

    //-- Perform ifft on the target.
    fftw_execute(backward);

    //-- Scale the idft output.
    for (size_t idx = 0; idx < numSamples; idx++) {
        target[idx][REAL] /= numSamples;
        target[idx][IMAG] /= numSamples;
    }
}


inline void HilbertTransform::RealToComplex(const double* source, fftw_complex* target, const size_t numSamples) {

    //-- Iterate through and ``drop`` the negative half of the frequency
    //-- spectrum, effectively turning the signal into a complex one.
    for (size_t idx = 0; idx < numSamples; idx++) {
        target[idx][REAL] = source[idx];
        target[idx][IMAG] = 0.0;
    }
}


inline void HilbertTransform::ComplexToReal(const fftw_complex* source, double* target, const size_t numSamples) {

    //-- Iterate through and convert the data to a real number.
    for (size_t idx = 0; idx < numSamples; idx++) {
        target[idx] = c2r(source[idx]);
    }
}


inline double HilbertTransform::c2r(const fftw_complex x) {
    //-- u = sqrt( ur^2 + ui^2 )
    return sqrt( x[REAL]*x[REAL] + x[IMAG]*x[IMAG] );
}
