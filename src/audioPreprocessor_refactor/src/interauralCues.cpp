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
 * @file  interauralCues.cpp
 * @brief Implementation of the gammatone filter bank (see header file).
 * =========================================================================== */

#include <iCub/interauralCues.h>

InterauralCues::InterauralCues(int mics, int dist, int c, int samples, int frames, int bands, int beamsPerHemi) : 
	numMics(mics), 
    micDistance(dist),
    speedOfSound(c),
	samplingRate(samples), 
	numFrameSamples(frames), 
	numBands(bands),
    numBeamsPerHemifield(beamsPerHemi) {

    numBeams = 2 * numBeamsPerHemifield + 1;
}


InterauralCues::~InterauralCues() {
	
}


void InterauralCues::getBeamformedAudio(yarp::sig::Matrix& BeamformedAudio, const yarp::sig::Matrix& FilterBank) {

}


void InterauralCues::getBeamformedRmsAudio(yarp::sig::Matrix& BeamformedAudio, const yarp::sig::Matrix& FilterBank) {

}


void InterauralCues::getBeamformedRmsPower(yarp::sig::Matrix& BeamPower, const yarp::sig::Matrix& ) {

}
