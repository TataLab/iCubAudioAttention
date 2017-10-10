// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2016  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Francesco Rea
  * email: francesco.rea@iit.it
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
 * @file tutorialThread.h
 * @brief Definition of a thread that receives an RGN image from input port and sends it to the output port.
 */


#ifndef _FREQ_VISUALISATION_RATETHREAD_H_
#define _FREQ_VISUALISATION_RATETHREAD_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Log.h>
#include <iostream>
#include <fstream>
#include <time.h>


class freqVisualisationRatethread : public yarp::os::RateThread {
private:
    bool result;                    //result of the processing

    int imageOutHeight;
    int imageOutWidth;
    int visGain;

    std::string robot;              // name of the robot
    std::string configFile;         // name of the configFile where the parameter of the camera are set
    std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber

    yarp::sig::ImageOf<yarp::sig::PixelRgb>* tmpImage;                              // tmpImage generated by the processing
    yarp::sig::ImageOf<yarp::sig::PixelRgb>* inputImage;                            // input image from the inputPort
    yarp::sig::ImageOf<yarp::sig::PixelRgb>* outputImage;                           // output image

    yarp::os::BufferedPort<yarp::sig::Matrix>    inputPort;      // receives a bottle from the bayesianMap module
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > outputPort;     // output port to plot event
    std::string name;                                                                // rootname of all the ports opened by this thread
    
public:
    /**
    * constructor default
    */
    freqVisualisationRatethread();

    /**
    * constructor 
    * @param robotname name of the robot
    */
    freqVisualisationRatethread(std::string robotname,std::string configFile);

    /**
     * destructor
     */
    ~freqVisualisationRatethread();

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
    * function that sets the gain for the image
    */
    void setGain(int inGain);

     /**
     * method for the processing in the ratethread
     * @param mat matrix to be processed in the method
     **/
    bool processing(yarp::sig::Matrix *mat);

    /**
     * method for the processing in the ratethread
     **/
    bool processing(yarp::os::Bottle *b);
};

#endif  //_TUTORIAL_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------

