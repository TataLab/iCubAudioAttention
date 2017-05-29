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




#ifndef _AUDIO_PREPROCESSER_MODULE_H_
#define _AUDIO_PREPROCESSER_MODULE_H_

#include "audioPreprocesserRatethread.h"
#include "gammatoneFilter.h"
#include "beamFormer.h"

#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/NetInt32.h>
#include <yarp/sig/Sound.h>
//#include <iCub/audio/Sound.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>

//Memory mapping requirements
#include <sys/mman.h>
#include <fcntl.h>
#include <string>


const float normDivid = pow(2,23);  //Number that is used to conver the integer number resived as the audio signal and convert it to a double audio signal

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
	/**
	*	loadFile
	*	Accesses the loadFile.xml that is found in the root directory of this
	*	module and load all required parameters for the beam former.
	*/
	void loadFile(yarp::os::ResourceFinder &rf);


	/**
	*	createMemoryMappedFile
	*	Creates and allocates all the data required for the memory mapping.
	*/
	void createMemoryMappedFile();

	/**
	*	memoryMapper
	*	Taking the Audio data
	*/
	void memoryMapper();

	/**
	*	memoryMapperRawAudio
	*	Taking the Audio data
	*/
	void memoryMapperRawAudio();

	/**
	*	memoryMapperRawAudio
	*	Taking the Audio data
	*/
	void memoryMapperGammaToneFilteredAudio(const std::vector<float*>  gammatoneAudio);

	/**
	*	sendAudioMap
	*	Function used to send audio map after its been though the gammaton, beamforming, and reduction steps.
	*	The audio map is stored in outAudioMap and sent though port audioMapPort.
	*/
	void sendAudioMap();

	/**
	*	sendGammatoneFilteredAudio
	*	Function used to send audio after its pass though the gammaton filter.
	*	The audio is held in outGammatoneFilteredAudio matrix and is sent though port GammatoneFilteredAudioPort.
	*/
	void sendGammatoneFilteredAudio(const std::vector<float*> &gammatoneAudio);

	/**
	* 	sendBeamFormedAudio
	*	Function used to send beam formed audio that is held in outBeamFormedAudio though port beamFormedAudioPort.
	*/
	void sendBeamFormedAudio();

	/**
	* 	interpl
	*	Helper function used by spineInterp to do liner interpolation between points (x1,y1) and (x2,y2).
	*/
	double interpl(int x, int x1, int x2, double y1, double y2);

	/**
	* SpineInterp()
	*
	*	Taking the Audio data that is found in reducedBeamFormedAudioVector.
	*	Creates an interpolation of the data corresponding to the interpellateNSamples that was specified in the xml.
	*	The data of this function will be saved in highResolutionAudioMap.
	*/
	void spineInterp();

	double startTime, stopTime;

	int interpellateNSamples;

	std::vector < std::vector < double > > highResolutionAudioMap;


	//Incoming Audio Data from the iCub and remoteInterface
	yarp::os::BufferedPort<yarp::sig::Sound> *inPort;
	//yarp::os::BufferedPort<audio::Sound> *inPort;
	
	yarp::os::Port *outPort;
	yarp::os::Port *outGammaToneFilteredAudioPort;
	yarp::sig::Sound* s;
	//audio::Sound* s;
	yarp::os::Stamp ts;
	float *rawAudio;

	yarp::sig::Matrix* outAudioMap;
	yarp::sig::Matrix* outGammaToneFilteredAudioMap;

	std::string fileName;
	int frameSamples;
	int nBands;
	int totalBeams;
	int nMics;

	GammatoneFilter *gammatoneAudioFilter;
	BeamFormer *beamForm;

	double oldtime;
	int lastframe;
	std::vector < std::vector < float* > > beamFormedAudioVector;
	std::vector < std::vector < double > > reducedBeamFormedAudioVector;

	FILE *fid;
	int mappedFileID;
	double *mappedAudioData;

	FILE *rawFid;
	int mappedRawAduioFileID;
	double *mappedRawAduioData;

	FILE *gammaToneFilteredFid;
	int mappedGammaToneFilteredAduioFileID;
	double *mappedGammaToneFilteredAduioData;


	int samplingRate;
	int lowCf;
	int highCf;

	int longBufferSize;
	int nBeamsPerHemi;

	int micDistance;
	int C;

};

#endif
