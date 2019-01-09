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
 * @file  AudioUtil.cpp
 * @brief Implementation utility functions (see header file).
 * =========================================================================== */

#include <iCub/util/AudioUtil.h> 

int AudioUtil::myRound(const double a) {
    int ceilValue  = (int)ceil(a);
    int floorValue = (int)floor(a);
    return (a - floorValue <= 0.5) ? floorValue : ceilValue;
}


double AudioUtil::myMod_double(const double a, const double b) { 
    return ( ( a ) - ( b ) * floor ( ( a ) / ( b ) ) ); 
}


int AudioUtil::myMod_int(const int a, const int b) { 
    return a >= 0 ? a % b : ((a % b) + b) % b; 
}


void AudioUtil::downSampleMatrix(const yMatrix& source, yMatrix& target, const size_t downSamp) {

    //-- Don't do any down sampling. Standard Copy.
    if (downSamp == 1) { target = source; return; }

    //-- Allocate space for the down sampled matrix.
    const size_t RowSize = source.rows();
    const size_t ColSize = source.cols() / downSamp;
    const size_t offset  = source.cols() % downSamp;
    target.resize(RowSize, ColSize);

    //-- Set a pointer to the target.
    double *trg = target.data();

    //-- Set a pointer to the source.
    const double *src = source.data();

    for (size_t row = 0; row < RowSize; row++) {
        for (size_t col = 0; col < ColSize; col++) {
            //-- Dereference and copy data.
            *trg = *src;
            trg++; src += downSamp;
        }
        //-- Skip any remaining points at the end of the row.
        src += offset;
    }
}


void AudioUtil::MatrixToImageOfFloat(const yMatrix& source, yImageOfFloat& target) {

    //-- Allocate space for the Image.
    const size_t RowSize = source.rows();
    const size_t ColSize = source.cols();
    target.resize(ColSize, RowSize);

    //-- Set a pointer to the data.
    const double *src = source.data();

    //-- Iterate through copying the contents
    //-- from the matrix, into the target image.
    //-- Matrix holds doubles. Conversion to
    //-- floats should be ok.
    for (size_t row = 0; row < RowSize; row++) {
        for (size_t col = 0; col < ColSize; col++) {
            target(col, row) = *src;
            src++;
        }
    }
}


void AudioUtil::ones(yMatrix& mat) {

    const size_t RowSize = mat.rows();
    const size_t ColSize = mat.cols();
    double*      m       = mat.data();
    for (size_t r = 0; r < RowSize; r++) {
        for (size_t c = 0; c < ColSize; c++) {
            *m = 1.0; m++;
        }
    }
}


void AudioUtil::makeTimeStamp(double& totalStat, double& timeStat, double& startTime, double& stopTime) {

    stopTime   = yarp::os::Time::now();
    timeStat   = stopTime - startTime;
    totalStat += timeStat;
    startTime  = stopTime;
}


void AudioUtil::RootMeanSquareMatrix(const yMatrix& source, yMatrix& target, const size_t x, const size_t y) {
    
    //-- Get dim sizes, and ensure space is allocated.
    const size_t RowSize = source.rows(); //assert(RowSize == x * y);          //TODO: USE ASSERTS??
    const size_t ColSize = source.cols();
    target.resize(x, y);

    //-- Loop variables.
    size_t row, col;

    //TODO: Enable Threading.
    #ifdef WITH_OMP
    #pragma omp parallel \
     private (row, col)
    #pragma omp for schedule(guided)
    #endif
    for (row = 0; row < RowSize; row++) {
        
        double rowSum = 0.0;
        for (col = 0; col < ColSize; col++) {
            rowSum += (source[row][col] * source[row][col]); // s^2 
        }
        target[row / y][row % y] = sqrt( rowSum / (double) ColSize );
    }
}


 void AudioUtil::SoundToMatrix(const yarp::sig::Sound* source, yMatrix& target) {
    
    //-- Get information about the audio.
    const size_t BytesPerSample = source->getBytesPerSample();
    const size_t numSamples     = source->getSamples();
    const size_t numChannels    = source->getChannels();

    //-- Derive number of bits, and normalisation values.
    //const size_t padding        = ((numSamples % 8 == 0) ? 0 : 8 - (numSamples % 8));
    const size_t numBitsTotal   = BytesPerSample * 8;
    const double _norm          = (1 << (numBitsTotal-1));
    

    //-- Ensure appropriate space is allocated. 
    target.resize(numChannels, numSamples);

    //-- Get pointers to data memory.
    const unsigned char* src = source->getRawData();
    double*              trg = target.data();

    //-- Init a var to bit shift on.
    uint32_t current_sample = 0;

    for (int ch = 0; ch < numChannels; ch++) {
        for (int samp = 0; samp < numSamples; samp++) {
            
            //-- Reset.
            current_sample = 0;

            for (int bitshift = 0; bitshift < numBitsTotal; bitshift += 8) {
                
                //-- Bit shift the variable at source,
                //-- bitwise or with the variable.
                current_sample |= (*src << bitshift); src++;
            }

            *trg = current_sample / _norm;
            trg++;         
        }
        //src += padding;
    }
 }
