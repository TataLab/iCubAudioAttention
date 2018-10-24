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
 * @file  interauralCues.h
 * @brief Object for processing filtered audio into various 
 *         different interaural cues (ITD, ILD, IPD, etc.).
 * =========================================================================== */

#ifndef _INTERAURAL_CUES_H_
#define _INTERAURAL_CUES_H_

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>

class InterauralCues { 

  private:
    
	/* ===========================================================================
	 *  Variables set in the constructor. Should not be changed for life of object.
	 * =========================================================================== */
    int    numMics;
	int    micDistance;
	double speedOfSound;
	int    samplingRate;
	int    numFrameSamples;
	int    numBands;
	int    numBeamsPerHemifield;

    const double _pi = 2 * acos(0.0); //-- High precision pi.
    double numBeams;
	

  public:

	/* ===========================================================================
	 *  Main Constructor.
	 * 
	 * @param mics         : Number of Microphones.
	 * @param dist         : Distance Between Microphones.
	 * @param c            : Speed of Sound.
	 * @param samples      : Number of Samples Recorded per Second.
	 * @param frames       : Number of Samples in incoming Frame.
     * @param bands        : Number of Frequency Bands.
     * @param beamsPerHemi : Number of Beams in a single hemi field.
	 * =========================================================================== */
	InterauralCues(int mics, int dist, int c, int samples, int frames, int bands, int beamsPerHemi);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~InterauralCues();


    /* ===========================================================================
	 *  Generate the Interaural Time Difference by applying simple
	 *   delay and sum beamforming onto the provided filter bank.
	 * 
	 * @param BeamformedAudio : Target for the beamformed audio (number of bands * number of beams, numFrameSamples)
     * @param FilterBank      : The filtered audio (number of mics * number of bands, number of samples).
	 * =========================================================================== */
    void getBeamformedAudio(yarp::sig::Matrix& BeamformedAudio, const yarp::sig::Matrix& FilterBank);


	/* ===========================================================================
	 *  Generate the Interaural Time Difference by applying simple 
	 *   delay and sum beamforming with RMS applied across frames 
	 *   onto the provided filter bank.
	 * 
	 * @param BeamformedAudio : Target for the beamformed audio (number of bands, number of beams).
     * @param FilterBank      : The filtered audio (number of mics * number of bands, number of samples).
	 * =========================================================================== */
    void getBeamformedRmsAudio(yarp::sig::Matrix& BeamformedAudio, const yarp::sig::Matrix& FilterBank);


	/* ===========================================================================
	 *  Find the total RMS power for each band across beams.
	 * 
	 * @param BankPower  : Target for the power of each band (number of bands).
     * @param FilterBank : The filtered audio (number of mics * number of bands, number of samples).
	 * =========================================================================== */
    void getBeamformedRmsPower(yarp::sig::Matrix& BeamPower, const yarp::sig::Matrix& );


  private:
	
	;


};

#endif //_INTERAURAL_CUES_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
