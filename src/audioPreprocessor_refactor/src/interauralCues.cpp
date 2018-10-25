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

inline int myMod(int a, int b) { return  a >= 0 ? a % b : ((a % b) + b) % b; }

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

    //-- Ensure space is allocated for this.
	BeamformedAudio.resize(numBands * numBeams, numFrameSamples);

    //-- Loop variabes.
    int band, beam, sample;

    //-- TODO: Uncomment when ready to multi-thread.
    /*
    # pragma omp parallel \
      shared  (BeamformedAudio, FilterBank, numBands, numBeams, numBeamsPerHemifield, numFrameSamples) \
      private (band, beam, sample)

    # pragma omp for schedule(guided)
    */
    for (band = 0; band < numBands; band++) {

        //-- Set the location of the channels.
        int ch0 = band;
        int ch1 = band + numBands;

        for (beam = 0; beam < numBeams; beam++) {

            //-- Reduce redundant computation.
            int itrBeam = beam + (band * numBeams); 
            int offset  = beam - numBeamsPerHemifield;

            for (sample = 0; sample < numFrameSamples; sample++) {
                BeamformedAudio[itrBeam][sample] = (
                    FilterBank[ch0][sample] + 
                    FilterBank[ch1][myMod(sample + offset, numFrameSamples)]
                );
            }
        }
    }
}


void InterauralCues::getBeamformedRmsAudio(yarp::sig::Matrix& BeamformedAudio, const yarp::sig::Matrix& FilterBank) {
    
    //-- Ensure space is allocated for this.
	BeamformedAudio.resize(numBands, numBeams);

    //-- Loop variabes.
    int band, beam, sample;

    //-- TODO: Uncomment when ready to multi-thread.
    /*
    # pragma omp parallel \
      shared  (BeamformedAudio, FilterBank, numBands, numBeams, numBeamsPerHemifield, numFrameSamples) \
      private (band, beam, sample)

    # pragma omp for schedule(guided)
    */
    for (band = 0; band < numBands; band++) {

        //-- Set the location of the channels.
        int ch0 = band;
        int ch1 = band + numBands;

        for (beam = 0; beam < numBeams; beam++) {

            //-- Reduce redundant computation.
            int offset     = beam - numBeamsPerHemifield;
            double beamSum = 0.0;

            //-- Sum the squares of the provided filter bank.
            for (sample = 0; sample < numFrameSamples; sample++) {
                beamSum += pow (
                    FilterBank[ch0][sample] + 
                    FilterBank[ch1][myMod(sample + offset, numFrameSamples)],
                    2.0
                );
            }

            //-- Take the root of the mean.
            BeamformedAudio[band][beam] = sqrt( beamSum / (double) numFrameSamples );
        }
    }
}


void InterauralCues::getBeamformedRmsPower(yarp::sig::Matrix& BeamPower, const yarp::sig::Matrix& BeamformedAudio) {

    //-- Ensure space is allocated for the powermap.
	BeamPower.resize(2, numBands);

    //-- Iterate through and take the rms of all beams for a particular band.
	for (int band = 0; band < numBands; band++) {

		//-- Set the current band to 0 before beginning.
		double bandSum = 0.0;

		//-- Sum the squares of the provided filter bank.
		for (int beam = 0; beam < numBeams; beam++) {
			bandSum += pow(BeamformedAudio[band][beam], 2.0);
		}

		//-- Take the root of the mean.
		BeamPower[0][band] = BeamPower[1][band] = sqrt( bandSum / (double) numBeams );
	}
}
