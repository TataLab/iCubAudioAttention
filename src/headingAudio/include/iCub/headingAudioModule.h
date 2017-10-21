// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Francesco Rea
  * email: francesco.rea@iit.it
  * Permission is granted to copy, distribute, and/or modify this program
  * under the terms of the GNU General Public License, version 2 or any
  * later version published by the Free Software Foundation.
  *
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details
*/


/**
 * @file headingAudioModule.h
 * @brief Simple module as tutorial.
 */

#ifndef _HEADING_AUDIO_MODULE_H_
#define _HEADING_AUDIO_MODULE_H_

/** 
 *
 * \defgroup icub_tutorialRateThread tutorialRateThread
 * @ingroup icub_morphoGen
 *
 * This is a module that receives the RGB image from input connection and sends it back to output connection. The purpose
 * of the module is to shift the point of congestion in a network.
 * 
 *
 * 
 * \section lib_sec Libraries
 *
 * YARP.
 *
 * \section parameters_sec Parameters
 * 
 * <b>Command-line Parameters</b> 
 * 
 * The following key-value pairs can be specified as command-line parameters by prefixing \c -- to the key 
 * (e.g. \c --from file.ini. The value part can be changed to suit your needs; the default values are shown below. 
 *
 * - \c from \c tutorialRateThread.ini \n 
 *   specifies the configuration file
 *
 * - \c context \c tutorialRateThread/conf \n
 *   specifies the sub-path from \c $ICUB_ROOT/icub/app to the configuration file
 *
 * - \c name \c tutorialRateThread \n 
 *   specifies the name of the tutorialRateThread (used to form the stem of tutorialRateThread port names)  
 *
 * - \c robot \c icub \n 
 *   specifies the name of the robot (used to form the root of robot port names)
 *
 *
 * <b>Configuration File Parameters</b>
 *
 * The following key-value pairs can be specified as parameters in the configuration file 
 * (they can also be specified as command-line parameters if you so wish). 
 * The value part can be changed to suit your needs; the default values are shown below. 
 *   
 *
 * 
 * \section portsa_sec Ports Accessed
 * 
 * - None
 *                      
 * \section portsc_sec Ports Created
 *
 *  <b>Input ports</b>
 *
 *  - \c /tutorialRateThread \n
 *    This port is used to change the parameters of the tutorialRateThread at run time or stop the tutorialRateThread. \n
 *    The following commands are available
 * 
 *  -  \c help \n
 *  -  \c quit \n
 *
 *    Note that the name of this port mirrors whatever is provided by the \c --name parameter value
 *    The port is attached to the terminal so that you can type in commands and receive replies.
 *    The port can be used by other tutorialRateThreads but also interactively by a user through the yarp rpc directive, viz.: \c yarp \c rpc \c /tutorialRateThread
 *    This opens a connection from a terminal to the port and allows the user to then type in commands and receive replies.
 *       
 *  - \c /tutorialRateThread/image:i \n
 *
 * <b>Output ports</b>
 *
 *  - \c /tutorialRateThread \n
 *    see above
 *
 *  - \c /tutorialRateThread/image:o \n
 *
 * <b>Port types</b>
 *
 * The functional specification only names the ports to be used to communicate with the tutorialRateThread 
 * but doesn't say anything about the data transmitted on the ports. This is defined by the following code. 
 *
 * \c BufferedPort<ImageOf<PixelRgb> >   \c myInputPort; \n 
 * \c BufferedPort<ImageOf<PixelRgb> >   \c myOutputPort;       
 *
 * \section in_files_sec Input Data Files
 *
 * None
 *
 * \section out_data_sec Output Data Files
 *
 * None
 *
 * \section conf_file_sec Configuration Files
 *
 * \c tutorialRateThread.ini  in \c $ICUB_ROOT/app/tutorialRateThread/conf \n
 * \c icubEyes.ini  in \c $ICUB_ROOT/app/tutorialRateThread/conf
 * 
 * \section tested_os_sec Tested OS
 *
 * Linux
 *
 * \section example_sec Example Instantiation of the Module
 * 
 * <tt>tutorialRateThread --name tutorialRateThread --context tutorialRateThread/conf --from tutorialRateThread.ini --robot icub</tt>
 *
 * \author Rea Francesco
 *
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.\n
 * This file can be edited at \c $ATTENTION_ROOT/src/tutorial/tutorialRateThread/include/iCub/tutorialRateThread.h
 * 
 */


#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 

#include <yarp/os/all.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/GazeControl.h>

#include <yarp/sig/Vector.h>
#include <yarp/sig/Matrix.h>

//#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Thread.h>
#include <yarp/os/Log.h>
#include <yarp/os/Time.h>
//#include <iCub/attention/commandDictionary.h>

//within project includes  
//#include <iCub/headingAudioRatethread.h> // TODO: introduce the ratethread for more efficient processing

#define COMMAND_VOCAB_ON    VOCAB2('o','n')
#define COMMAND_VOCAB_OK    VOCAB2('o','k')


#define COMMAND_VOCAB_OFF   VOCAB3('o','f','f')
#define COMMAND_VOCAB_SUS   VOCAB3('s','u','s')
#define COMMAND_VOCAB_RES   VOCAB3('r','e','s')
#define COMMAND_VOCAB_FIX   VOCAB3('f','i','x')


#define COMMAND_VOCAB_DUMP  VOCAB4('d','u','m','p')
#define COMMAND_VOCAB_SYNC  VOCAB4('s','y','n','c')
#define COMMAND_VOCAB_HELP  VOCAB4('h','e','l','p')
#define COMMAND_VOCAB_STOP  VOCAB4('s','t','o','p')
#define COMMAND_VOCAB_SEEK  VOCAB4('s','e','e','k')
#define COMMAND_VOCAB_CENT  VOCAB4('c','e','n','t')
#define COMMAND_VOCAB_FAIL  VOCAB4('f','a','i','l')


#define DELTAENC 0.0000001
#define deg2rad  3.1415/180

class headingAudioModule:public yarp::os::RFModule {
    bool idle;

    double r;
    double value;
    double endPos2; 
    double startPos2;
    double startPos3;
    double startPos4;

    yarp::os::Semaphore mutex;

    yarp::dev::IGazeControl* igaze;
    yarp::dev::IPositionControl *pos;
    yarp::dev::IEncoders *encs;
    yarp::sig::Vector encoders;
    yarp::sig::Vector command;
    yarp::sig::Vector tmp;
    yarp::sig::Vector angles;
    yarp::sig::Vector position;
    
    std::string moduleName;                  // name of the module
    std::string robotName;                   // name of the robot 
    std::string robotPortName;               // name of robot port
    std::string inputPortName;               // name of the input port for events
    std::string outputPortName;              // name of output port
    std::string handlerPortName;             // name of handler port
    std::string configFile;                  // name of the configFile that the resource Finder will seek
    
    yarp::os::BufferedPort<yarp::os::Bottle>* _pInPort; //= new BufferedPort<Bottle>;
    yarp::os::BufferedPort<yarp::sig::Matrix>* _pAbsolutePort;
    yarp::os::Port* _pOutPort; //= new Port;
    yarp::os::Port handlerPort;              // a port to handle messages 
    /* // TODO: introduce the ratethread for more efficient processing */
    //headingAudioRatethread *rThread;             // pointer to a new thread to be created and started in configure() and stopped in close()

public:
    /**
    *  configure all the tutorial parameters and return true if successful
    * @param rf reference to the resource finder
    * @return flag for the success
    */
    bool configure(yarp::os::ResourceFinder &rf); 
   
    /**
    *  interrupt, e.g., the ports 
    */
    bool interruptModule();                    

    /**
    *  close and shut down the tutorial
    */
    bool close();

    /**
    *  to respond through rpc port
    * @param command reference to bottle given to rpc port of module, alongwith parameters
    * @param reply reference to bottle returned by the rpc port in response to command
    * @return bool flag for the success of response else termination of module
    */
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);

    /**
    *  unimplemented
    */
    double getPeriod();

    /**
    *  unimplemented
    */ 
    bool updateModule();
};


#endif // _HEADING_AUDIO_MODULE_H__

//----- end-of-file --- ( next line intentionally left blank ) ------------------

