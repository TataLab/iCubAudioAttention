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
 * @brief Implementation of the interaural cues calculator (see header file).
 * =========================================================================== */

#include <iCub/interauralCues.h>

inline int    myMod(int a, int b) { return a >= 0 ? a % b : ((a % b) + b) % b; }
inline double lininterp(double x, double x1, double y1, double x2, double y2) { return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1); }

InterauralCues::InterauralCues(int mics, int dist, int c, int samples, int frames, int bands, int beamsPerHemi, int resolution) : 
	numMics(mics), 
    micDistance(dist),
    speedOfSound(c),
	samplingRate(samples), 
	numFrameSamples(frames), 
	numBands(bands),
    numBeamsPerHemifield(beamsPerHemi),
    angleResolution(resolution) {

    numBeams            = 2 * numBeamsPerHemifield + 1;
    numFrontFieldAngles = _baseAngles * angleResolution + 1;
    numFullFieldAngles  = _baseAngles * angleResolution * 2;

    frontFieldAudioMap.resize(numBands, numFrontFieldAngles);

    setFrontFieldBeamAngles();
    setFrontFieldRealAngles();
}


InterauralCues::~InterauralCues() {
	
}


void InterauralCues::getBeamformedAudio(yarp::sig::Matrix& BeamformedAudio, const yarp::sig::Matrix& FilterBank) {

    //-- Ensure space is allocated for this.
	BeamformedAudio.resize(numBands * numBeams, numFrameSamples);

    //-- Loop variabes.
    int band, beam, sample;

    #ifdef WITH_OMP
    # pragma omp parallel \
      shared  (BeamformedAudio, FilterBank, numBands, numBeams, numBeamsPerHemifield, numFrameSamples) \
      private (band, beam, sample)

    # pragma omp for schedule(guided)
    #endif
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

    #ifdef WITH_OMP
    # pragma omp parallel \
      shared  (BeamformedAudio, FilterBank, numBands, numBeams, numBeamsPerHemifield, numFrameSamples) \
      private (band, beam, sample)

    # pragma omp for schedule(guided)
    #endif
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


void InterauralCues::getAngleNormalAudioMap(yarp::sig::Matrix& AngleNormalAudio, const yarp::sig::Matrix& BeamformedRmsAudio, const int Offset) {

    /* ===========================================================================
     *  Step 1) Interpolate the rms of the beamformed audio to get 
     *           angle normal audio map of the front field.
     * =========================================================================== */
    interpolateFrontFieldBeamsRms (
        /* Target = */ frontFieldAudioMap,
        /* Source = */ BeamformedRmsAudio
    );

    /* ===========================================================================
     *  Step 2) Mirror the front field onto the back field. To produce an
     *           Egocentric audio map, the offset should be zero always.
     *           For Allocentric the offset should be relative to the head angle.
     * =========================================================================== */
    mirrorFrontField (
        /* Target = */ AngleNormalAudio,
        /* Source = */ frontFieldAudioMap,
        /* Offset = */ Offset
    );
}


void InterauralCues::interpolateFrontFieldBeamsRms(yarp::sig::Matrix& FrontFieldAudio, const yarp::sig::Matrix& BeamformedRmsAudio) {

    //-- Ensure space is allocated for the audio map.
    FrontFieldAudio.resize(numBands, numFrontFieldAngles);

    //-- Iterate through and linearly interpolate the non-linear beams across linearly spaced angles.
    int band, beam, realAngle;

    #ifdef WITH_OMP
    #pragma omp parallel \
     shared  (FrontFieldAudio, BeamformedRmsAudio, frontFieldRealAngles, frontFieldBeamAngles, numBands, numFrontFieldAngles) \
     private (band, beam, realAngle)

    #pragma omp for schedule(guided)
    #endif
    for (band = 0; band < numBands; band++) {

        beam = 0;
        for (realAngle = 0; realAngle < numFrontFieldAngles; realAngle++) {

            if (frontFieldRealAngles[realAngle] > frontFieldBeamAngles[beam+1]) {
                beam += 1;
            }

            FrontFieldAudio[band][realAngle] = lininterp (
                /* x  = */ frontFieldRealAngles[realAngle],
                /* x1 = */ frontFieldBeamAngles[beam],
                /* y1 = */ BeamformedRmsAudio[band][beam],
                /* x2 = */ frontFieldBeamAngles[beam],
                /* y2 = */ BeamformedRmsAudio[band][beam]
            );
        }
    }
}


void InterauralCues::mirrorFrontField(yarp::sig::Matrix& FullFieldAudio, const yarp::sig::Matrix& FrontFieldAudio, const int offset) {

    //-- Get the number of rows for the front field audio.
    const int numRow = FrontFieldAudio.rows();

    //-- Ensure space is allocated for the audio map.
    FullFieldAudio.resize(numRow, numFullFieldAngles);

    //-- Store some commonly used indicies.
    const int full_length  = numFrontFieldAngles;
    const int half_length  = numFrontFieldAngles / 2; //-- floor.
    const int two_and_half = full_length + full_length + half_length - 2;
    int idx0, idx1, row, index;

    //-- Iterate through mirroring the indicies of the front field onto the back field.
    for (row = 0; row < numRow; row++) {
        for (index = 0; index <= half_length; index++) {

            //-- Both start in middle. idx0 moves right, idx1 moves left.
            idx0 = myMod(half_length + index + offset, numFullFieldAngles);
            idx1 = myMod(half_length - index + offset, numFullFieldAngles);

            FullFieldAudio[row][idx0] = FrontFieldAudio[row][index];
            FullFieldAudio[row][idx1] = FrontFieldAudio[row][index];   
        }

        for (index = half_length+1; index < full_length; index++) {

            idx0 = myMod(half_length  + index + offset, numFullFieldAngles);
            idx1 = myMod(two_and_half - index + offset, numFullFieldAngles);

            FullFieldAudio[row][idx0] = FrontFieldAudio[row][index];
            FullFieldAudio[row][idx1] = FrontFieldAudio[row][index];   
        }
    }  
}




void InterauralCues::setFrontFieldBeamAngles() {

    //-- Ensure space is allocated for the angle positions.
    frontFieldBeamAngles.resize(numBeams);
    
    //-- Generate the angles at which each beam is pointed.
	for (int beam = 0; beam < numBeams; beam++) {
		frontFieldBeamAngles[beam] = (1.0 / micDistance) * (-numBeamsPerHemifield + beam) * (speedOfSound / samplingRate);
		frontFieldBeamAngles[beam] = (frontFieldBeamAngles[beam] <= -1.0) ? -1.0 : frontFieldBeamAngles[beam];  //-- Make sure we are in 
		frontFieldBeamAngles[beam] = (frontFieldBeamAngles[beam] >=  1.0) ?  1.0 : frontFieldBeamAngles[beam];  //-- range to avoid NAN.
		frontFieldBeamAngles[beam] = asin(frontFieldBeamAngles[beam]);
	}
}


void InterauralCues::setFrontFieldRealAngles() {

    //-- Ensure space is allocated for the angle positions.
    frontFieldRealAngles.resize(numFrontFieldAngles);

    //-- Generate the linear spaced angles.
    double linspace_step = ((_pi / 2.0) - (-_pi / 2.0)) / (numFrontFieldAngles - 1.0);
	double current_step  = (-_pi / 2.0);

    for (int angle = 0; angle < numFrontFieldAngles; angle++) {
        frontFieldRealAngles[angle] = current_step;
        current_step += linspace_step;
    }
}