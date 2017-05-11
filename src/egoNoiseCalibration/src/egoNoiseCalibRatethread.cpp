// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
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
 * @file egoNoiseCalibRatethread.cpp
 * @brief Implementation of the eventDriven thread (see egoNoiseCalibRatethread.h).
 */

#include <iCub/egoNoiseCalibRatethread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 100 //ms
#define MAXCOUNTERFRAMES 80

const std::string test = "hello";

egoNoiseCalibRatethread::egoNoiseCalibRatethread():RateThread(THRATE) {
    robot = "icub";
    alfa = 1.0;
    counterFrames = 0;
    counterAngles = 0;
    memset(&sum[0],0,sizeof(double) * SUMDIM);
}

egoNoiseCalibRatethread::egoNoiseCalibRatethread(string _robot, string _configFile):RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
    alfa = 1.0;
    counterFrames = 0;
    counterAngles = 0;
    memset(&sum[0],0,sizeof(double) * SUMDIM);
}

egoNoiseCalibRatethread::~egoNoiseCalibRatethread() {
    free(&sum[0]);
}


bool egoNoiseCalibRatethread::threadInit() {
    // opening the port for direct input
    if (!inputPort.open(getName("/image:i").c_str())) {
        yError("unable to open port to receive input");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!outputPort.open(getName("/img:o").c_str())) {
        yError(": unable to open port to send unmasked events ");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    //--------------------------------------------------------------------
   
    string headPort = "/" + robot + "/head";
    string nameLocal("egoNoiseCalibrator");

    //initialising the head polydriver
    optionsHead.put("device", "remote_controlboard");
    optionsHead.put("local", ("/" + nameLocal + "/localhead").c_str());
    optionsHead.put("remote", headPort.c_str());
    robotHead = new PolyDriver (optionsHead);

    if (!robotHead->isValid()){
        yError("cannot connect to robot head\n");
        return false;
    }
    robotHead->view(encHead);
   
    robotHead->view(posHead);
    jnts = 0;
    posHead->getAxes(&jnts);
    encoders.resize(jnts);
    encHead->getEncoders(encoders.data());
    
    yInfo("head encoders data %f", encoders[2]);
    Vector tmp; tmp.resize(jnts);
    command_position.resize(jnts);
    int i;
    for (i = 0; i < jnts; i++) {
        tmp[i] = 40.0;
    }
    posHead->setRefSpeeds(tmp.data());
    command_position = encoders;
    int value = encoders[2];
    command_position[2] = -40.0;
    bool ok = posHead->positionMove(command_position.data());
    Time::delay(2.0);

    encHead->getEncoders(encoders.data());
    yInfo("head encoders data %s", encoders.toString().c_str());
    yInfo("head encoders data %f", encoders[2]);
    
    //-------------------------------------------------------------------

    pFile = fopen("noiseMap.dat", "wb");
    //if (ret == 0) {
    //    yError("file could not open");
    //}
    
    
    //-------------------------------------------------------------------
    yInfo("Initialization of the processing thread correctly ended");
    

    return true;
}

void egoNoiseCalibRatethread::setName(string str) {
    this->name=str;
}


std::string egoNoiseCalibRatethread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void egoNoiseCalibRatethread::setInputPortName(string InpPort) {
    
}

void egoNoiseCalibRatethread::headMovePan() {
    counterAngles++;
    command_position = encoders;
    if(command_position[2] > 40) {
        alfa = -1.0;
    }
    else if(command_position[2] < -40) {
        alfa = 1.0;
    }
    command_position[2] += alfa;
    bool ok = posHead->positionMove(command_position.data());
    Time::delay(1);
    encHead->getEncoders(encoders.data());
    yInfo("head encoders data %f; alfa = %f", encoders[2], alfa);
}

void egoNoiseCalibRatethread::saveEgoNoise() {
    for(int i = 0; i < SUMDIM; i++) {
        sum[i] = sum[i] / counterFrames;
    }
    yDebug("%f %f %f %f", sum[0], sum[1], sum[2], sum[3]);
    fwrite((void*) sum, sizeof(double), SUMDIM, pFile);
}


void egoNoiseCalibRatethread::run() {    
    // input ports
    if (inputPort.getInputCount()) {
        if(counterAngles < MAXCOUNTERFRAMES) { 
            Bottle* b = inputPort.read(true);   //blocking reading for synchr with the input
            result = processing(b);
            if(counterFrames % 10 == 0) {
                // head pan execution
                headMovePan();
            }
        }
        else {
            saveEgoNoise();
            this->suspend();
        }
    }

    //output port
    if (outputPort.getOutputCount()) {
        *outputImage = outputPort.prepare();
        outputImage->resize(inputImage->width(), inputImage->height());
        // changing the pointer of the prepared area for the outputPort.write()
        // copy(inputImage, outImage);
        // outputPort.prepare() = *inputImage; //deprecated

        outputPort.write();
    }

}

bool egoNoiseCalibRatethread::processing(Bottle* b){
    counterFrames++;
    yDebug("COUNTERFRAMES: %d bottle length %d",counterFrames, b->size());
    Bottle* matrixBottle = b->get(2).asList();
    yDebug("COUNTERFRAMES: %d bottle MATRIX length %d",counterFrames, matrixBottle->size());
    for(int i = 0; i < SUMDIM; i++) {
        double value = matrixBottle->get(i).asDouble();
        sum[i] = sum[i] + value;
    }
    return true;
}


void egoNoiseCalibRatethread::threadRelease() {
    fclose(pFile);
    robotHead->close(); 
    inputPort.interrupt();
    outputPort.interrupt();
    inputPort.close();
    outputPort.close();
}


