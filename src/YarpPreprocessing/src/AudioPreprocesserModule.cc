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
 * @file AudioPreprocesserModule.cpp
 * @brief Implementation of the processing module 
 */

#include "AudioPreprocesserModule.h"

using namespace yarp::os;

AudioPreprocesserModule::AudioPreprocesserModule()
{
	//Colouring to match the colour code of yarp
	myerror = "\033[0;31m";
	myinfo = "\033[0;32m";
	mywarn = "\033[0;33m";
	myreset = "\033[0m";

	//TODO how should this path be better protected
	fileName = "../../src/Configuration/loadFile.xml";
	loadFile();

	gammatonAudioFilter = new GammatonFilter(fileName);
	beamForm = new BeamFormer(fileName);
	rawAudio = new float[(frameSamples * nMics)];
	oldtime = 0;

	for (int i = 0; i < interpellateNSamples * 2; i++) {
		std::vector<double> tempvector;
		for (int j = 0; j < nBands; j++) {
			tempvector.push_back(0);
		}
		highResolutionAudioMap.push_back(tempvector);
	}

	outAudioMap = new yarp::sig::Matrix(nBands, interpellateNSamples * 2);
	createMemoryMappedFile();

}

AudioPreprocesserModule::~AudioPreprocesserModule()
{
	delete gammatonAudioFilter;
	delete beamForm;
	delete rawAudio;

}

bool AudioPreprocesserModule::configure(yarp::os::ResourceFinder &rf)
{
  /* Process all parameters from both command-line and .ini file */
  
  /* get the module name which will form the stem of all module port names */
  moduleName            = rf.check("name", 
				   Value("/AudioPreprocessor"), 
				   "module name (string)").asString();
  /*
   * before continuing, set the module name before getting any other parameters, 
   * specifically the port names which are dependent on the module name
   */
  setName(moduleName.c_str());
  
  /*
   * get the robot name which will form the stem of the robot ports names
   * and append the specific part and device required
   */
  robotName             = rf.check("robot", 
				   Value("icub"), 
				   "Robot name (string)").asString();
  robotPortName         = "/" + robotName + "/head";
  
  inputPortName           = rf.check("inputPortName",
				     Value(":i"),
				     "Input port name (string)").asString();
  inPort = new yarp::os::BufferedPort<yarp::sig::Sound>();
  inPort->open("/iCubAudioAttention/Preprocesser:i");
  
  audioMapPort = new yarp::os::Port();
  audioMapPort->open("/iCubAudioAttention/Preprocesser:o");
  //TODO this show not be done here
  
  if (yarp::os::Network::exists("/iCubAudioAttention/Preprocesser:i"))
    {
      if (yarp::os::Network::connect("/sender", "/iCubAudioAttention/Preprocesser:i") == false)
	{
	  std::cout << myerror << "[ERROR]" << myreset << " Could not make connection to /sender. Exiting.\n";
	  yError("Could not make connection to /sender. Exiting");
	  return false;
	}
    }
  else
    {
      return false;
    }

  if (rf.check("config")) {
        configFile=rf.findFile(rf.find("config").asString().c_str());
        if (configFile=="") {
            return false;
        }
    }
    else {
        configFile.clear();
    }
  
  /* create the thread and pass pointers to the module parameters */
  apr = new AudioPreprocesserRatethread(robotName, configFile);
  apr->setName(getName().c_str());
  //rThread->setInputPortName(inputPortName.c_str());
  
  /* now start the thread to do the work */
  //apr->start(); // this calls threadInit() and it if returns true, it then calls run()
  
  return true;
}

double AudioPreprocesserModule::getPeriod()
{
	// TODO Should this all ways stay this low
	return 0.05;
}

bool AudioPreprocesserModule::updateModule()
{
	s = inPort->read(true);
	gettimeofday(&st, NULL);
	inPort->getEnvelope(ts);

	if (ts.getCount() != lastframe + 1) {
		printf("[WARN] Too Slow\n");
	}

	int row = 0;
	for (int col = 0 ; col < frameSamples; col++) {
		yarp::os::NetInt16 temp_c = (yarp::os::NetInt16) s->get(col, 0);
		yarp::os::NetInt16 temp_d = (yarp::os::NetInt16) s->get(col, 1);
		rawAudio[row]   = (float) temp_c / normDivid;
		rawAudio[row + 1]	= (float) temp_d / normDivid;
		row += 2;
	}

	gammatonAudioFilter->inputAudio(rawAudio);
	 if (gammatonFilteredAudioPort.getOutputCount()) {
		sendAudioMap();
		gammatonFilteredAudioPort->setEnvelope(ts);
		gammatonFilteredAudioPort->write(*outAudioMap);
    }
	beamForm->inputAudio(gammatonAudioFilter->getFilteredAudio());
	reducedBeamFormedAudioVector = beamForm->getReducedBeamAudio();
	if (beamFormedAudioPort.getOutputCount()) {
		sendAudioMap();
		beamFormedAudioPort->setEnvelope(ts);
		beamFormedAudioPort->write(*outAudioMap);
    }
	spineInterp();
	memoryMapper();
	if (audioMapPort.getOutputCount()) {
		sendAudioMap();
		audioMapPort->setEnvelope(ts);
		audioMapPort->write(*outAudioMap);
    }

	//Timing how long the module took
	lastframe = ts.getCount();
	gettimeofday(&en, NULL);
	seconds  = en.tv_sec  - st.tv_sec;
	useconds = en.tv_usec - st.tv_usec;
	mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;
	printf("[INFO] Count:%d Time:%ld milliseconds. \n", ts.getCount(),  mtime);

	return true;
}

bool AudioPreprocesserModule::interruptModule()
{
	fprintf(stderr, "[WARN] Interrupting\n");
	return true;
}

bool AudioPreprocesserModule::close()
{
	fprintf(stderr, "[INFO] Calling close\n");
	return true;
}

void AudioPreprocesserModule::createMemoryMappedFile()
{
	int memoryMapSize = ((nBands * (interpellateNSamples + 1)) * 2 + 2);
	double initializationArray [memoryMapSize];
	fid = fopen("/tmp/preprocessedAudioMap.tmp", "w");
	fwrite(initializationArray, sizeof(double), sizeof(initializationArray), fid);
	fclose(fid);
	mappedFileID = open("/tmp/preprocessedAudioMap.tmp", O_RDWR);
	std::cout << sizeof(initializationArray) << std::endl;
	mappedAudioData = (double *)mmap(0, (sizeof(initializationArray)), PROT_WRITE, MAP_SHARED , mappedFileID, 0);
}

void AudioPreprocesserModule::loadFile()
{

	ConfigParser *confPars;
	try {
		confPars = ConfigParser::getInstance(fileName);
		Config pars = (confPars->getConfig("default"));

		frameSamples = pars.getFrameSamples();
		nBands = pars.getNBands();
		nMics = pars.getNMics();
		interpellateNSamples = pars.getInterpellateNSamples();
		totalBeams = pars.getNBeamsPerHemifield() * 2 + 1;
	}
	catch (int a) {

	}


}

void AudioPreprocesserModule::memoryMapper()
{
	mappedAudioData[0] = ts.getCount();
	mappedAudioData[1] = ts.getTime();
	int count = 0;
	for (int i = 0; i < interpellateNSamples * 2; i++)
	{
		for (int j = 0; j < nBands; j++)
		{
			mappedAudioData[(count++) + 2] = highResolutionAudioMap[i][j];
		}
	}

}
void AudioPreprocesserModule::sendAudioMap()
{
	for (int i = 0; i < nBands; i++)
	{
		yarp::sig::Vector tempV(interpellateNSamples * 2);
		for (int j = 0; j < interpellateNSamples * 2; j++)
		{
			tempV[j] = highResolutionAudioMap[j][i];
		}
		outAudioMap->setRow(i, tempV);
	}
}
void AudioPreprocesserModule::sendGammatonFilteredAudio()
{
	for (int i = 0; i < nBands; i++)
	{
		yarp::sig::Vector tempV(interpellateNSamples * 2);
		for (int j = 0; j < interpellateNSamples * 2; j++)
		{
			tempV[j] = highResolutionAudioMap[j][i];
		}
		outAudioMap->setRow(i, tempV);
	}
}
void AudioPreprocesserModule::sendGammatonFilteredAudio()
{
	for (int i = 0; i < nBands; i++)
	{
		yarp::sig::Vector tempV(interpellateNSamples * 2);
		for (int j = 0; j < interpellateNSamples * 2; j++)
		{
			tempV[j] = highResolutionAudioMap[j][i];
		}
		outAudioMap->setRow(i, tempV);
	}
}

void AudioPreprocesserModule::spineInterp()
{
	double offset = (interpellateNSamples / (double)totalBeams);
	for (int i = 0; i < nBands; i++)
	{
		int k = 0;
		double curroffset = 0;
		for (int j = 0; j < interpellateNSamples; j++)
		{
			if (j == (int)curroffset && k < (totalBeams - 1))
			{
				curroffset += offset;
				k++;
			}
			highResolutionAudioMap[j][i] = interpl(j, curroffset - offset, curroffset , reducedBeamFormedAudioVector[k - 1][i], reducedBeamFormedAudioVector[k][i]);
		}
		curroffset = interpellateNSamples;
		for (int j = interpellateNSamples; j < interpellateNSamples * 2; j++)
		{
			if (j == (int)curroffset && k > 1)
			{
				if (j != interpellateNSamples)
					k--;
				curroffset += offset;
			}
			highResolutionAudioMap[j][i] = interpl(j, curroffset - offset, curroffset, reducedBeamFormedAudioVector[k][i], reducedBeamFormedAudioVector[k - 1][i]);
		}
	}

}

double AudioPreprocesserModule::interpl(int x, int x1, int x2, double y1, double y2)
{
	return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
}
