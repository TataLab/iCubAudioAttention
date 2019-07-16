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
 * @file soundMonitorRatethread.cpp
 * @brief Implementation of the eventDriven thread (see soundMonitorRatethread.h).
 */

#include <iCub/soundMonitorRatethread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 100 //ms
#define MAXCOUNTERFRAMES 80

const std::string test = "hello";

soundMonitorRatethread::soundMonitorRatethread():RateThread(THRATE) {
    robot = "icub";
    alfa = 1.0;
    counterFrames = 0;
    counterAngles = 0;
    memset(&sum[0],0,sizeof(double) * SUMDIM);
    frequency=new Vector();
    frequency->resize(240);
}

soundMonitorRatethread::soundMonitorRatethread(string _robot, string _configFile):RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
    alfa = 1.0;
    counterFrames = 0;
    counterAngles = 0;
    memset(&sum[0],0,sizeof(double) * SUMDIM);
    frequency=new Vector();
    frequency->resize(240);
}

soundMonitorRatethread::~soundMonitorRatethread() {
    free(&sum[0]);
}


bool soundMonitorRatethread::threadInit() {
    // opening the port for direct input
    if (!inputPort.open(getName("/image:i").c_str())) {
        yError("unable to open port to receive input");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!outputPort.open(getName("/img:o").c_str())) {
        yError(": unable to open port to send unmasked events ");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    //-------------------------------------------------------------------

    pFile = fopen("noiseMap.dat", "wb");
    //if (ret == 0) {
    //    yError("file could not open");
    //}
    
    
    //-------------------------------------------------------------------
    yInfo("Initialization of the processing thread correctly ended");
    

    return true;
}

void soundMonitorRatethread::setName(string str) {
    this->name=str;
}


std::string soundMonitorRatethread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void soundMonitorRatethread::setInputPortName(string InpPort) {
    
}

void soundMonitorRatethread::headMovePan() {
    counterAngles++;    
}

void soundMonitorRatethread::saveEgoNoise() {
    for(int i = 0; i < SUMDIM; i++) {
        sum[i] = sum[i] / counterFrames;
    }
    yDebug("%f %f %f %f", sum[0], sum[1], sum[2], sum[3]);
    fwrite((void*) sum, sizeof(double), SUMDIM, pFile);
}


void soundMonitorRatethread::run() {    
    // input ports
    if (inputPort.getInputCount()) {
        if(counterAngles < MAXCOUNTERFRAMES) { 
            Bottle* b = inputPort.read(true);   //blocking reading for synchr with the input
            result = processing(b);
            
        }
        else {
            saveEgoNoise();
            this->suspend();
        }
    }

    //output port
    if (outputPort.getOutputCount()) {
        yDebug("preparing the pointer to image");
        outputImage = &outputPort.prepare();
        outputImage->resize(320, 240);
        // preparing the spectrogramm
        yDebug("preparing the sprectrogram");
        spectrogram();
        //
        yDebug("preparing the image");
        prepareImage(outputImage);
        // outputPort.prepare() = *inputImage; //deprecated

        outputPort.write();
    }

}


bool soundMonitorRatethread::spectrogram() {
    yDebug("spectrogram");
    for(int i = 0; i < 240; i++) {
        (*frequency)[i] = 0.5;
    }
}

bool soundMonitorRatethread::prepareImage(yarp::sig::ImageOf<yarp::sig::PixelRgb>* outputImage) {
    unsigned char* p = outputImage->getRawImage();
    int padding = outputImage->getPadding();
    for (int r = 0; r < outputImage->height(); r++) {
        double vFreq = (*frequency)[r];
        int limit = (int)(320.0 * vFreq);
        for (int c = 0; c < outputImage->width(); c++) {
            if (c > limit) {
                *p = 0; p++;
                *p = 0; p++;
                *p = 0; p++;
            }
            else {
                *p = 255; p++;
                *p = 255; p++;
                *p = 255; p++;
            }
        }
        p+=padding;
    }

}

bool soundMonitorRatethread::processing(Bottle* b){
    counterFrames++;
    yDebug("COUNTERFRAMES: %d bottle length %d",counterFrames, b->size());
    Bottle* matrixBottle = b->get(0).asList();
    yDebug("COUNTERFRAMES: %d bottle MATRIX length %d",counterFrames, matrixBottle->size());
    for(int i = 0; i < SUMDIM; i++) {
        double value = matrixBottle->get(i).asDouble();
        sum[i] = value;
    }
    return true;
}


void soundMonitorRatethread::threadRelease() {
    fclose(pFile);
    inputPort.interrupt();
    outputPort.interrupt();
    inputPort.close();
    outputPort.close();
}


