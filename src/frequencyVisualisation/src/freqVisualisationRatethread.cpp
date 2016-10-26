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
 * @file freqVisualisationRatethread.cpp
 * @brief Implementation of the eventDriven thread (see freqVisualisationRatethread.h).
 */

#include <iCub/freqVisualisationRatethread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 100 //ms

freqVisualisationRatethread::freqVisualisationRatethread():RateThread(THRATE) {
    robot = "icub";        
}

freqVisualisationRatethread::freqVisualisationRatethread(string _robot, string _configFile):RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
}

freqVisualisationRatethread::~freqVisualisationRatethread() {
    // do nothing
}

bool freqVisualisationRatethread::threadInit() {
    // opening the port for direct input
    if (!inputPort.open(getName("/bayesian:i").c_str())) {
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

void freqVisualisationRatethread::setName(string str) {
    this->name=str;
}


std::string freqVisualisationRatethread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void freqVisualisationRatethread::setInputPortName(string InpPort) {
    
}

void freqVisualisationRatethread::run() {    
    //code here .....
    if (inputPort.getInputCount()) {
        Matrix* mat = inputPort.read(false);   //blocking reading for synchr with the input
        if (mat!=NULL) {
            yDebug("matrix is not null");
            if (outputPort.getOutputCount()) {
                yDebug("preparing the image %d %d",mat->cols(),mat->rows() );
                outputImage = &outputPort.prepare();
                outputImage->resize(mat->cols(),mat->rows());
                result = processing(mat);
            }
        }
    }

    if (outputPort.getOutputCount()) {
        yDebug("writing the output image out");
        // changing the pointer of the prepared area for the outputPort.write()
        //outputPort.prepare() = *inputImage;

        outputPort.write();
    }

}

bool freqVisualisationRatethread::processing(Matrix* mat){
    // here goes the processing...
    int nRows = mat->rows();
    int nCols = mat->cols();
    yDebug("Matrix rows %d cols %d ", nRows, nCols );
    imageOutWidth  = mat->cols();
    imageOutHeight = mat->rows();
    unsigned char* pImage = outputImage->getRawImage();
    yDebug("image width %d, height %d", outputImage->width(), outputImage->height());
    double* pMat = mat->data();
    int padding = outputImage->getPadding();
    for (int r = 0; r < nRows; r++) {
        for (int c = 0; c < nCols; c++) {
            *pImage = 0;
            pImage += 3;
            pMat++;
        }
        pImage += padding;
    }
    return true;
}

bool freqVisualisationRatethread::processing(Bottle* b){
    // here goes the processing...
    yDebug("length %d ", b->size());
    int intA = b->get(0).asInt();
    int intB = b->get(1).asInt();
    Bottle* subBottle = b->get(2).asList();
    yDebug("a %d b %d", intA, intB);
    yDebug("bottle length %d: %s", 10, subBottle->toString().c_str());
    return true;
}


void freqVisualisationRatethread::threadRelease() {
    // nothing
    inputPort.interrupt();
    outputPort.interrupt();
    inputPort.close();
    outputPort.close();
}


