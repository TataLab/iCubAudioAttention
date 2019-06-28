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
 *          different interaural cues (ITD, ILD, IPD, etc.).
 * =========================================================================== */

#ifndef _INTERAURAL_CUES_H_
#define _INTERAURAL_CUES_H_

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>

#ifdef WITH_OMP
#include <omp.h>
#endif

#include <iCub/util/audioUtil.h>

class InterauralCues { 

  private:
    
	/* ===========================================================================
	 *  Variables set in the constructor. Should not be changed for life of object.
	 * =========================================================================== */
    int    numMics;
	double micDistance;
	double speedOfSound;
	int    samplingRate;
	int    numFrameSamples;
	int    numBands;
	int    numBeamsPerHemifield;
	int    angleResolution;

	/* ===========================================================================
	 *  Derive variables from constructor.
	 * =========================================================================== */
    int numBeams;
	int numFrontFieldAngles;
	int numFullFieldAngles;

	/* ===========================================================================
	 *  Constant variables.
	 * =========================================================================== */
    const double _pi         = 2 * acos(0.0); //-- High precision pi.
	const int    _baseAngles = 180;           //-- Base number of degree positions. 

	/* ===========================================================================
	 *  Yarp Matrices and Vectors used for encapsulated computation and look up tables.
	 * =========================================================================== */
	yarp::sig::Vector frontFieldBeamAngles;
	yarp::sig::Vector frontFieldRealAngles;
	yarp::sig::Matrix frontFieldAudioMap;
	

  public:

	/* ===========================================================================
	 *  Main Constructor.
	 * 
	 * @param mics         : Number of Microphones.
	 * @param dist         : Distance Between Microphones.
	 * @param c            : Speed of Sound.
	 * @param rate         : Number of Samples Recorded per Second.
	 * @param samples      : Number of Samples in incoming Frame.
     * @param bands        : Number of Frequency Bands.
     * @param beamsPerHemi : Number of Beams in a single hemi field.
	 * @param resolution   : Number of positions one degree should cover.
	 * =========================================================================== */
	InterauralCues(int mics, double dist, double c, int rate, int samples, int bands, int beamsPerHemi, int resolution);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~InterauralCues();
	

    /* ===========================================================================
	 *  Generate the Interaural Time Difference by applying simple
	 *    delay and sum beamforming onto the provided filter bank.
	 * 
	 * @param FilterBank      : The filtered audio (number of mics * number of bands, number of samples).
	 * @param BeamformedAudio : Target for the beamformed audio (number of bands * number of beams, numFrameSamples)
	 * =========================================================================== */
    void getBeamformedAudio(const yarp::sig::Matrix& FilterBank, yarp::sig::Matrix& BeamformedAudio);


	/* ===========================================================================
	 *  Generate the Interaural Time Difference by applying simple 
	 *    delay and sum beamforming with RMS applied across frames 
	 *    onto the provided filter bank.
	 * 
	 * @param FilterBank      : The filtered audio (number of mics * number of bands, number of samples).
	 * @param BeamformedAudio : Target for the beamformed audio (number of bands, number of beams).
	 * =========================================================================== */
    //void getBeamformedRmsAudio(const yarp::sig::Matrix& FilterBank, yarp::sig::Matrix& BeamformedAudio);


	/* ===========================================================================
	 *  Find the total RMS power for each band across beams.
	 * 
	 * @param BeamformedRmsAudio : The RMS of beamformed audio (number of bands, number of beams).
	 * @param BeamPower          : Target for the power of each band (number of bands, 2).
	 * =========================================================================== */
    void getBeamformedRmsPower(const yarp::sig::Matrix& BeamformedRmsAudio, yarp::sig::Matrix& BeamPower);


	/* ===========================================================================
	 *  Find the full field audio map where column spacing is linear by angle.
	 * 
	 * @param BeamformedRmsAudio : The RMS of beamformed audio (number of bands, number of beams).
	 * @param AngleNormalAudio   : Target for the angle normal audio map (number of bands, total angle positions).
	 * @param Offset             : The Offset of the head realative to zero.
	 * =========================================================================== */
	void getAngleNormalAudioMap(const yarp::sig::Matrix& BeamformedRmsAudio, yarp::sig::Matrix& AngleNormalAudio, const double Offset);


  private:
	
	/* ===========================================================================
	 *  Approximate some y, for a given x that is between (x1,y1) and (x2,y2).
	 * =========================================================================== */
	inline double lininterp(const double x, const double x1, const double y1, const double x2, const double y2);


	/* ===========================================================================
	 *  Given the Rms of the front field beams, for each frequency band 
	 *    interpolate over the beams to produce degree normal audio map
	 *    of the front field auditory scene.
	 * 
	 * @param BeamformedRmsAudio : The RMS of beamformed audio (number of bands, number of beams).
	 * @param FrontFieldAudio    : Target for angle normal audio map (number of bands, number of angle positions).
	 * =========================================================================== */
	void interpolateFrontFieldBeamsRms(const yarp::sig::Matrix& BeamformedRmsAudio, yarp::sig::Matrix& FrontFieldAudio);


	/* ===========================================================================
	 *  Given a front field auditory map, mirror it onto the back field.
	 *    the map will be offsetted by the provided int.
	 * 
	 * @param FrontFieldAudio : The angle normal audio map of the front field auditory scene (number of bands, number of angle positions).
	 * @param FullFieldAudio  : Target for full field (360 degree) audio map (number of bands, total angle positions).
	 * @param Offset          : The number of samples the auditory environment should be offset by. (-1 = offset right, 0 = no offset, 1 = offset left).
	 * =========================================================================== */
	void mirrorFrontField(const yarp::sig::Matrix& FrontFieldAudio, yarp::sig::Matrix& FullFieldAudio, const int Offset);


	/* ===========================================================================
	 *  Fills the vector frontFieldBeamAngles with the left to right 
	 *    angles (in radians), each beam is pointed.
	 * =========================================================================== */
	void setFrontFieldBeamAngles();


	/* ===========================================================================
	 *  Fills the vector frontFieldRealAngles with full front field angles (in radians).
	 * =========================================================================== */
    void setFrontFieldRealAngles();
};

#endif //_INTERAURAL_CUES_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
