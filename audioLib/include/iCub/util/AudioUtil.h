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
 * @file  AudioUtil.h
 * @brief Implementation of general utility functions and
 *         inlined macros for this repository.
 * =========================================================================== */

#ifndef _AUDIO_UTIL_H
#define _AUDIO_UTIL_H_

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
typedef yarp::sig::ImageOf< yarp::sig::PixelFloat > yImageOfFloat;

namespace AudioUtil {

    /* ================================================================
     *  Down sample the columns of a yarp matrix. 
     *  - Note: The number of rows is same as source.
     * ================================================================ */
    void downSampleMatrix(const yMatrix& source, yMatrix& target, const size_t downSamp);


    /* ================================================================
     *  General purpose time stamper. Updates start and stop time.
     * ================================================================ */
    void makeTimeStamp(double& totalStat, double& timeStat, double& startTime, double& stopTime);


    /* ================================================================
     *  Convert a yarp matrix into an image of floats.
     *  - Note: This was used for easier sending of matrices
     *           to python for prototyping.
     * ================================================================ */
    void MatrixToImageOfFloat(const yMatrix& source, yImageOfFloat& target);


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
}

#endif  //_AUDIO_UTIL_H

//----- end-of-file --- ( next line intentionally left blank ) ------------------
