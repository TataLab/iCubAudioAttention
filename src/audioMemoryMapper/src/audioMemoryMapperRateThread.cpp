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
#include <audioMemoryMapperRateThread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 80 //ms

audioMemoryMapperRateThread::audioMemoryMapperRateThread():RateThread(THRATE) {
    robot = "icub";      

}

audioMemoryMapperRateThread::audioMemoryMapperRateThread(string _robot, string _configFile):RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
}

audioMemoryMapperRateThread::~audioMemoryMapperRateThread() {
    // do nothing
}

bool audioMemoryMapperRateThread::threadInit() {
    // opening the port for direct input

}

void audioMemoryMapperRateThread::setName(string str) {
    this->name=str;
}


std::string audioMemoryMapperRateThread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void audioMemoryMapperRateThread::setInputPortName(string InpPort) {
    
}

void audioMemoryMapperRateThread::run() {    
    //code here .....
    

}

bool audioMemoryMapperRateThread::processing(){
    // here goes the processing...
    return true;
}


void audioMemoryMapperRateThread::threadRelease() {
    // nothing

}


