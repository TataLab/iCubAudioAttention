// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Marko Ilievski, Austin Kothig
  * email: m.ilievski@uleth.ca, matthew.tata@uleth.ca, francesco.rea@iit.it
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
 * @file  audioBayesianRatethread.cpp
 * @brief Implementation of the bayesian ratethread.
 *        This is where the bayesian probability is processed.
 */

#ifndef _BAYESIAN_RATETHREAD_H_
#define _BAYESIAN_RATETHREAD_H_

#include <yarp/dev/all.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IPositionControl.h>

#include <yarp/os/all.h>
#include <yarp/os/Log.h>
#include <yarp/os/RateThread.h>

#include <yarp/sig/all.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>

#include <iostream>
#include <queue>
#include <vector>

#define THRATE 80 //ms
inline int    myModed(int a, int b) { return  a >= 0 ? a % b : (a % b) + b; }
inline double myABS  (double a)     { return  a >= 0 ? a : ((a) * -1);      }

class AudioBayesianRatethread : public yarp::os::RateThread {

 private:
	//
	// name strings
	//
	std::string configFile;         // name of the configFile where the parameter of the camera are set
	std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber
	std::string name;               // rootname of all the ports opened by this thread
	std::string robot;              // name of the robot
	const std::string noiseMapPath = "./noiseMap.dat";

	//
	// Incoming and Outgoing Ports
    yarp::os::BufferedPort<yarp::os::Bottle > *headAngleInPort;
	yarp::os::BufferedPort<yarp::sig::Matrix> *inPort;
	yarp::os::Port *outPort;
	yarp::os::BufferedPort<yarp::sig::Matrix> *outProbabilityPort;

	yarp::os::Stamp ts;                // time stamper


	//
	// containers for matrix and maps
	//
	yarp::sig::Matrix* inputMatrix;
	yarp::sig::Matrix* outputMatrix;
	yarp::sig::Vector* outProbabilityMap;
    yarp::os::Bottle* headAngleBottle;

	std::vector <double> longProbabilityAngleMap;

	std::vector < std::vector <double> > currentAudioMap;
	std::vector < std::vector <double> > longMap;
	std::vector < std::vector <double> > mediumMap;
	std::vector < std::vector <double> > noiseMap;
	std::vector < std::vector <double> > probabilityMap;
	std::vector < std::vector <double> > shortMap;


	std::queue <double> bufferedOffSet;
	std::queue < std::vector < std::vector <double> > > bufferedMap;


	//
	// Memory mapping variables
	//
	int interpolateNSamples;
	int nBands;
	int nMics;
	int noiseBufferMap;
	int numberOfNoiseMaps;


	int totalBeams;

	int longTimeFrame;
	int mediumTimeFrame;
	int shortTimeFrame;

	double nBeamsPerHemi;
	double offset;

	bool first;

	std::string fileName;

	//
	// Variables need to time the update module
	//
	struct timeval st, en;
	long mtime, seconds, useconds;
	double startTime, stopTime;

 public:
	/**
	 *  constructor
	 */
	AudioBayesianRatethread();


	/**
	 *  constructor
	 *
	 *  set robotname and configFile, then calls loadFile
	 *	on the passed in resource finder
	 *
	 *  @param   _robotname : name of the robot
	 *  @param  _configFile : configuration file
	 *  @param           rf : resource finder object for setting constants
	 */
	AudioBayesianRatethread(std::string _robotname, std::string _configFile, yarp::os::ResourceFinder &rf);


	/**
	 *  destructor
	 */
	~AudioBayesianRatethread();


	/**
	 *  threadInit
	 *
	 *  initialises the thread
	 *
	 *  @return whether or not initialization executed correctly
	 */
	bool threadInit();


	/**
	 *  threadRelease
	 *
	 *  correctly releases the thread
	 */
	void threadRelease();


	/**
	 *  run
	 *
	 *  active part of the thread
	 */
	void run();


	/**
	 *  setName
	 *
	 *  function that sets the rootname of all the ports that are going to be created by the thread
	 *
	 *  @param str : the rootname used for all ports opened by this thread
	 */
	void setName(std::string str);


	/**
	 *  getName
	 *
	 *  function that returns the original root name and appends another string iff passed as parameter
	 *
	 *  @param p : pointer to the string that has to be added
	 *
	 *  @return rootname
	 */
	std::string getName(const char* p);


	/**
	 *  setInputPortName
	 *
	 *  function that sets the inputPort name
	 *
	 *  @param inPrtName : the name to set input ports to
	 */
	void setInputPortName(std::string inpPrtName);


	/**
	 *  processing
	 *
	 *  method for the processing in the ratethread
	 *
	 *  @return whether processing was successful
	 */
	bool processing();


    //
    // why are these two functions . . .
    //

	/**
	 *  processing
	 *
	 *  method for the processing in the ratethread
	 *
	 *  @param mat : matrix to be processed in the method
	 */
	bool processing(yarp::sig::Matrix *mat);


	/**
	 *  processing
	 *
	 *  method for the processing in the ratethread
	 *
	 *  @param b : bottle to be processed in the method
	 */
	bool processing(yarp::os::Bottle *b);


	/**
	 *  getLongProbabilityMap
	 *
	 *  method for getting the long probability map
	 *
	 *  @return the long probability map
	 */
	std::vector < std::vector <double> > getLongProbabilityMap();


	/**
	 *  getMediumProbabilityMap
	 *
	 *  method for getting the medium probability map
	 *
	 *	@return the medium probability map
	 */
	std::vector < std::vector <double> > getMediumProbabilityMap();


	/**
	 *  getShortProbabilityMap
	 *
	 *	method for getting the short probability map
	 *
	 *	@return the short probability map
	 */
	std::vector < std::vector <double> > getShortProbabilityMap();


 private:
	/**
	 *  findPeaks
	 *
	 *  This function will look at the vector probabilityMap which is
	 *  passed in as input and find the peaks saving them to peakMap.
	 *
	 *	@param        peakMap : will contain ones to mark the peaks
	 *                          found and zeros everywhere else.
	 *  @param probabilityMap : a map of the auditory sean with probabilities
	 *                          that a given sound is at a given angle.
	 */
	void findPeaks(std::vector<double> &peakMap, const std::vector<double> &probabilityMap);


	/**
	 *  setAcousticMap
	 *
	 *  sets the acoustic map
	 */
	void setAcousticMap();


	/**
	 *  normalizePropabilityMap
	 *
	 *  @param probabilityMap : a map of the auditory sean with probabilities
	 *                          that a given sound is at a given angle.
	 */
	void normalizePropabilityMap(std::vector <std::vector <double> > &probabilityMap);


	/**
	 *  loadFile
	 *
	 *  Accesses the loadFile.xml that is found in the root directory of this
	 *  module and load all required parameters for the beam former.
	 *
	 *  @param rf : resource finder object containing the values of presets
	 */
	void loadFile(yarp::os::ResourceFinder &rf);


	/**
	 *  calcOffset
	 *
	 *  Calculates the off set that is needed to convert the Ego acoustic map into an Alo acoustic map.
	 */
	void calcOffset();


	/**
	 *  sendAudioMap
	 *
	 *  Sends the audio map VIA the output port provided.
	 *
	 *  @param probabilityMap : a map of the auditory sean with probabilities
	 *                          that a given sound is at a given angle.
	 */
	void sendAudioMap(std::vector <std::vector <double> > &probabilityMap);

	/**
	 *  sendProbabilityMap
	 *
	 *  Sends the probability map VIA the output port provided.
	 *
	 *  @param outputProbabilityMap : a map of the auditory sean with probabilities
	 *                                that a given sound is at a given angle.
	 */
	void sendProbabilityMap(std::vector <double> &outputProbabilityMap);


	/**
	 *  createBaysianMaps
	 *
	 *	create the probability map with baysian probability
	 */
	void createBaysianMaps();


	/**
	 *  createBaysianMaps
	 *
	 *	create a noise map
	 */
	void createNoiseMaps();

	void removeNoise(std::vector <std::vector <double>> &probabilityMap);
	void removeMap(std::vector <std::vector <double>> &probabilityMap, std::vector <std::vector <double>> &cAM);
	void addMap(std::vector <std::vector <double>> &probabilityMap, std::vector <std::vector <double>> &cAM);
	void collapseMap(const std::vector <std::vector <double>> &inputMap, std::vector <double> &outputProbabilityMap);
};

#endif  //_AUDIO_BAYESIAN_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
