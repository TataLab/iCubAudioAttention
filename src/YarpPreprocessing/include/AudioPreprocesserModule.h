// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2013  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Francesco Rea
  * email: francesco.reak@iit.it
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
 * @file tutorialModule.h
 * @brief Simple module as tutorial.
 */



#ifndef _AUDIO_PREPROCESSER_MODULE_H_
#define _AUDIO_PREPROCESSER_MODULE_H_

#include "AudioPreprocesserRatethread.h"
#include "gammatonFilter.h"
#include "beamFormer.h"

#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/NetInt16.h>
#include <yarp/sig/Sound.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>

//Memory mapping requirements
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <sys/time.h>

const yarp::os::NetInt16 normDivid = 32768;

class AudioPreprocesserModule: public yarp::os::RFModule
{
 private:
  std::string moduleName;                  // name of the module 
  std::string robotPortName;               // name of robot port
  std::string inputPortName;               // name of the input port for events
  std::string robotName;                   // name of the robot
  std::string configFile;                  // name of the configFile that the resource Finder will seek
  AudioPreprocesserRatethread* apr;         // ratethread handling the processing in the module
  
 public:
	/**
	* default constructor
	*/
	AudioPreprocesserModule();

	/**
	* destructor
	*/
	~AudioPreprocesserModule();


	bool configure(yarp::os::ResourceFinder &rf);
	double getPeriod();
	bool updateModule();
	bool interruptModule();
	bool close();


private:
	//Colored warnings to match YARP
	std::string myerror;
	std::string myinfo;
	std::string mywarn;
	std::string myreset;

	void loadFile();
	void createMemoryMappedFile();
	void memoryMapper();
	void sendAudioMap();
	void sendGammatonFilteredAudio();
	void sendBeamFormedAudio();
	double interpl(int x, int x1, int x2, double y1, double y2);

	/**
	* SpineInterp()
	*
	*	Taking the Audio data that is found in reducedBeamFormedAudioVector. Creates an interpolation of the data corresponding to the interpellateNSamples that was specified in the xml.
	*	The data of this function will be saved in highResolutionAudioMap.
	*/
	void spineInterp();



	int interpellateNSamples;

	std::vector < std::vector < double > > highResolutionAudioMap;


	//Variables need to time the update module
	struct timeval st, en;
	long mtime, seconds, useconds;
	double oldtime;
	int lastframe;

	//Yarp Ports
	yarp::os::BufferedPort<yarp::sig::Sound> *inPort;						//Incoming Audio Data from the iCub and remoteInterface
	yarp::os::Port *audioMapPort;											//Outgoing port audio map after full preprocessing
	yarp::os::Port *gammatonFilteredAudioPort;								//Outgoing port 
	yarp::os::Port *beamFormedAudioPort;									//Outgoing port

	yarp::sig::Sound* s;													//Yarp Sound that stores the incoming sound from inPort
	yarp::os::Stamp ts;														//Contains the time stamps for the perticulare frame of sound
	float *rawAudio;														//Contains 

	yarp::sig::Matrix* outAudioMap;											//Yarp Matrix used to store preprocessed audio
	yarp::sig::Matrix* outGammatonFilteredAudio;							//Yarp Matrix used to store audio after its been gammaton filtered
	yarp::sig::Matrix* outBeamFormedAudio;									//Yarp Matrix used to store the beam formed audio

	std::string fileName;
	int frameSamples;
	int nBands;
	int totalBeams;
	int nMics;

	GammatonFilter *gammatonAudioFilter;
	BeamFormer *beamForm;

	std::vector < std::vector < float* > > beamFormedAudioVector;
	std::vector < std::vector < double > > reducedBeamFormedAudioVector;

	FILE *fid;
	int mappedFileID;
	double *mappedAudioData;

};

#endif
