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
#include <yarp/math/Math.h>
#include <yarp/sig/Sound.h>
//#include <iCub/audio/Sound.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>

//Memory mapping requirements
#include <sys/mman.h>
#include <fcntl.h>
#include <string>


const float normDivid = pow(2,23);  //Number that is used to conver the integer number resived as the audio signal and convert it to a double audio signal

struct knotValues {
   double k0, k1, k2;
};

class AudioPreprocesserModule: public yarp::os::RFModule
{
 private:
  	std::string moduleName;                  // name of the module
  	std::string robotPortName;               // name of robot port
  	std::string inputPortName;               // name of the input port for events
  	std::string robotName;                   // name of the robot
  	std::string configFile;                  // name of the configFile that the resource Finder will seek
  	AudioPreprocesserRatethread* apr;        // ratethread handling the processing in the module

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
	*
	*	Accesses the loadFile.xml that is found in the root directory of this
	*	module and load all required parameters for the beam former.
	*
	* 	@param rf : resource finder object containing the values of presets
	*/
	void loadFile(yarp::os::ResourceFinder &rf);


	/**
	*	sendAudioMap
	*
	*	Function used to send audio map after its been though the gammaton, beamforming, and reduction steps.
	*	The audio map is stored in outAudioMap and sent though port audioMapPort.
	*/
	void sendAudioMap();


	/**
	*	sendGammatoneFilteredAudio
	*
	*	Function used to send audio after its pass though the gammaton filter.
	*	The audio is held in outGammatoneFilteredAudio matrix and is sent though port GammatoneFilteredAudioPort.
	*
	*	@param gammatoneAudio : filtered gammatone audio
	*/
	void sendGammatoneFilteredAudio(const std::vector<float*> &gammatoneAudio);


	/**
	* 	sendBeamFormedAudio
	*
	*	Function used to send beam formed audio that is held in outBeamFormedAudio though port beamFormedAudioPort.
	*/
	void sendBeamFormedAudio();


	/**
	* 	linerApproximation
	*
	*	Helper function used by linerInterp to do liner interpolation between points (x1,y1) and (x2,y2).
	* 	
	*	@param  x : the position on the curve being looked for
	*	@param x1 : x coordinate for point 1
	*	@param x2 : x coordinate for point 2
	*	@param y1 : y coordinate for point 1
	*	@param y2 : y coordinate for point 2
	*	@return the y value of the asked x
	*/
	inline double linerApproximation(int x, int x1, int x2, double y1, double y2);


	/**
	* 	linerInterpolate
	*
	*	Taking the Audio data that is found in reducedBeamFormedAudioVector.
	*	Creates an interpolation of the data corresponding to the interpolateNSamples that was specified in the xml.
	*	The data of this function will be saved in highResolutionAudioMap.
	*/
	void linerInterpolate();


	/**
	* 	splineApproximation
	*
 	* 	Given a particular x on a curve, return its y value.
 	*
 	* 	@param  x : the position on the curve being looked for
 	* 	@param x1 : x coordinate for point 1 
 	* 	@param y1 : y coordinate for point 1
 	* 	@param x2 : x coordinate for point 2
 	* 	@param y2 : y coordinate for point 2
 	* 	@param x3 : x coordinate for point 3
 	* 	@param y3 : y coordinate for point 3
 	* 	@return the y value of the asked x
 	*/
	double splineApproximation(double x, double x1, double y1, double x2, double y2, double x3, double y3);


	/**
	* 	calcSplineKnots
	*
 	* 	Given three coordinates, find the value of the knots for each point.
 	* 
 	* 	@param x1 : x coordinate for point 1 
 	* 	@param y1 : y coordinate for point 1
 	* 	@param x2 : x coordinate for point 2
 	* 	@param y2 : y coordinate for point 2
 	* 	@param x3 : x coordinate for point 3
 	* 	@param y3 : y coordinate for point 3
 	*	@return the calculated value of the curves knots
 	*/  
	knotValues calcSplineKnots(double x1, double y1, double x2, double y2, double x3, double y3);


	/**
	* 	splineInterpolate
	*
	*	Taking the Audio data that is found in reducedBeamFormedAudioVector.
	*	Creates an interpolation of the data corresponding to the interpolateNSamples that was specified in the xml.
	*	The data of this function will be saved in highResolutionAudioMap.
	*/
	void splineInterpolate();

	double startTime, stopTime;

	int interpolateNSamples;

	std::vector < std::vector < double > > highResolutionAudioMap;


	//Incoming Audio Data from the iCub and remoteInterface
	yarp::os::BufferedPort<yarp::sig::Sound> *inPort;
	//yarp::os::BufferedPort<audio::Sound> *inPort;
	
	yarp::os::Port *outGammaToneAudioPort;
    yarp::os::Port *outReducedBeamFormedAudioPort;
    yarp::os::Port *outBeamFormedAudioPort;
    yarp::os::Port *outAudioMapEgoPort;

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

	int lastframe;
	std::vector < std::vector < float* > > beamFormedAudioVector;
	std::vector < std::vector < double > > reducedBeamFormedAudioVector;

	int samplingRate;
	int lowCf;
	int highCf;

	int longBufferSize;
	int nBeamsPerHemi;

	double micDistance;
	int C;
};

#endif
