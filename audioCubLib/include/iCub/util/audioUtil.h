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
 * @file  AudioUtil.h
 * @brief Implementation of general utility functions and
 *         inlined macros for this repository.
 * =========================================================================== */

#ifndef _AUDIO_UTIL_H
#define _AUDIO_UTIL_H_

#include <iostream>
#include <fstream>
#include <cstdlib>
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
typedef yarp::sig::ImageOf< yarp::sig::PixelFloat > yImageOfFloat;
typedef std::reference_wrapper< short int > ySample;


namespace AudioUtil {

    /* ================================================================
     *  Down sample the columns of a yarp matrix. 
     *  - Note: The number of rows is same as source.
     * ================================================================ */
    void downSampleMatrix(const yMatrix& source, yMatrix& target, const size_t downSamp, const std::string flag);
    void downSampleMatrix(const yMatrix& source, yMatrix& target, const size_t downSamp);
    yMatrix downSampleMatrix(const yMatrix& source, const size_t downSamp, const std::string flag);
    yMatrix downSampleMatrix(const yMatrix& source, const size_t downSamp);


    /* ================================================================
     *  Take a string with environment variables and return the 
     *   expanded path.
     * ================================================================ */
    std::string expandEnvironmentVariables(const std::string filename);


    /* ================================================================
     *  Convert a number to a string prefixed by some num leading zeros.
     * ================================================================ */
    std::string leadingZeros(const int value, const unsigned int leading);


    /* ================================================================
     *  Call a system function for making directories. Returns sucess.
     * ================================================================ */
    bool makeDirectory(const std::string path);


    /* ================================================================
     *  General purpose time stamper. Updates start and stop time.
     * ================================================================ */
    void makeTimeStamp(double& totalStat, double& timeStat, double& startTime, double& stopTime);


    /* ================================================================
     *  Save a yarp matrix to a file.
     * ================================================================ */
    void MatrixToFile(const yMatrix& source, const std::string fileName);


    /* ================================================================
     *  Convert a yarp matrix into an image of floats.
     *  - Note: This was used for easier sending of matrices
     *           to python for prototyping.
     * ================================================================ */
    void MatrixToImageOfFloat(const yMatrix& source, yImageOfFloat& target);

    
    /* ================================================================
     *  Convert a yarp matrix to a string. Faster than internal implementation.
     * ================================================================ */
    std::string MatrixToString(const yMatrix& source, const int precision, const int width, const char* endRowStr);


    /* ================================================================ 
     *  Allows modulos operation for doubles.
     * ================================================================ */
    double myMod_double(const double a, const double b);


    /* ================================================================
     *  Allows correct modulos operation for negative numbers.
     * ================================================================ */
    int myMod_int(const int a, const int b);


    /* ================================================================
     *  Custom round to ensure negative numbers are rounded properly.
     * ================================================================ */
    int myRound(const double a);


    /* ================================================================
     *  Fill a yarp matrix with ones.
     * ================================================================ */
    void ones(yMatrix& mat);


    /* ================================================================
     *  Find the root mean square of each row in the source matrix.
     *  The resulting target will have a shape of (x,y) where the 
     *    number of rows in source =  x * y.
     * 
     * @param source : Source of matrix. Expected Shape: [x*y, numCols].
     * @param target : Target of rms'd matrix. Resulting Shape: [x, y].
     * @param x      : Number of rows for target matrix.
     * @param y      : Number of columns for target matrix.
     * ================================================================ */
    void RootMeanSquareMatrix(const yMatrix& source, yMatrix& target, const size_t x, const size_t y);


    /* ================================================================
     *  Convert a yarp sound object into a yarp matrix.
     * ================================================================ */
    void SoundToMatrix(const yarp::sig::Sound* source, yMatrix& target, const double sampleNormaliser);


    /* ================================================================
     *  Slide a window over some source matrix. 
     * 
     * @param source       : Source of matrix. 
     * @param target       : Target of windowd matrix.
     * @param windowLength : The length of each window.
     * @param hopLength    : How far to shift the window.
     * @param flag         : Operation to do on each window ( mean | rms ).
     * ================================================================ */
    void WindowMatrix(const yMatrix& source, yMatrix& target, const size_t windowLength, const size_t hopLength, const std::string flag);
}

#endif  //_AUDIO_UTIL_H

//----- end-of-file --- ( next line intentionally left blank ) ------------------
