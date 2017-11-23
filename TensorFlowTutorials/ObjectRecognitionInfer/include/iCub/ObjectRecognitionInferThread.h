// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author: jonas gonzalez
  * email: 
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
 * @file ObjectRecognitionInferThread.h
 * @brief Definition of a thread that receives an data from input port and sends it to the output port.
 */


#ifndef _ObjectRecognitionInferThread_RATETHREAD_H_
#define _ObjectRecognitionInferThread_RATETHREAD_H_


#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Log.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>

#include "../include/iCub/TensorFlowInference.h"


class ObjectRecognitionInferThread : public yarp::os::RateThread {
private:
    bool result;                    //result of the processing

    std::string robot;              // name of the robot
    std::string configFile;         // name of the configFile where the parameter of the camera are set
    std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber
    std::string name;               // rootname of all the ports opened by this thread
    std::string graph_path;         // path to the graph to be loaded
    std::string labels_path;        // path to the associated graph labels


    std::unique_ptr<TensorFlowInference> tensorFlowInference;


    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > inputImagePort;
    yarp::os::BufferedPort<yarp::os::Bottle> outputLabelPort;

public:
    /**
    * constructor default
    */
    ObjectRecognitionInferThread(yarp::os::ResourceFinder &rf);

    /**
    * constructor 
    * @param robotname name of the robot
    */
    ObjectRecognitionInferThread(yarp::os::ResourceFinder &rf, std::string robotname, std::string configFile);

    /**
     * destructor
     */
    ~ObjectRecognitionInferThread();

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
    std::string getName(const char *p);

    /**
    * function that sets the inputPort name
    */
    void setInputPortName(std::string inpPrtName);

    /**
     * Function to write into the Bottle outputLabelPort
     * @param label
     */
    void writeToLabelPort(std::string label);

    std::string predictTopClass();


};

#endif  //_ObjectRecognitionInferThread_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

