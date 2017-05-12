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

#ifndef _BAYESIAN_MODULE_H_
#define _BAYESIAN_MODULE_H_

#include "../../Configuration/ConfigParser.h"

#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Property.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/IEncoders.h>
#include <cstring> //for hardcoded path to noiseMap

#include <vector>
#include <queue>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

class BayesianModule: public yarp::os::RFModule
{
public:
    BayesianModule();
    ~BayesianModule();

    bool configure(yarp::os::ResourceFinder &rf);
    double getPeriod();
    bool updateModule();
    bool interruptModule();
    bool close();

    /**
    *   getLongProbabilityMap
    *   
    */
    std::vector <std::vector <double>> getLongProbabilityMap();

    /**
    *   getMediumProbabilityMap
    *  
    */
    std::vector <std::vector <double>> getMediumProbabilityMap();

    /**
    *   getShortProbabilityMap
    *  
    */
    std::vector <std::vector <double>> getShortProbabilityMap();

private:
    
    /**
    *   voidfindPeaks
    *   This function will look at the vector probabilityMap which is passed in as input and find the peaks saving them to peakMap.
    *   @param probabilityMap a map of the auditory sean with probabilities that a given sound is at a given angle. 
    *   @param peakMap will contain ones to mark the peaks found and zeros everywhere else.   
    */
    void findPeaks(std::vector<double> &peakMap, const std::vector<double> &probabilityMap);

    /**
    *   setAcousticMap
    *  
    */
    void setAcousticMap();


    /**
    *   normalizePropabilityMap
    *   @param
    */
    void normalizePropabilityMap(std::vector <std::vector <double>> &probabilityMap);

    /**
    *   loadFile
    *   Accesses the loadFile.xml that is found in the root directory of this
    *   module and load all required parameters for the beam former.
    */
    void loadFile();

    /**
    *   calcOffset
    *   Calculates the off set that is needed to convert the Ego acoustic map into an Alo acoustic map.
    */
    void calcOffset();

    /**
    *   createMemoryMappedFile
    *   Allocates the required memory needed and the FID need for the memory mapped locations. 
    */
    void createMemoryMappedFile();

    /**
    *   memoryMapper
    *   Memory maps the given probability map to file given by the probablilityMapID
    *   @param
    *   @param probabilityMappingFileID
    */
    void memoryMapper(std::vector <std::vector <double>> probabilityMap, double* probabilityMappingFileID);

    /**
    *   sendAudioMap
    *   Sends the audio map VIA the output port provided.
    *   @param 
    */
    void sendAudioMap(std::vector <std::vector <double>> &probabilityMap);

    /**
    *   createBaysianMaps
    *  
    */
    void createBaysianMaps();

    /**
    *   createBaysianMaps
    *  
    */
    void createNoiseMaps();

    void removeNoise(std::vector <std::vector <double>> &probabilityMap);
    void removeMap(std::vector <std::vector <double>> &probabilityMap, std::vector <std::vector <double>> &cAM);
    void addMap(std::vector <std::vector <double>> &probabilityMap, std::vector <std::vector <double>> &cAM);

    void collapseMap(const std::vector <std::vector <double>> &inputMap, std::vector <double> &outputProbabilityMap);

    //Variables need to time the update module
    struct timeval st, en;
    long mtime, seconds, useconds;


    yarp::os::BufferedPort<yarp::sig::Matrix> *inPort;
    yarp::os::Port *outPort;
    yarp::os::Port *outAngle;

	const std::string noiseMapPath="./noiseMap.dat";

	std::string robotName;
    std::vector <std::vector <double>> longMap;
    std::vector <std::vector <double>> mediumMap;
    std::vector <std::vector <double>> shortMap;

    std::vector <std::vector <double>> currentAudioMap;
    std::vector <std::vector <double>> noiseMap;

    std::vector <double> longProbabilityAngleMap;

    yarp::sig::Matrix *inputMatrix;
    yarp::sig::Matrix *outputMatrix;

    std::queue <std::vector <std::vector <double>>> bufferedMap;

    std::queue <double> bufferedOffSet;

    yarp::os::Property options;
    yarp::dev::PolyDriver *robotHead;
    yarp::dev::IEncoders *enc;
    IPositionControl *pos;

    yarp::os::Stamp ts;

    //Memory mapping variables
    FILE *fidShort;
    int mappingFileIDShort;
    double *probabilityMappingShort;
    FILE *fidMedium;
    int mappingFileIDMedium;
    double *probabilityMappingMedium;
    FILE *fidLong;
    int mappingFileIDLong;
    double *probabilityMappingLong;

    FILE *fidLongProbabilityAngle;
    int mappingFileIDLongProbabilityAngle;
    double *probabilityMappingLongProbabilityAngle;


    int nBands;
    std::string fileName;
    double offset;
    int interpellateNSamples;
    int longTimeFrame;
    int mediumTimeFrame;
    int shortTimeFrame;

    int numberOfNoiseMaps;

    int noiseBufferMap;
    bool first;
};

#endif
