// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2018  Department of Neuroscience - University of Lethbridge
  * Author: Austin Kothig, Matt Tata
  * email: kothiga@uleth.ca matthew.tata@uleth.ca
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

/**
 * @file audioPowerMapRatethread.h
 * @brief Header file of the power map ratethread.
 *        This is where the processing happens.
 */

#ifndef _AUDIO_POWER_MAP_RATETHREAD_H_
#define _AUDIO_POWER_MAP_RATETHREAD_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Log.h>

#include <iostream>
#include <queue>
#include <vector>
#include <time.h>

#define THRATE 80 //ms

class AudioPowerMapRatethread : public yarp::os::RateThread {

 private:

    //--
    //-- Name Strings.
    //--
    std::string name;               // rootname of all the ports opened by this thread
    std::string robot;              // name of the robot
    std::string configFile;         // name of the configFile where the parameter of the camera are set
    std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber


    //--
    //-- Incoming and Outgoing Ports.
    //--
    yarp::os::BufferedPort<yarp::sig::Matrix> *inBandsPowerPort;
    yarp::os::BufferedPort<yarp::sig::Matrix> *inBayesMapPort;
    yarp::os::BufferedPort<yarp::sig::Matrix> *outBayesPowerPort;
    yarp::os::BufferedPort<yarp::sig::Matrix> *outBayesPowerAnglePort;
    yarp::os::BufferedPort<yarp::sig::Matrix> *outProbabilityPowerPort;
    yarp::os::BufferedPort<yarp::sig::Matrix> *outBayesProbabilityPowerPort;
    yarp::os::BufferedPort<yarp::sig::Matrix> *outBayesProbabilityPowerAnglePort;

    yarp::os::Stamp ts;   // time stamper
    

    //--
    //-- Containers for Matrix and Maps.
    //--
    yarp::sig::Matrix *inBandsPowerMatrix;
    yarp::sig::Matrix *inBayesMapMatrix;
    yarp::sig::Matrix *outBayesPowerMatrix;
    yarp::sig::Matrix *outBayesPowerAngleMatrix;
    yarp::sig::Matrix *outProbabilityPowerMatrix;
    yarp::sig::Matrix *outBayesProbabilityPowerMatrix;
    yarp::sig::Matrix *outBayesProbabilityPowerAngleMatrix;

    std::vector <double>                 currentBandPowerMap;
	std::vector < std::vector <double> > currentBayesMap;
    std::vector < std::vector <double> > currentBayesPowerMap;
    std::vector <double>                 currentBayesPowerAngleMap;
    std::vector <double>                 currentProbabilityPowerMap;
    std::vector < std::vector <double> > currentBayesProbabilityPowerMap;
    std::vector <double>                 currentBayesProbabilityPowerAngleMap;
    std::queue < std::vector <double> >  bufferedPowerMap;
	

    //--
    //-- Memory Mapping Variables.
    //--
    int interpolateNSamples;
    int nAngles;
	int nBands;
	int nMics;
    int bufferSize;
	int noiseBufferMap;
	int numberOfNoiseMaps;

	int totalBeams;

	int TimeFrame;

	double nBeamsPerHemi;

	std::string fileName;

    //--
    //-- Variables Need to time the Update Module.
    //--
    long mtime, seconds, useconds;
	double startTime, stopTime;


 public:

    /**
     *  Default Constructor.
     */
    AudioPowerMapRatethread();

    /**
     *  Constructor.
     * 
     *  Set robotname and configFile, then calls loadFile
     *  on the passed in resource finder.
     * 
     *  @param _robotname  : name of the robot
     *  @param _configFile : configuration file
     *  @param rf          : resource finder object for setting constants
     */
    AudioPowerMapRatethread(std::string _robotname, std::string _configFile, yarp::os::ResourceFinder &rf);


    /**
     *  Destructor.
     */
    ~AudioPowerMapRatethread();


    /**
     *  threadInit
     * 
     *  Initialize the threads.
     * 
     *  @return Whether or not initialization executed successfully.
     */
    bool threadInit();


    /**
     *  threadRelease
     * 
     *  Correctly releases all threads.
     */
    void threadRelease();


    /**
     *  run
     * 
     *  Active part of the thread.
     */
    void run();


    /**
	 *  setName
	 *
	 *  Function that sets the rootname of all the ports that are going to be created by the thread.
	 *
	 *  @param str : the rootname used for all ports opened by this thread
	 */
	void setName(std::string str);


    /**
	 *  getName
	 *
	 *  Function that returns the original root name and appends another string iff passed as parameter.
	 *
	 *  @param p : pointer to the string that has to be added
	 *
	 *  @return rootname
	 */
	std::string getName(const char* p);


    /**
	 *  processing
	 *
	 *  Method for the processing in the ratethread.
	 *
	 *  @return whether processing was successful
	 */
	bool processing();


 private:

	/**
	 *  loadFile
	 *
	 *  Accesses the loadFile.xml that is found in the root directory of this
	 *  module and load all required parameters for the beam former.
	 *
	 *  @param rf : resource finder object containing the values of presets
	 */
	void loadFile(yarp::os::ResourceFinder &rf);

    void setInputBayesMap();
    void setInputBandPower();


    void updatePowerProbability();
    void removeMap(std::vector <double> &probabilityPowerMap, std::vector <double> oldPowerMap);
    void addMap(std::vector <double> &probabilityPowerMap, std::vector <double> newPowerMap);

    void normalizeVector(std::vector<double> &targetVector);

    void combineBayesPower(std::vector < std::vector <double> > &targetMap, std::vector < std::vector <double> > &bandAngleMap, std::vector <double> &bandMap);
    void collapseBayesPower(std::vector <double> &targetMap, std::vector < std::vector <double> > &sourceMap);

    void sendBayesPower();
    void sendBayesPowerAngle();
    void sendProbabilityPower();
    void sendBayesProbabilityPower();
    void sendBayesProbabilityPowerAngle();
};

#endif  //_AUDIO_POWER_MAP_RATETHREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
