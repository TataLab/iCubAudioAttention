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
 * @file  audioPreprocessorPeriodicThread.h
 * @brief Definition of a periodic thread that receives raw audio data 
 *         from an input port and applies various signal processing to it.
 * =========================================================================== */

#ifndef _AUDIO_PREPROCESSOR_PERIODICTHREAD_H_
#define _AUDIO_PREPROCESSOR_PERIODICTHREAD_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/os/Log.h>

#include <iCub/gammatoneFilterBank.h>
#include <iCub/interauralCues.h>

class AudioPreprocessorPeriodicThread : public yarp::os::PeriodicThread {

  private:

	bool result;                //-- Result of the processing.

	std::string name;           //-- Rootname of all the ports opened by this thread.
	std::string robot;          //-- Name of the robot.
	std::string configFile;     //-- Name of the configFile where the parameter of the camera are set.
	std::string inputPortName;  //-- Name of input port for incoming events, typically from aexGrabber.

	yarp::os::Stamp timeStamp;  //-- Time stamp updated by yarp network.

	/* ===========================================================================
	 *  Yarp Ports for Sending and Receiving Data from this Periodic Thread.
	 * =========================================================================== */
	yarp::os::BufferedPort< yarp::sig::Sound  > inRawAudioPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outGammatoneFilteredAudioPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outGammatoneFilteredPowerPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outBeamformedAudioPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outBeamformedRmsAudioPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outBeamformedRmsPowerPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outAllocentricAudioPort;

	/* ===========================================================================
	 *  Yarp Matrices used for Modules Computation. 
	 *   Objects passed around to encapsulated objects.
	 * =========================================================================== */
	yarp::sig::Sound* inputSound;
	yarp::sig::Matrix RawAudioMatrix;
	yarp::sig::Matrix GammatoneFilteredAudioMatrix;
	yarp::sig::Matrix GammatoneFilteredPowerMatrix;
	yarp::sig::Matrix BeamformedAudioMatrix;
	yarp::sig::Matrix BeamformedRmsAudioMatrix;
	yarp::sig::Matrix BeamformedRmsPowerMatrix;
	yarp::sig::Matrix AllocentricAudioMatrix;

	/* ===========================================================================
	 *  Encapsulated objects to perform processing.
	 * =========================================================================== */
	GammatoneFilterBank* gammatoneFilterBank;
	InterauralCues*      interauralCues;

	/* ===========================================================================
	 *  Variables received from the resource finder.
	 * =========================================================================== */
	int    panAngle;
	int    numMics;
	double micDistance;

	double speedOfSound;
	int    samplingRate;
	int    numFrameSamples;

	int    numBands;
	int    lowCf;
	int    highCf;
	bool   halfRec;
	bool   erbSpaced;
	int    angleRes;

	/* ===========================================================================
	 *  Derive variables from resource finders variables.
	 * =========================================================================== */
	int numBeamsPerHemifield;
	int numBeams;
    int numFrontFieldAngles;
    int numFullFieldAngles;

	/* ===========================================================================
	 *  Constant variables.
	 * =========================================================================== */
	const double _norm       = pow(2,15);     //-- Used to convert received int to double.
	const double _pi         = 2 * acos(0.0); //-- High precision pi.
	const int    _baseAngles = 180;           //-- Base number of angles in front field.


  public:

	/* ===========================================================================
	 *  Default Constructor.
	 * =========================================================================== */
	AudioPreprocessorPeriodicThread();


	/* ===========================================================================
	 *  Main Constructor.
	 * 
	 * @param robotname  : Name of the robot.
	 * @param configFile : Path to the .ini configuration file.
	 * =========================================================================== */
	AudioPreprocessorPeriodicThread(std::string robotname, std::string configFile);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~AudioPreprocessorPeriodicThread();


	/* ===========================================================================
	 *  Configure all the parameters and return true if successful.
	 * 
	 * @param rf : Reference to the resource finder.
	 * 
	 * @return Flag for the success.
	 * =========================================================================== */
	bool configure(yarp::os::ResourceFinder &rf);


	/* ===========================================================================
	 *  Initialises the thread. Build tables and open ports.
	 * =========================================================================== */
	bool threadInit();


	/* ===========================================================================
	 *  Correctly releases the thread.
	 * =========================================================================== */
	void threadRelease();


	/* ===========================================================================
	 *  Function that sets the rootname of all the ports 
	 *   that are going to be created by the thread.
	 * 
	 * @param str : rootname.
	 * =========================================================================== */
	void setName(std::string str);
	

	/* ===========================================================================
	 *  Function that returns the original root name and 
	 *   appends another string iff passed as parameter.
	 * 
	 * @param p : pointer to the string that has to be added.
	 * 
	 * @return rootname + appended string.
	 * =========================================================================== */
	std::string getName(const char* p);


	/* ===========================================================================
	 *  Function that sets the inputPort name.
	 * =========================================================================== */
	void setInputPortName(std::string inpPrtName);


	/* ===========================================================================
	 *  Active part of the thread.
	 * =========================================================================== */
	void run(); 


	/* ===========================================================================
	 *  Method for the processing in the PeriodicThread.
	 * =========================================================================== */
	bool processing();
};

#endif  //_AUDIO_PREPROCESSOR_PERIODICTHREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
