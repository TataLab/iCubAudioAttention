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
#include <audioPreprocesserRatethread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 80 //ms

AudioPreprocesserRatethread::AudioPreprocesserRatethread():RateThread(THRATE) {
    robot = "icub";        
}

AudioPreprocesserRatethread::AudioPreprocesserRatethread(string _robot, string _configFile):RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
}

AudioPreprocesserRatethread::~AudioPreprocesserRatethread() {
    // do nothing
}

bool AudioPreprocesserRatethread::threadInit() {
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

void AudioPreprocesserRatethread::setName(string str) {
    this->name=str;
}


std::string AudioPreprocesserRatethread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void AudioPreprocesserRatethread::setInputPortName(string InpPort) {
    
}

void AudioPreprocesserRatethread::run() {    
    //code here .....
    

}

bool AudioPreprocesserRatethread::processing(){
    // here goes the processing...
    return true;
}


void AudioPreprocesserRatethread::threadRelease() {
    // nothing
    inputPort.interrupt();
    outputPort.interrupt();
    inputPort.close();
    outputPort.close();
}


