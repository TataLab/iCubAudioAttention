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
 * @file audioAttentionManagerRatethread.cpp
 * @brief Implementation of the eventDriven thread (see audioAttentionManagerRatethread.h).
 */

#include <iCub/audioAttentionManagerRatethread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 100 //ms

audioAttentionManagerRatethread::audioAttentionManagerRatethread():RateThread(THRATE) {
    robot = "icub";        
}

audioAttentionManagerRatethread::audioAttentionManagerRatethread(string _robot, string _configFile) : RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
}

audioAttentionManagerRatethread::~audioAttentionManagerRatethread() {
    // do nothing
}

bool audioAttentionManagerRatethread::threadInit() {
    // opening the port for direct input
    if (!inputPort.open(getName("/image:i").c_str())) {
        yError("unable to open port to receive input");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!outputPort.open(getName("/img:o").c_str())) {
        yError(": unable to open port to send unmasked events ");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    yInfo("Initialization of the processing thread correctly ended");

    return true;
}

void audioAttentionManagerRatethread::setName(string str) {
    this->name=str;
}


std::string audioAttentionManagerRatethread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void audioAttentionManagerRatethread::setInputPortName(string InpPort) {
    
}

void audioAttentionManagerRatethread::run() {    
    //code here .....
    if (inputPort.getInputCount()) {
        inputImage = inputPort.read(true);   //blocking reading for synchr with the input
        result = processing();
    }

    if (outputPort.getOutputCount()) {
        *outputImage = outputPort.prepare();
        outputImage->resize(inputImage->width(), inputImage->height());
        // changing the pointer of the prepared area for the outputPort.write()
        // copy(inputImage, outImage);
        // outputPort.prepare() = *inputImage; //deprecated

        outputPort.write();
    }

}

bool audioAttentionManagerRatethread::processing(){
    // here goes the processing...
    return true;
}


void audioAttentionManagerRatethread::threadRelease() {
    // nothing
    inputPort.interrupt();
    outputPort.interrupt();
    inputPort.close();
    outputPort.close();
}


