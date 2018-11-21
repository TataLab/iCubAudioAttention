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
 * @file  util.cpp
 * @brief Implementation utility functions (see header file).
 * =========================================================================== */

#include <iCub/util/util.h>

using namespace Util; 

//template<class T>
//inline int myRound(T a) {
//    /* ================================================================
//     *  Custom round to ensure negative numbers are rounded properly.
//     * ================================================================ */
//    int ceilValue  = (int)ceil(a);
//    int floorValue = (int)floor(a);
//    return (a - floorValue <= 0.5) ? floorValue : ceilValue;
//}
//
//
//inline double myMod_double(double a, double b) { 
//    /* ================================================================
//     *  Allows modulos operation for doubles.
//     * ================================================================ */
//    return ( ( a ) - ( b ) * floor ( ( a ) / ( b ) ) ); 
//}
//
//
//inline int myMod_int(int a, int b) { 
//    /* ================================================================
//     *  Allows correct modulos operation for negative numbers.
//     * ================================================================ */
//    return a >= 0 ? a % b : ((a % b) + b) % b; 
//}
//
//
//inline void downSampleMatrix(const yMatrix& source, yMatrix& target, const size_t downSamp) {
//    /* ================================================================
//     *  Down sample the columns of a yarp matrix. 
//     *  - Note: The number of rows is same as source.
//     * ================================================================ */
//
//    //-- Don't do any down sampling. Standard Copy.
//    if (downSamp == 1) { target = source; return; }
//
//    //-- Allocate space for the down sampled matrix.
//    const size_t RowSize = source.rows();
//    const size_t ColSize = source.cols() / downSamp;
//    const size_t offset  = source.cols() % downSamp;
//    target.resize(RowSize, ColSize);
//
//    //-- Set a pointer to the target.
//    double *trg = target.data();
//
//    //-- Set a pointer to the source.
//    const double *src = source.data();
//
//    for (size_t row = 0; row < RowSize; row++) {
//        for (size_t col = 0; col < ColSize; col++) {
//            //-- Dereference and copy data.
//            *trg = *src;
//            trg++; src += downSamp;
//        }
//        //-- Skip any remaining points at the end of the row.
//        src += offset;
//    }
//}
//
//
////inline void MatrixToImageOfFloat(const yMatrix& source, yImageOfFloat& target) {
//    /* ================================================================
//     *  Convert a yarp matrix into an image of floats.
//     *  - Note: This was used for easier sending of matrices
//     *           to python for prototyping.
//     * ================================================================ */
///*
//    //-- Allocate space for the Image.
//    const size_t RowSize = source.rows();
//    const size_t ColSize = source.cols();
//    target.resize(ColSize, RowSize);
//
//    //-- Set a pointer to the data.
//    const double *src = source.data();
//
//    //-- Iterate through copying the contents
//    //-- from the matrix, into the target image.
//    //-- Matrix holds doubles. Conversion to
//    //-- floats should be ok.
//    for (size_t row = 0; row < RowSize; row++) {
//        for (size_t col = 0; col < ColSize; col++) {
//            target(col, row) = *src;
//            src++;
//        }
//    }
//}
//*/
//
//
//inline void ones(yMatrix& mat) {
//    /* ================================================================
//     *  Fill a yarp matrix with ones.
//     * ================================================================ */
//    const size_t RowSize = mat.rows();
//    const size_t ColSize = mat.cols();
//    double*      m       = mat.data();
//    for (size_t r = 0; r < RowSize; r++) {
//        for (size_t c = 0; c < ColSize; c++) {
//            *m = 1.0; m++;
//        }
//    }
//}
//

//inline void makeTimeStamp(double& totalStat, double& timeStat, double& startTime, double& stopTime) {
//    /* ================================================================
//     *  General purpose time stamper. Updates start and stop time.
//     * ================================================================ */
//    stopTime   = yarp::os::Time::now();
//    timeStat   = stopTime - startTime;
//    totalStat += timeStat;
//    startTime  = stopTime;
//}