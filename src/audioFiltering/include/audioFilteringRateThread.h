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


#ifndef _AUDIO_MEMORY_MAPPER_RATETHREAD_H_
#define _AUDIO_MEMORY_MAPPER_RATETHREAD_H_

#include <iostream>
#include <fstream>
#include <time.h>

#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/NetInt32.h>
#include <yarp/sig/Sound.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>

#include "webrtc/modules/audio_processing/include/audio_processing.h"
#include "webrtc/modules/interface/module_common_types.h"
#include "webrtc/common_audio/channel_buffer.h"
#include "webrtc/modules/audio_processing/beamformer/nonlinear_beamformer.h"
#include "webrtc/common_audio/include/audio_util.h"

class audioFilteringRateThread : public yarp::os::RateThread {
public:
    /**
    * constructor default
    */
    audioFilteringRateThread();

    /**
    * constructor 
    * @param robotname name of the robot
    */
    audioFilteringRateThread(std::string robotname,std::string configFile);

    /**
     * destructor
     */
    ~audioFilteringRateThread();

    /**
    *  initialises the thread
    */
    bool threadInit(yarp::os::ResourceFinder &rf);

    /**
    *  correctly releases the thread
    */
    void threadRelease();

    /**
    *  active part of the thread
    */
    void run(); 

    /**
    * function that sets the rootname of all the ports that are going to be created by the thread
    * @param str rootnma
    */
    void setName(std::string str);
    
    /**
    * function that returns the original root name and appends another string iff passed as parameter
    * @param p pointer to the string that has to be added
    * @return rootname 
    */
    std::string getName(const char* p);

    /**
     * method for the processing in the ratethread
     **/
    bool processing();

private:
    bool result;                    //result of the processing

    std::string robot;              // name of the robot
    std::string configFile;         // name of the configFile where the parameter of the camera are set
    std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber

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

    std::string name;                                                                // rootname of all the ports opened by this thread

    yarp::sig::Sound* s;
    yarp::os::Stamp ts;

    yarp::os::BufferedPort<yarp::sig::Sound> *inPort;

};

#endif  //_AUDIO_PREPROCESSER_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------