// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Austin Kothig, Francesco Rea
  * email: kothiga@uleth.ca, matthew.tata@uleth.ca, francesco.rea@iit.it
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
 * @file audioAttentionManagerRatethread.h
 * @brief Definition of a thread that receives an RGN image from input port and sends it to the output port.
 */

#ifndef _AUDIO_ATTENTION_MANAGER_RATETHREAD_H_
#define _AUDIO_ATTENTION_MANAGER_RATETHREAD_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Log.h>
#include <iostream>
#include <fstream>
#include <time.h>

// 
// delay for time awake
//
#define DELAY 10.0

//
// vocab terms
//
#define COMMAND_VOCAB_SUSPEND            VOCAB3('s','u','s')
#define COMMAND_VOCAB_RESUME             VOCAB3('r','e','s')

class audioAttentionManagerRatethread : public yarp::os::RateThread {
 
 private:
    bool result;                    //result of the processing

    std::string robot;              // name of the robot
    std::string configFile;         // name of the configFile where the parameter of the camera are set
    std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber

    double startTime;
    double stopTime;
    double ciaoTime;

    bool ciaoAwake;

    yarp::os::BufferedPort<yarp::os::Bottle> inputPort;		  // input from rpc
    yarp::os::BufferedPort<yarp::os::Bottle> inputSpeechPort; // input from speech recon
    yarp::os::RpcClient outputPort;						  	  // publish to headinAudio
    std::string name;                                      	  // rootname of all the ports opened by this thread

    yarp::os::Bottle* inputCommand;
    yarp::os::Bottle* inputSpeech;
    yarp::os::Bottle* output;
    yarp::os::Bottle  current;

    std::string command;
    std::string speech;

    
 public:
    /**
    * constructor default
    */
    audioAttentionManagerRatethread();

    /**
    * constructor 
    * @param robotname name of the robot
    */
    audioAttentionManagerRatethread(std::string robotname,std::string configFile);

    /**
     * destructor
     */
    ~audioAttentionManagerRatethread();

    /**
    *  initialises the thread
    */
    bool threadInit();

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
    * function that sets the inputPort name
    */
    void setInputPortName(std::string inpPrtName);

    /**
     * method for the processing in the ratethread
     **/
    bool processing();

    /**
     * method for the managing various states to be sent
     **/
    void stateTransition();
};

#endif  //_AUDIO_ATTENTION_MANAGER_RATETHREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

