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
 * @file objectRecognitionTrainThreadRatethread.cpp
 * @brief Implementation of the eventDriven thread (see objectRecognitionTrainThreadRatethread.h).
 */

#include "../include/iCub/objectRecognitionTrainThread.h"

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 100 //ms


//********************ObjectRecognitionRatethread******************************************************

objectRecognitionTrainThread::objectRecognitionTrainThread(yarp::os::ResourceFinder &rf) : RateThread(THRATE) {
    robot = "icub";
    inputImage = new ImageOf<PixelRgb>;



    launcher_tensor = new TensorFlowLauncher(rf);



}

objectRecognitionTrainThread::objectRecognitionTrainThread(string _robot, string _configFile,
                                                           yarp::os::ResourceFinder &rf) : RateThread(THRATE) {
    robot = _robot;
    configFile = _configFile;
    inputImage = new ImageOf<PixelRgb>;

    launcher_tensor = new TensorFlowLauncher(rf);

}

objectRecognitionTrainThread::~objectRecognitionTrainThread() {
    // do nothing
}

bool objectRecognitionTrainThread::threadInit() {


    yInfo("Initialization of the processing thread correctly ended");

    if (!inputImagePort.open(getName("/imageRGB:i").c_str())) {
        cout << ": unable to open port /imageRGB:i " << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!logPort.open(getName("/trainingLog:o").c_str())) {
        cout << ": unable to open port /trainingLog:o " << endl;
        return false;  // unable to open; let RFModule know so that it won't run
    }


    if(!launcher_tensor->checkParameters()){
        yError("Check your parameters there were an error");
        return false;  // unable find the required parameters
    }

    /* get the log directory name which will be used to save the train dataset*/
    outputDirectoryName = launcher_tensor->parametersMap["output_graph"];


    // Store the full path to the dataSet
    pathDataSet = launcher_tensor->parametersMap["image_dir"];

    if(pathDataSet == "" || outputDirectoryName == ""){
        yError("You have to specify output_dir and image_dir");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    return true;
}

void objectRecognitionTrainThread::setName(string str) {
    this->name = str;
}


std::string objectRecognitionTrainThread::getName(const char *p) {
    string str(name);
    str.append(p);
    return str;
}

void objectRecognitionTrainThread::setInputPortName(string InpPort) {

}

void objectRecognitionTrainThread::run() {

}


void objectRecognitionTrainThread::threadRelease() {
    // nothing

}

void objectRecognitionTrainThread::saveImageWithLabel(std::string t_labelName) {

    inputImage = inputImagePort.read(true);

    checkCreateDirectory(this->pathDataSet);

    if (inputImage != NULL) {
        const string pathDirLabel =  this->pathDataSet + "/" + t_labelName + "/";
        const string uniqFileIdentifier = getNumberFiles(pathDirLabel)+".jpeg";
        const string fileName = pathDirLabel + uniqFileIdentifier;

        cv::Mat rgbImageMat;
        cv::Mat bgrImage = cv::cvarrToMat(inputImage->getIplImage());

        cv::cvtColor(bgrImage, rgbImageMat, CV_BGR2RGB);


        cv::imwrite(fileName, rgbImageMat);


    }
}

string objectRecognitionTrainThread::getNumberFiles(string t_directoryPath) {

    int uniqFileName = 0;

    DIR *dir = checkCreateDirectory(t_directoryPath);
    struct dirent *ent;
    if (dir != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if(!S_ISDIR(ent->d_type) && ent->d_type != 4 ){
                uniqFileName++;
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror (" could not open directory  " );
        return "";
    }

    return to_string(uniqFileName);
}

void objectRecognitionTrainThread::addImagesToDataSet(std::string t_labelName, int t_imagesNumber) {

    for(int i = 0 ; i < t_imagesNumber ; ++i){
        saveImageWithLabel(t_labelName);
    }
}

DIR* objectRecognitionTrainThread::checkCreateDirectory(std::string t_directoryPath) {

    DIR *dir = opendir(t_directoryPath.c_str());
    if(dir==NULL){
        int status = mkdir(t_directoryPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        if(status ==-1){
            const string errorMessage = "Unable to create directory "+t_directoryPath;
            yError("%s", errorMessage.c_str());
        }

        dir = opendir(t_directoryPath.c_str());
    }

    return dir;



}

std::string objectRecognitionTrainThread::launchTraining() {

    launcher_tensor->launchTensorFlowRetraining(logPort);

    return "";

}




