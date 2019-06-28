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
 * @file  audioStreamerPeriodicThread.h
 * @brief Definition of a periodic thread that receives an allocentric map
 *          of the auditory environment, and builds a Bayesian map over time.
 * =========================================================================== */

#ifndef _AUDIO_STREAMER_PERIODICTHREAD_H_
#define _AUDIO_STREAMER_PERIODICTHREAD_H_

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

#include <iCub/util/AudioUtil.h>

class AudioStreamerPeriodicThread : public yarp::os::PeriodicThread {

  private:

	bool result;                //-- Result of the processing.

	std::string name;           //-- Rootname of all the ports opened by this thread.
	std::string robot;          //-- Name of the robot.
	std::string configFile;     //-- Name of the configFile where the parameter of the camera are set.
	std::string inputPortName;  //-- Name of input port for incoming events, typically from aexGrabber.

	yarp::os::Stamp timeStamp;  //-- Time stamp updated by yarp network.

	double startTime;           //-- Used for keeping time and reporting temporal
    double stopTime;            //-- events to the user via command line.
	
	double timeDelay,        totalDelay;        //-- Hold on to and store time
	double timeReading,      totalReading;	    //-- events for clean display
	double timeProcessing,   totalProcessing;   //-- at the end of a loop.
	double timeTransmission, totalTransmission; //-- Include stats on the average execution
	double timeTotal,        totalTime;         //-- when the RFModule is closed.

	int    totalIterations;

	/* ===========================================================================
	 *  Yarp Ports for Sending and Receiving Data from this Periodic Thread.
	 * =========================================================================== */
	yarp::os::BufferedPort< yarp::sig::Sound > outRawAudioPort;
	yarp::os::BufferedPort< yarp::os::Bottle > inHeadAnglePort;

	yarp::sig::Sound  RawAudio;
	yarp::sig::Vector Encoder;
	yarp::sig::Vector Position;
	yarp::sig::Vector Speed;
	

	/* ===========================================================================
	 *  Yarp Devices.
	 * =========================================================================== */
	yarp::dev::PolyDriver         *robotHead;
	yarp::dev::IPositionControl   *robotPos;
	yarp::dev::IVelocityControl   *robotVel;
	yarp::dev::IEncoders          *robotEnc;
	
	yarp::dev::PolyDriver         *robotMic;
	yarp::dev::IAudioGrabberSound *robotSound;


	/* ===========================================================================
	 *  Variables received from the resource finder.
	 * =========================================================================== */
	int    panAngle;
	int    numMics;
	double minDegree;
	double maxDegree;
	double motorSpeed;
	
	bool   movements;
	
	int    samplingRate;
	int    numFrameSamples;
	

  public:

	/* ===========================================================================
	 *  Default Constructor.
	 * =========================================================================== */
	AudioStreamerPeriodicThread();


	/* ===========================================================================
	 *  Main Constructor.
	 * 
	 * @param robotname  : Name of the robot.
	 * @param configFile : Path to the .ini configuration file.
	 * =========================================================================== */
	AudioStreamerPeriodicThread(std::string robotname, std::string configFile);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~AudioStreamerPeriodicThread();


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
	 *    that are going to be created by the thread.
	 * 
	 * @param str : rootname.
	 * =========================================================================== */
	void setName(std::string str);
	

	/* ===========================================================================
	 *  Function that returns the original root name and 
	 *    appends another string iff passed as parameter.
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


	/* ===========================================================================
	 *  Method for moving the robots head to some degree.
	 * 
	 * @param target : the target degree of the robots head.
	 * @param reply  : a response to be added to the bottle.
	 * =========================================================================== */
	void moveRobotHead(const double target, std::string& reply);


  private:

	/* ===========================================================================
	 *  Write data to out going ports if something is connected.
	 * =========================================================================== */
	void publishOutPorts();


	/* ===========================================================================
	 *  Produce execution stats when the thread is interrupted.
	 * =========================================================================== */
	void endOfProcessingStats();
};

#endif  //_AUDIO_BAYESIAN_MAP_PERIODICTHREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
