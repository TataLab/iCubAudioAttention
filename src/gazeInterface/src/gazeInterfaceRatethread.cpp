// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2016  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
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
 * @file gazeInterfaceRatethread.cpp
 * @brief Implementation of the eventDriven thread (see gazeInterfaceRatethread.h).
 */

#include <iCub/gazeInterfaceRatethread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 10 //ms

gazeInterfaceRatethread::gazeInterfaceRatethread():RateThread(THRATE) {
    robot = "icub";        
}

gazeInterfaceRatethread::gazeInterfaceRatethread(string _robot, string _configFile):RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
}

gazeInterfaceRatethread::~gazeInterfaceRatethread() {
    // do nothing
    delete robotHead;
}

bool gazeInterfaceRatethread::threadInit() {
    // opening the port for direct input
    if (!inputPort.open(getName("/readings:i").c_str())) {
        yError("unable to open port to receive input");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!outputPort.open(getName("/command:o").c_str())) {
        yError(": unable to open port to send unmasked events ");
        return false;  // unable to open; let RFModule know so that it won't run
    }
    
    // set up polydriver with head
    options.put("device", "remote_controlboard");
    options.put("local", getName("local_controlboard"));
    options.put("remote", "/icub/head"); //TODO: make this remote name generic by using robot variable.

    robotHead = new PolyDriver(options);
      
    if (!robotHead->isValid()) {
        yInfo("Cannot connect to robot head\n");
        return 1;
    }

    robotHead->view(pos);
    robotHead->view(vel);
    robotHead->view(enc);


    if (pos==NULL || vel==NULL || enc==NULL) {
        yInfo("Cannot get interface to robot head\n");
        robotHead->close();
        return false;
    }

    // get total available axes
    int jnts = 0;
    pos->getAxes(&jnts);

    // resize these to axes
    setpoints.resize(jnts);
    checkpoints.resize(jnts);

    //currentpos = 0;
    //counter = 100;

    // init speed
    speeds[0] = 60;
    speeds[1] = 80;
    pos->setRefSpeeds(speeds);

    flagger[2];

    // init points
    setpoints[1] = 0;
    setpoints[0] = 0;
    
    // init position
    pos->positionMove(setpoints.data());

    yInfo("Initialization of the processing thread correctly ended");
    return true;
}

void gazeInterfaceRatethread::setName(string str) {
    this->name=str;
}


std::string gazeInterfaceRatethread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void gazeInterfaceRatethread::setInputPortName(string InpPort) {
    
}

void gazeInterfaceRatethread::run() {    
    if (inputPort.getInputCount()) {
        inputReading = inputPort.read(true);   //blocking reading for synchr with the input
        //yInfo("reading from SIM: %s", inputReading->toString().c_str());

        
        
        // changing the pointer of the prepared area for the outputPort.write()
        //double roll  = inputReading->get(1).asDouble();
        //double pitch = inputReading->get(0).asDouble();
        double yaw   = inputReading->get(2).asDouble();           

        result = processing();
            
        setpoints[0] = yaw;

        pos->positionMove(setpoints.data());  

        if (outputPort.getOutputCount()) {

            Bottle& outputCommand = outputPort.prepare();
            outputCommand.clear();                     

            //outputCommand.addDouble(roll);
            //outputCommand.addDouble(pitch);
            outputCommand.addDouble(yaw);
            outputPort.write();
        }
    }
}

bool gazeInterfaceRatethread::processing(){
    // here goes the processing...
    return true;
}


void gazeInterfaceRatethread::threadRelease() {
    // nothing
    inputPort.interrupt();
    outputPort.interrupt();
    inputPort.close();
    outputPort.close();
}


