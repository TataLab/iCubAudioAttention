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
 * @file  audioPreprocesserRatethread.h
 * @brief Header file of the processing ratethread.
 *        This is where the processing happens.
 */

#ifndef _AUDIO_PREPROCESSER_RATETHREAD_H_
#define _AUDIO_PREPROCESSER_RATETHREAD_H_

#include <iostream>
//#include <cmath>

#include <yarp/dev/all.h>

#include <yarp/math/Math.h>

#include <yarp/os/all.h>
#include <yarp/os/Log.h>
#include <yarp/os/RateThread.h>

#include <yarp/sig/all.h>

#include "gammatoneFilter.h"
#include "beamFormer.h"

const float  normDivid = pow(2,15);
const double _pi       = 2 * acos(0.0); 

//const float normDivid = pow(2,23);  // Number that is used to convert the integer number received
                                    // as the audio signal and convert it to a double audio signal

struct knotValues {
    double k0, k1, k2;
};

class AudioPreprocesserRatethread : public yarp::os::RateThread {

 private:
	//
	// name strings
	//
	std::string configFile;         // name of the configFile where the parameter of the camera are set
	std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber
	std::string name;               // rootname of all the ports opened by this thread
	std::string robot;              // name of the robot


	//
	// Incoming Audio Data from the iCub and remoteInterface
	//
	yarp::os::BufferedPort<yarp::sig::Sound> *inPort;
	yarp::os::Port *outGammaToneAudioPort;
    yarp::os::Port *outGammaTonePowerAudioPort;
	yarp::os::Port *outReducedBeamFormedAudioPort;
	yarp::os::Port *outBeamFormedAudioPort;
    //yarp::os::BufferedPort<yarp::sig::Matrix> *outBeamFormedPowerAudioPort;
    yarp::os::Port *outBeamFormedPowerAudioPort;
	yarp::os::Port *outAudioMapEgoPort;

	yarp::os::Stamp ts;
	yarp::sig::Sound* s;


	//
	// containers for processed data
	//
	yarp::sig::Matrix* outAudioMap;
	yarp::sig::Matrix* outGammaToneFilteredAudioMap;
    yarp::sig::Matrix* outGammaTonePowerAudioMap;
	yarp::sig::Matrix* outBeamFormedAudioMap;
	yarp::sig::Matrix* outReducedBeamFormedAudioMap;
    yarp::sig::Matrix* outBeamFormedPowerAudioMap;

	std::vector < std::vector < std::vector < float > > > beamFormedAudioVector;
	std::vector < std::vector < double > > highResolutionAudioMap;
	std::vector < std::vector < double > > reducedBeamFormedAudioVector;

	float *rawAudio;



    //----------------------------------
    
    yarp::sig::Vector spaceAngles;
    yarp::sig::Vector micAngles;


	//
	// processing objects
	//
	GammatoneFilter *gammatoneAudioFilter;
	BeamFormer      *beamForm;


    int lastframe;
    int longBufferSize;
    

    double startTime;
    double stopTime;


    //--
    //-- Variables Set on Init.
    //--
    double C;
    int    nMics;
    double micDistance;
    int    frameSamples;
    int    samplingRate;
    int    nBands;
    int    lowCf;
    int    highCf;
    int    interpolateNSamples;
    int    radialRes_degrees;

    int    nBeamsPerHemi;
    int    nBeams;
    int    nMicAngles;
    double radialRes_radians;
    int    nSpaceAngles;
    
    
    
    

    


 public:
    /**
     *  constructor
     */
    AudioPreprocesserRatethread();


    /**
     *  constructor
     *
     *  set robotname and configFile, the calls loadFile
     *  on the passed in resource finder
     *
     *  @param   _robotname : name of the robot
     *  @param  _configFile : configuration file
     *  @param           rf : resource finder object for setting constants
     */
    AudioPreprocesserRatethread(std::string _robotname, std::string _configFile, yarp::os::ResourceFinder &rf);


    /**
     *  destructor
     */
    ~AudioPreprocesserRatethread();


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
     *  sendAudioMap
     *
     *  Function used to send audio map after its been though the gammaton, beamforming, and reduction steps.
     *  The audio map is stored in outAudioMap and sent though port audioMapPort.
     */
     void sendAudioMap();


    /**
     *  sendGammatoneFilteredAudio
     *
     *  Function used to send audio after its pass though the gammatone filter.
     *  The audio is held in outGammatToneFilteredAudio matrix and is sent though port GammatoneFilteredAudioPort.
     *
     *  @param gammatoneAudio : filtered gammatone audio
     */
    void sendGammatoneFilteredAudio(const std::vector<float*> &gammatoneAudio);


    /**
     *  sendGammatonePowerAudio
     * 
     *  Function used to send the power of gammatone filtered audio at each band.
     *  The audio is held in outGammaTonePowerAudio matrix and is sent
     *  through port /iCubAudioAttention/GammaTonePowerAudio:o.
     * 
     *  @param gammatonePower : power of gammatone audio across framesamples per beam.
     */
    void sendGammatonePowerAudio(const std::vector<float> &gammatonePower);


    /**
     *  sendBeamFormedAudio
     *
     *  Function used to send beam formed audio that is held in outBeamFormedAudio though port beamFormedAudioPort.
     */
    void sendBeamFormedAudio(const std::vector<std::vector<std::vector<float> > > &beamFormedAudio);


    /**
     *  sendReducedBeamFormedAudio
     *
     *  Function used to send beam formed audio that is held in outBeamFormedAudio though port beamFormedAudioPort.
     */
    void sendReducedBeamFormedAudio(const std::vector<std::vector<double> > &reducedBeamFormedAudio);


    /**
     *  sendBeamFormedPowerAudio
     * 
     *  Function used to send the power of beam formed audio at each beam.
     *  The audio is held in outBeamFormedPowerAudio matrix and is sent
     *  through port /iCubAudioAttention/BeamFormedPowerAudio:o.
     * 
     *  @param beamFormedPower : power of reduced beamformed audio across each band.
     */
    void sendBeamFormedPowerAudio(const std::vector< double > &beamFormedPower);


    /**
     *  linerApproximation
     *
     *  Helper function used by linerInterp to do liner interpolation between points (x1,y1) and (x2,y2).
     *
     *  @param  x : the position on the curve being looked for
     *  @param x1 : x coordinate for point 1
     *  @param y1 : y coordinate for point 1
     *  @param x2 : x coordinate for point 2
     *  @param y2 : y coordinate for point 2
     *
     *  @return the corresponding y value of the asked x
     */
    inline double linerApproximation(int x, int x1, double y1, int x2, double y2);


    /**
     *  linerInterpolate
     *
     *  Taking the Audio data that is found in reducedBeamFormedAudioVector.
     *  Creates an interpolation of the data corresponding to the interpolateNSamples that was specified in the xml.
     *  The data of this function will be saved in highResolutionAudioMap.
     */
    void linerInterpolate();


    /**
     *  splineApproximation
     *
     *  Given a particular x on a curve, return its y value.
     *
     *  @param  x : the position on the curve being looked for
     *  @param x1 : x coordinate for point 1
     *  @param y1 : y coordinate for point 1
     *  @param x2 : x coordinate for point 2
     *  @param y2 : y coordinate for point 2
     *  @param x3 : x coordinate for point 3
     *  @param y3 : y coordinate for point 3
     *
     *  @return the corresponding y value of the asked x
     */
    double splineApproximation(double x, double x1, double y1, double x2, double y2, double x3, double y3);


    /**
     *  calcSplineKnots
     *
     *  Given three coordinates, find the value of the knots for each point.
     *
     *  @param x1 : x coordinate for point 1
     *  @param y1 : y coordinate for point 1
     *  @param x2 : x coordinate for point 2
     *  @param y2 : y coordinate for point 2
     *  @param x3 : x coordinate for point 3
     *  @param y3 : y coordinate for point 3
     *
     *  @return the calculated value of the curves knots
     */
    knotValues calcSplineKnots(double x1, double y1, double x2, double y2, double x3, double y3);


    /**
     *  splineInterpolate
     *
     *  Taking the Audio data that is found in reducedBeamFormedAudioVector.
     *  Creates an interpolation of the data corresponding to the interpolateNSamples that was specified in the xml.
     *  The data of this function will be saved in highResolutionAudioMap.
     */
    void splineInterpolate();
};

#endif  //_AUDIO_PREPROCESSER_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
