// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Marko Ilievski
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
 * @file  audioMemoryMapperRateThread.h
 * @brief Header file of the memory mapper ratethread.
 *        This is where the processing happens.
 */

#ifndef _AUDIO_MEMORY_MAPPER_RATETHREAD_H_
#define _AUDIO_MEMORY_MAPPER_RATETHREAD_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Log.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <time.h>

// Memory mapping requirements
#include <sys/mman.h>
#include <fcntl.h>
#include <string>

const float normDivid = pow(2,23);  // Number that is used to convert the integer number received
                                    // as the audio signal and convert it to a double audio signal


class audioMemoryMapperRateThread : public yarp::os::RateThread {

 private:
	bool result;                    // result of the processing
	std::string name;               // rootname of all the ports opened by this thread
	std::string robot;              // name of the robot
	std::string configFile;         // name of the configFile where the parameter of the camera are set
	std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber

	yarp::os::BufferedPort<yarp::sig::Sound>   rawAudioPort;
	yarp::os::BufferedPort<yarp::sig::Matrix>  gammaToneAudioPort;
	yarp::os::BufferedPort<yarp::sig::Matrix>  beamFormedAudioPort;
	yarp::os::BufferedPort<yarp::sig::Matrix>  audioMapEgoPort;
	yarp::os::BufferedPort<yarp::sig::Matrix>  audioMapAloPort;
	yarp::os::BufferedPort<yarp::sig::Matrix>  longTermBayesianMapPort;
	yarp::os::BufferedPort<yarp::sig::Matrix>  collapesedBayesianMapPort;

	yarp::os::Stamp ts;
	yarp::sig::Sound*  rawAudioSoundObj;
	yarp::sig::Matrix* gammaToneAudioMatrix;
	yarp::sig::Matrix* beamFormedAudioMatrix;
	yarp::sig::Matrix* audioMapEgoMatrix;
	yarp::sig::Matrix* audioMapAloMatrix;
	yarp::sig::Matrix* longTermBayesianMatrix;
	yarp::sig::Matrix* collapesedBayesianMatrix;

	bool  rawAudioPortActive;
	bool  gammaToneAudioPortActive;
	bool  beamFormedAudioPortActive;
	bool  audioMapEgoPortActive;
	bool  audioMapAloPortActive;
	bool  longTermBayesianMapPortActive;
	bool  collapesedBayesianMapPortActive;

	std::string  rawAudioPortName;
	std::string  gammaToneAudioPortName;
	std::string  beamFormedAudioPortName;
	std::string  audioMapEgoPortName;
	std::string  audioMapAloPortName;
	std::string  longTermBayesianMapPortName;
	std::string  collapesedBayesianMapPortName;

	FILE *rawAudioFid;
	int rawAudioMappedFileID;
	double *rawAudioData;
	double *initializationRawAudioArray;

	FILE *gammaToneAudioFid;
	int gammaToneAudioFileID;
	double *gammaToneAudioData;
	double *initializationGammaToneAudioArray;

	FILE *beamFormedAudioFid;
	int beamFormedAudioFileID;
	double *beamFormedAudioData;
	double *initializationBeamFormedAudioArray;

	FILE *audioMapEgoFid;
	int audioMapEgoFileID;
	double *audioMapEgoData;
	double *initializationAudioMapEgoArray;

	FILE *audioMapAloFid;
	int audioMapAloFileID;
	double *audioMapAloData;
	double *initializationAudioMapAloArray;

	FILE *longTermBayesianMapFid;
	int longTermBayesianMapFileID;
	double *longTermBayesianMapData;
	double *initializationLongTermBayesianMapArray;

	FILE *collapesedBayesianMapFid;
	int collapesedBayesianMapFileID;
	double *collapesedBayesianMapData;
	double *initializationCollapesedBayesianMapArray;

	int frameSamples;
	int nBands;
	int nMics;
	int interpolateNSamples;
	double micDistance;
	int C;
	int samplingRate;
	int longTimeFrame;
	int nBeamsPerHemi;
	int totalBeams;


	void createMemoryMappingSection();

	/**
	 *  loadFile
	 *
	 *  Accesses the loadFile.xml that is found in the root directory of this
	 *  module and load all required parameters for the beam former.
	 *
	 *  @param rf : resource finder object containing the values of presets
	 */
	void loadFile(yarp::os::ResourceFinder &rf);

	void memoryMapRawAudio();
	void memoryMapGammaToneAudio();
	void memoryMapBeamFormedAudio();
	void memoryMapAudioMapEgo();
	void memoryMapAudioMapAlo();
	void memoryMapLongTermBayesianMap();
	void memoryMapCollapesedBayesianMap();


 public:
	/**
	 *  constructor
	 */
	audioMemoryMapperRateThread();


	/**
	 *  constructor
	 *
	 *  @param robotname : name of the robot
	 *  @param        rf : resource finder object
	 */
	audioMemoryMapperRateThread(std::string robotname, yarp::os::ResourceFinder &rf);


	/**
	 *  destructor
	 */
	~audioMemoryMapperRateThread();


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
	 *  @param str rootnma
	 */
	void setName(std::string str);


	/**
	 *  getName
	 *
	 *  function that returns the original root name and appends another string iff passed as parameter
	 *
	 *  @param p pointer to the string that has to be added
	 *
	 *  @return rootname
	 */
	std::string getName(const char* p);


	/**
	 *	processing
	 *
	 *  method for the processing in the ratethread
	 */
	bool processing();
};

#endif  //_AUDIO_PREPROCESSER_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
