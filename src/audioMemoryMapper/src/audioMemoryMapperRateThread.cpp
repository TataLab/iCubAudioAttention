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

audioMemoryMapperRateThread::audioMemoryMapperRateThread(string _robot, yarp::os::ResourceFinder &rf):RateThread(THRATE){
    robot = _robot;
    loadFile(rf);
    createMemoryMappingSection();


}

audioMemoryMapperRateThread::~audioMemoryMapperRateThread() {
    // do nothing
}

bool audioMemoryMapperRateThread::threadInit() {


    if(rawAudioPortActive){
      if(!rawAudioPort.open(rawAudioPortName.c_str())) {
        yError("unable to open port to receive input");
          return false;  // unable to open; let RFModule know so that it won't run
        }
      }
      if(gammaToneAudioPortActive){
        if (!gammaToneAudioPort.open(gammaToneAudioPortName.c_str())) {
          yError("unable to open port to receive input");
          return false;  // unable to open; let RFModule know so that it won't run
        }
      }
      if(beamFormedAudioPortActive){
        if (!beamFormedAudioPort.open(beamFormedAudioPortName.c_str())) {
          yError("unable to open port to receive input");
          return false;  // unable to open; let RFModule know so that it won't run
        }
      }
      if(audioMapEgoPortActive){
        if (!audioMapEgoPort.open(audioMapEgoPortName.c_str())) {
          yError("unable to open port to receive input");
          return false;  // unable to open; let RFModule know so that it won't run
        }
      }
      if(audioMapAloPortActive){
        if (!audioMapAloPort.open(audioMapAloPortName.c_str())) {
          yError("unable to open port to receive input");
          return false;  // unable to open; let RFModule know so that it won't run
        }
      }
      if(longTermBayesianMapPortActive){
        if (!longTermBayesianMapPort.open(longTermBayesianMapPortName.c_str())) {
          yError("unable to open port to receive input");
          return false;  // unable to open; let RFModule know so that it won't run
        }
      }
      if(collapesedBayesianMapPortActive){
        if (!collapesedBayesianMapPort.open(collapesedBayesianMapPortName.c_str())) {
          yError("unable to open port to receive input");
          return false;  // unable to open; let RFModule know so that it won't run
        }
      }
    return true;
}

void audioMemoryMapperRateThread::setName(string str) {
    this->name=str;
}


std::string audioMemoryMapperRateThread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void audioMemoryMapperRateThread::run() {
    if(rawAudioPortActive){

  }
  if(gammaToneAudioPortActive){

  }
  if(beamFormedAudioPortActive){

  }
  if(audioMapEgoPortActive){

  }
  if(audioMapAloPortActive){

  }
  if(longTermBayesianMapPortActive){

  }
  if(collapesedBayesianMapPortActive){

  }


}

bool audioMemoryMapperRateThread::processing(){
    // here goes the processing...
    return true;
}


void audioMemoryMapperRateThread::threadRelease() {
    // nothing

}

void audioMemoryMapperRateThread::createMemoryMappingSection(){

  if(rawAudioPortActive){

    int rawAudioSize = (nMics+2 * frameSamples);
    initializationRawAudioArray = new double(rawAudioSize);
  //  yInfo("initializationRawAudioArray size = %d", rawAudioSize);
    rawAudioFid = fopen("/tmp/rawAudio.tmp", "w");
    fwrite(initializationRawAudioArray, sizeof(double), rawAudioSize, rawAudioFid);
    fclose(rawAudioFid);
    rawAudioMappedFileID = open("/tmp/rawAudio.tmp", O_RDWR);
    rawAudioData = (double *)mmap(0, (sizeof(initializationRawAudioArray)*rawAudioSize), PROT_WRITE, MAP_SHARED , rawAudioMappedFileID, 0);
  }
  if(gammaToneAudioPortActive){
    int gammaToneAudioSize = (nBands * nMics * frameSamples);
    initializationGammaToneAudioArray = new double(gammaToneAudioSize);
    gammaToneAudioFid = fopen("/tmp/gammaToneAudio.tmp", "w");
    fwrite(initializationGammaToneAudioArray, sizeof(double), sizeof(initializationGammaToneAudioArray), gammaToneAudioFid);
    fclose(gammaToneAudioFid);
    gammaToneAudioFileID = open("/tmp/gammaToneAudio.tmp", O_RDWR);
    gammaToneAudioData = (double *)mmap(0, (sizeof(initializationGammaToneAudioArray)), PROT_WRITE, MAP_SHARED , gammaToneAudioFileID, 0);
  }
  if(beamFormedAudioPortActive){
    int beamFormedAudioSize = (nBands * frameSamples * totalBeams);
    initializationBeamFormedAudioArray = new double(beamFormedAudioSize);
    beamFormedAudioFid = fopen("/tmp/beamFormedAudio.tmp", "w");
    fwrite(initializationBeamFormedAudioArray, sizeof(double), sizeof(initializationBeamFormedAudioArray), beamFormedAudioFid);
    fclose(beamFormedAudioFid);
    beamFormedAudioFileID = open("/tmp/beamFormedAudio.tmp", O_RDWR);
    beamFormedAudioData = (double *)mmap(0, (sizeof(initializationBeamFormedAudioArray)), PROT_WRITE, MAP_SHARED , beamFormedAudioFileID, 0);
  }
  if(audioMapEgoPortActive){
    audioMapEgoMatrix = new yarp::sig::Matrix();
    int audioMapEgoSize = (nBands * interpolateNSamples * 2);
    initializationAudioMapEgoArray  = new double(audioMapEgoSize);
    audioMapEgoFid = fopen("/tmp/audioMapEgo.tmp", "w");
    fwrite(initializationAudioMapEgoArray, sizeof(double), sizeof(initializationAudioMapEgoArray), audioMapEgoFid);
    fclose(audioMapEgoFid);
    audioMapEgoFileID = open("/tmp/audioMapEgo.tmp", O_RDWR);
    audioMapEgoData = (double *)mmap(0, (sizeof(initializationAudioMapEgoArray)*audioMapEgoSize), PROT_WRITE, MAP_SHARED , audioMapEgoFileID, 0);
  }
  if(audioMapAloPortActive){
    int audioMapAloSize = (nBands * interpolateNSamples * 2);
    initializationAudioMapAloArray = new double(audioMapAloSize);
    audioMapAloFid = fopen("/tmp/audioMapAlo.tmp", "w");
    fwrite(initializationAudioMapAloArray, sizeof(double), sizeof(initializationAudioMapAloArray), audioMapAloFid);
    fclose(audioMapAloFid);
    audioMapAloFileID = open("/tmp/audioMapAlo.tmp", O_RDWR);
    audioMapAloData = (double *)mmap(0, (sizeof(initializationAudioMapAloArray)), PROT_WRITE, MAP_SHARED , audioMapAloFileID, 0);
  }
  if(longTermBayesianMapPortActive){
    int longTermBayesianMapSize = (nBands * interpolateNSamples * 2);
    initializationLongTermBayesianMapArray = new double(longTermBayesianMapSize);
    longTermBayesianMapFid = fopen("/tmp/longTermBayesianMap.tmp", "w");
    fwrite(initializationLongTermBayesianMapArray, sizeof(double), sizeof(initializationLongTermBayesianMapArray), longTermBayesianMapFid);
    fclose(longTermBayesianMapFid);
    longTermBayesianMapFileID = open("/tmp/longTermBayesianMap.tmp", O_RDWR);
    longTermBayesianMapData = (double *)mmap(0, (sizeof(initializationLongTermBayesianMapArray)), PROT_WRITE, MAP_SHARED , longTermBayesianMapFileID, 0);
  }
  if(collapesedBayesianMapPortActive){
    int collapesedBayesianMapSize = (interpolateNSamples * 2);
    initializationCollapesedBayesianMapArray = new double(collapesedBayesianMapSize);
    collapesedBayesianMapFid = fopen("/tmp/collapesedBayesianMap.tmp", "w");
    fwrite(initializationCollapesedBayesianMapArray, sizeof(double), sizeof(initializationCollapesedBayesianMapArray), collapesedBayesianMapFid);
    fclose(collapesedBayesianMapFid);
    collapesedBayesianMapFileID = open("/tmp/collapesedBayesianMap.tmp", O_RDWR);
    collapesedBayesianMapData = (double *)mmap(0, (sizeof(initializationCollapesedBayesianMapArray)), PROT_WRITE, MAP_SHARED , collapesedBayesianMapFileID, 0);
  }
  yInfo("Done Creating Memory Mapped Regions");
}

void audioMemoryMapperRateThread::loadFile(yarp::os::ResourceFinder &rf)
{

  rawAudioPortActive = rf.check("rawAudioPortActive",
                           Value("false"),
                           "Check if rawPort should be Active (str)").asString() == "true";

  gammaToneAudioPortActive = rf.check("gammatoneAudioPortActive",
                           Value("false"),
                           "Check if gammaToneAudioPort should be Active (str)").asString() == "true";

  beamFormedAudioPortActive = rf.check("beamFormedAudioPortActive",
                           Value("false"),
                           "Check if beamFormedAudioPort should be Active (str)").asString() == "true";

  audioMapEgoPortActive = rf.check("audioMapEgoPortActive",
                           Value("false"),
                           "Check if audioMapEgoPort should be Active (str)").asString() == "true";

  audioMapAloPortActive = rf.check("audioMapAloPortActive",
                           Value("false"),
                           "Check if audioMapAloPort should be Active (str)").asString() == "true";

  longTermBayesianMapPortActive = rf.check("longTermBayesianMapPortActive",
                           Value("false"),
                           "Check if longTermBayesianMapPort should be Active (str)").asString() == "true";

  collapesedBayesianMapPortActive = rf.check("collapesedBayesianMapPortActive",
                           Value("false"),
                           "Check if collapesedBayesianMapPort should be Active (str)").asString() == "true";


    yInfo("rawAudioPortActive = %d",rawAudioPortActive);
    yInfo("gammaToneAudioPortActive = %d",gammaToneAudioPortActive);
    yInfo("beamFormedAudioPortActive = %d",beamFormedAudioPortActive);
    yInfo("audioMapEgoPortActive = %d",audioMapEgoPortActive);
    yInfo("audioMapAloPortActive = %d",audioMapAloPortActive);
    yInfo("longTermBayesianMapPortActive = %d",longTermBayesianMapPortActive);
    yInfo("collapesedBayesianMapPortActive = %d",collapesedBayesianMapPortActive);

    //TODO Check this Out Man
    yarp::os::ResourceFinder rff;
    int argc;
    char ** argv;
    rff.setDefaultConfigFile("audioConfig.ini");    //overridden by --from parameter
    rff.setDefaultContext("icubAudioAttention/conf");    //overridden by --context parameter
    rff.configure(argc, argv);

    frameSamples  = rff.check("frameSamples",
        Value("4096"),
        "frame samples (int)").asInt();
    nBands  = rff.check("nBands",
        Value("128"),
        "numberBands (int)").asInt();
    nMics  = rff.check("nMics",
        Value("2"),
        "number mics (int)").asInt();
    interpolateNSamples  = rff.check("interpolateNSamples",
         Value("180"),
         "interpellate N samples (int)").asInt();
    micDistance = rff.check("micDistance",
         Value("0.145"),
         "micDistance (double)").asDouble();
    C = rff.check("C",
         Value("338"),
         "C speed of sound (int)").asInt();
    samplingRate = rff.check("samplingRate",
         Value("48000"),
         "sampling rate of mics (int)").asInt();
    longTimeFrame = rff.check("longBufferSize",
         Value("360"),
         "long Buffer Size (int)").asInt();

    nBeamsPerHemi  = (int)((micDistance / C) * samplingRate) - 1;
    yInfo("_beamsPerHemi %d = %f / %d * %d", nBeamsPerHemi, micDistance, C, samplingRate);
    totalBeams = nBeamsPerHemi * 2 + 1;
    yInfo("frameSamples = %d", frameSamples);
    yInfo("nBands = %d", nBands);
    yInfo("nMics = %d", nMics);
    yInfo("interpolateNSamples = %d", interpolateNSamples );
    yInfo("total beams = %d",totalBeams);
}


void audioMemoryMapperRateThread::memoryMapRawAudio(){
  // int currentCounter = ts.getCount();
  // double currentTime = ts.getTime();
  // int row = 0;
  // int j = 0;
  //   for (int col = 0 ; col < frameSamples; col+=1) {
  //
  //   mappedRawAduioData[j] =   (double)rawAudio[row];
  //   mappedRawAduioData[j+1] =   (double)rawAudio[row+1];
  //   mappedRawAduioData[j+2] =   (double)(currentCounter * frameSamples) + col;
  //     mappedRawAduioData[j+3] =   (double)(currentTime + col * (1.0 / samplingRate));
  //     row += 2;
  //     j +=4;
  //   }
}
void audioMemoryMapperRateThread::memoryMapGammaToneAudio(){
  // for (int i = 0; i < nBands; i++)
  // {
  //   for (int j = 0; j < frameSamples; j++)
  //   {
  //     mappedGammaToneFilteredAduioData[(i*frameSamples)+j] = (double)gammatoneAudio[i][j];
  //   }
  // }
  // int visted = (nBands*frameSamples);
  // for (int i = 0; i < nBands; i++)
  // {
  //   for (int j = 0; j < frameSamples; j++)
  //   {
  //     mappedGammaToneFilteredAduioData[(i*frameSamples)+j+visted] = (double)gammatoneAudio[i + nBands][j];
  //   }
  // }
}
void audioMemoryMapperRateThread::memoryMapBeamFormedAudio(){

}
void audioMemoryMapperRateThread::memoryMapAudioMapEgo(){
  audioMapEgoMatrix = audioMapEgoPort.read(true);
  int count = 0;
  // //printf("inpterellateNSamples = %d",interpolateNSamples);
  for (int i = 0; i < interpolateNSamples * 2; i++)
  {
    for (int j = 0; j < nBands; j++)
    {
      audioMapEgoData[(count++)] = *audioMapEgoMatrix[i][j];
    }
  }
}
void audioMemoryMapperRateThread::memoryMapAudioMapAlo(){
// mappedAudioData[0] = ts.getCount();
//   mappedAudioData[1] = ts.getTime();
//   int count = 0;
//   //printf("inpterellateNSamples = %d",interpolateNSamples);
//   for (int i = 0; i < interpolateNSamples * 2; i++)
//   {
//     for (int j = 0; j < nBands; j++)
//     {
//       mappedAudioData[(count++) + 2] = highResolutionAudioMap[i][j];
//     }
//   }
}
void audioMemoryMapperRateThread::memoryMapLongTermBayesianMap(){
   // //Loops though the input(probabilityMap) and does a deep copy of all the elements into a memory mapped location identified by the probabilityMappingFileID
   //  int count = 0;
   //  for (int i = 0; i <  nBands; i++)
   //  {
   //      for (int j = 0; j <interpolateNSamples * 2; j++)
   //      {
   //        //printf("count = %d\n",count);

   //          probabilityMappingFileID[(count++)] = probabilityMap[i][j];
   //      }
   //  }
   //  //probabilityMappingFileID[(count++)] = offset;
}
void audioMemoryMapperRateThread::memoryMapCollapesedBayesianMap(){

}
