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
 * @file  beamformer.cpp
 * @brief Implementation of the interaural cues calculator (see header file).
 * =========================================================================== */

#include <iCub/filters/beamformer.h>

using namespace Filters;

Beamformer::Beamformer(int mics, double dist, double c, int rate, int samples, int bands, int beamsPerHemi, int resolution) : 
	numMics(mics), 
    micDistance(dist),
    speedOfSound(c),
	samplingRate(rate), 
	numFrameSamples(samples), 
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


Beamformer::~Beamformer() {
	
}


void Beamformer::getBeamformedAudio(const yarp::sig::Matrix& FilterBank, yarp::sig::Matrix& BeamformedAudio) {

    //-- Ensure space is allocated for this.
	BeamformedAudio.resize(numBands * numBeams, numFrameSamples);

    //-- Loop variabes.
    int band, beam, sample;

    #ifdef WITH_OMP
    #pragma omp parallel \
     private (band, beam, sample)
    #pragma omp for schedule(guided)
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
                    FilterBank[ch1][AudioUtil::myMod_int(sample + offset, numFrameSamples)]
                );
            }
        }
    }
}

/*
void Beamformer::getBeamformedRmsAudio(const yarp::sig::Matrix& FilterBank, yarp::sig::Matrix& BeamformedAudio) {

    //-- Ensure space is allocated for this.
	BeamformedAudio.resize(numBands, numBeams);

    //-- Loop variabes.
    int band, beam, sample;

    #ifdef WITH_OMP
    #pragma omp parallel \
     private (band, beam, sample)
    #pragma omp for schedule(guided)
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
                    FilterBank[ch1][AudioUtil::myMod_int(sample + offset, numFrameSamples)],
                    2.0
                );
            }

            //-- Take the root of the mean.
            BeamformedAudio[band][beam] = sqrt( beamSum / (double) numFrameSamples );
        }
    }
}
*/


void Beamformer::getBeamformedRmsPower(const yarp::sig::Matrix& BeamformedAudio, yarp::sig::Matrix& BeamPower) {

    //-- Ensure space is allocated for the powermap.
	BeamPower.resize(numBands, numMics);

    //-- Iterate through and take the rms of all beams for a particular band.
	for (int band = 0; band < numBands; band++) {

		//-- Set the current band to 0 before beginning.
		double bandSum = 0.0;

		//-- Sum the squares of the provided filter bank.
		for (int beam = 0; beam < numBeams; beam++) {
			bandSum += pow(BeamformedAudio[band][beam], 2.0);
		}

		//-- Take the root of the mean.
		BeamPower[band][0] = sqrt( bandSum / (double) numBeams );

        //-- Fill the rest of the columns.
        for (int mic = 1; mic < numMics; mic++) {
            BeamPower[band][mic] = BeamPower[band][0];
        }
	}
}


void Beamformer::getAngleNormalAudioMap(const yarp::sig::Matrix& BeamformedRmsAudio, yarp::sig::Matrix& AngleNormalAudio, const double Offset) {

    /* ===========================================================================
     *  Step 1) Interpolate the rms of the beamformed audio to get 
     *            angle normal audio map of the front field.
     * =========================================================================== */
    interpolateFrontFieldBeamsRms (
        /* Source = */ BeamformedRmsAudio,
        /* Target = */ frontFieldAudioMap
    );

    /* ===========================================================================
     *  Step 2) Mirror the front field onto the back field. To produce an
     *            Egocentric audio map, the offset should be zero always.
     *            For Allocentric the offset should be relative to the head angle.
     * =========================================================================== */
    mirrorFrontField (
        /* Source = */ frontFieldAudioMap,
        /* Target = */ AngleNormalAudio,
        /* Offset = */ AudioUtil::myRound(Offset * angleResolution) 
    );
}


inline double Beamformer::lininterp(const double x, const double x1, const double y1, const double x2, const double y2) { 
    return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1); 
}


void Beamformer::interpolateFrontFieldBeamsRms(const yarp::sig::Matrix& BeamformedRmsAudio, yarp::sig::Matrix& FrontFieldAudio) {

    //-- Ensure space is allocated for the audio map.
    FrontFieldAudio.resize(numBands, numFrontFieldAngles);

    //-- Iterate through and linearly interpolate the non-linear beams across linearly spaced angles.
    int band, beam, realAngle;
    
    #ifdef WITH_OMP
    #pragma omp parallel \
     private (band, beam, realAngle)
    #pragma omp for schedule(guided)
    #endif
    for (band = 0; band < numBands; band++) {

        beam = 0;
        for (realAngle = 0; realAngle < numFrontFieldAngles; realAngle++) {

            if (frontFieldRealAngles[realAngle] > frontFieldBeamAngles[beam+1] && beam+2 < numBeams) {
                   beam += 1;
            }

            FrontFieldAudio[band][realAngle] = lininterp (
                /* x  = */ frontFieldRealAngles[realAngle],
                /* x1 = */ frontFieldBeamAngles[beam],
                /* y1 = */ BeamformedRmsAudio[band][beam],
                /* x2 = */ frontFieldBeamAngles[beam+1],
                /* y2 = */ BeamformedRmsAudio[band][beam+1]
            );

        }
    }
}


void Beamformer::mirrorFrontField(const yarp::sig::Matrix& FrontFieldAudio, yarp::sig::Matrix& FullFieldAudio, const int offset) {

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
            idx0 = AudioUtil::myMod_int(half_length + index + offset, numFullFieldAngles);
            idx1 = AudioUtil::myMod_int(half_length - index + offset, numFullFieldAngles);

            FullFieldAudio[row][idx0] = FrontFieldAudio[row][index];
            FullFieldAudio[row][idx1] = FrontFieldAudio[row][index];   
        }

        for (index = half_length+1; index < full_length; index++) {

            //-- Start at middle and far right. Move towards eachother.
            idx0 = AudioUtil::myMod_int(half_length  + index + offset, numFullFieldAngles);
            idx1 = AudioUtil::myMod_int(two_and_half - index + offset, numFullFieldAngles);

            FullFieldAudio[row][idx0] = FrontFieldAudio[row][index];
            FullFieldAudio[row][idx1] = FrontFieldAudio[row][index];   
        }
    }  
}


void Beamformer::setFrontFieldBeamAngles() {

    //-- Ensure space is allocated for the angle positions.
    frontFieldBeamAngles.resize(numBeams);
    
    //-- Generate the angles at which each beam is pointed.
	for (int beam = 0; beam < numBeams; beam++) {

		frontFieldBeamAngles[beam] = (1.0 / micDistance) * (-numBeamsPerHemifield + beam) * (speedOfSound / (double) samplingRate);
		frontFieldBeamAngles[beam] = (frontFieldBeamAngles[beam] <= -1.0) ? -1.0 : frontFieldBeamAngles[beam];  //-- Make sure we are in 
		frontFieldBeamAngles[beam] = (frontFieldBeamAngles[beam] >=  1.0) ?  1.0 : frontFieldBeamAngles[beam];  //-- range to avoid NAN.
		frontFieldBeamAngles[beam] = asin(frontFieldBeamAngles[beam]);
	}
}


void Beamformer::setFrontFieldRealAngles() {

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