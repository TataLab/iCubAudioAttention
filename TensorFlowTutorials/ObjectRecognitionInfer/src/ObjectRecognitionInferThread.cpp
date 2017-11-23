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
 * @file ObjectRecognitionInferThreadRatethread.cpp
 * @brief Implementation of the eventDriven thread (see ObjectRecognitionInferThreadRatethread.h).
 */

#include "../include/iCub/ObjectRecognitionInferThread.h"

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 100 //ms

//********************interactionEngineRatethread******************************************************

ObjectRecognitionInferThread::ObjectRecognitionInferThread(yarp::os::ResourceFinder &rf) : RateThread(THRATE) {
    robot = "icub";

        const string graphPath = rf.find("graph_path").asString().c_str();
        const string labelsPath = rf.find("labels_path").asString().c_str();
        const string modelName = rf.find("model_name").asString().c_str();

        tensorFlowInference = std::unique_ptr<TensorFlowInference>(new TensorFlowInference(graphPath, labelsPath, modelName));

}

ObjectRecognitionInferThread::ObjectRecognitionInferThread(yarp::os::ResourceFinder &rf, string _robot, string _configFile) : RateThread(THRATE) {
    robot = _robot;
    configFile = _configFile;

        const string graphPath = rf.find("graph_path").asString().c_str();
        const string labelsPath = rf.find("labels_path").asString().c_str();
        const string modelName = rf.find("model_name").asString().c_str();

        tensorFlowInference = std::unique_ptr<TensorFlowInference>(new TensorFlowInference(graphPath, labelsPath, modelName));

}

ObjectRecognitionInferThread::~ObjectRecognitionInferThread() {
    // do nothing
}

bool ObjectRecognitionInferThread::threadInit() {

    if (!inputImagePort.open(getName("/imageRGB:i").c_str())) {
        std::cout << ": unable to open port /imageRGB:i " << std::endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }


    if (!outputLabelPort.open(getName("/label:o").c_str())) {
        std::cout << ": unable to open port /label:o " << std::endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    tensorflow::Status initGraphStatus = tensorFlowInference->initGraph();

    if( initGraphStatus != tensorflow::Status::OK()){
        yError("%s",initGraphStatus.ToString().c_str());
        return false;
    }

    yInfo("Initialization of the processing thread correctly ended");

    return true;
}

void ObjectRecognitionInferThread::setName(string str) {
    this->name = str;
}


std::string ObjectRecognitionInferThread::getName(const char *p) {
    string str(name);
    str.append(p);
    return str;
}

void ObjectRecognitionInferThread::setInputPortName(string InpPort) {

}

void ObjectRecognitionInferThread::run() {

}


void ObjectRecognitionInferThread::threadRelease() {
    // nothing

}

std::string ObjectRecognitionInferThread::predictTopClass() {

    yarp::sig::ImageOf<yarp::sig::PixelRgb> *inputImage = inputImagePort.read(true);
    std::vector<tensorflow::Tensor> outputs;


    if (inputImage != NULL) {
        cv::Mat inputImageMat = cv::cvarrToMat(inputImage->getIplImage());
        cv::cvtColor(inputImageMat, inputImageMat, CV_BGR2RGB);

       return tensorFlowInference->inferObject(inputImageMat);

    }


    return std::__cxx11::string();
}

void ObjectRecognitionInferThread::writeToLabelPort(string label) {
    Bottle &labelOutput = outputLabelPort.prepare();
    labelOutput.clear();

    labelOutput.addString(label);
    outputLabelPort.write();


}


