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

/**
 * @file  audioMemoryMapperRateThread.cpp
 * @brief Implementation of the memory mapper ratethread.
 *        This is where the processing happens.
 */

#include "audioMemoryMapperRateThread.h"

using namespace yarp::os;

#define THRATE 80 //ms


audioMemoryMapperRateThread::audioMemoryMapperRateThread() : RateThread(THRATE) {
    robot = "icub";
}


audioMemoryMapperRateThread::audioMemoryMapperRateThread(std::string _robot, yarp::os::ResourceFinder &rf) : RateThread(THRATE) {
    robot = _robot;
    loadFile(rf);
    createMemoryMappingSection();
}


audioMemoryMapperRateThread::~audioMemoryMapperRateThread() {
    // do nothing
}


bool audioMemoryMapperRateThread::threadInit() {

	if (rawAudioPortActive) {
		if (!rawAudioPort.open(rawAudioPortName.c_str())) {
			yError("unable to open port to receive input");
			return false;  // unable to open; let RFModule know so that it won't run
		}
	}

	if (gammaToneAudioPortActive) {
		if (!gammaToneAudioPort.open(gammaToneAudioPortName.c_str())) {
			yError("unable to open port to receive input");
			return false;  // unable to open; let RFModule know so that it won't run
		}
	}

	if (beamFormedAudioPortActive) {
		if (!beamFormedAudioPort.open(beamFormedAudioPortName.c_str())) {
			yError("unable to open port to receive input");
			return false;  // unable to open; let RFModule know so that it won't run
		}
	}

	if (audioMapEgoPortActive) {
		if (!audioMapEgoPort.open(audioMapEgoPortName.c_str())) {
			yError("unable to open port to receive input");
			return false;  // unable to open; let RFModule know so that it won't run
		}
	}

	if (audioMapAloPortActive) {
		if (!audioMapAloPort.open(audioMapAloPortName.c_str())) {
			yError("unable to open port to receive input");
			return false;  // unable to open; let RFModule know so that it won't run
		}
	}

	if (longTermBayesianMapPortActive) {
		if (!longTermBayesianMapPort.open(longTermBayesianMapPortName.c_str())) {
			yError("unable to open port to receive input");
			return false;  // unable to open; let RFModule know so that it won't run
		}
	}

	if (collapesedBayesianMapPortActive) {
		if (!collapesedBayesianMapPort.open(collapesedBayesianMapPortName.c_str())) {
			yError("unable to open port to receive input");
			return false;  // unable to open; let RFModule know so that it won't run
		}
	}

	return true;
}


void audioMemoryMapperRateThread::setName(std::string str) {
	this->name=str;
}


std::string audioMemoryMapperRateThread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void audioMemoryMapperRateThread::run() {

	if (rawAudioPortActive) {
		memoryMapRawAudio();
	}

	if (gammaToneAudioPortActive) {
		memoryMapGammaToneAudio();
	}

	if (beamFormedAudioPortActive) {
		memoryMapBeamFormedAudio();
	}

	if (audioMapEgoPortActive ) {
		memoryMapAudioMapEgo();
	}

	if (audioMapAloPortActive) {
		memoryMapAudioMapAlo();
	}

	if (longTermBayesianMapPortActive) {
		memoryMapLongTermBayesianMap();
	}

	if (collapesedBayesianMapPortActive) {
		memoryMapCollapesedBayesianMap();
	}

	yInfo("Done Memory Mapping");
}


bool audioMemoryMapperRateThread::processing() {
    // here goes the processing...
    return true;
}


void audioMemoryMapperRateThread::threadRelease() {
    // nothing
}


void audioMemoryMapperRateThread::createMemoryMappingSection() {

	if (rawAudioPortActive) {
		int rawAudioSize = (nMics+2 * frameSamples);
		initializationRawAudioArray = new double(rawAudioSize);
		rawAudioFid = fopen("/tmp/rawAudio.tmp", "w");
		fwrite(initializationRawAudioArray, sizeof(double), rawAudioSize, rawAudioFid);
		fclose(rawAudioFid);
		rawAudioMappedFileID = open("/tmp/rawAudio.tmp", O_RDWR);
		rawAudioData = (double *)mmap(0, (sizeof(initializationRawAudioArray)*rawAudioSize), PROT_WRITE, MAP_SHARED , rawAudioMappedFileID, 0);
	}

	if (gammaToneAudioPortActive) {
		int gammaToneAudioSize = (nBands * nMics * frameSamples);
		initializationGammaToneAudioArray = new double(gammaToneAudioSize);
		gammaToneAudioFid = fopen("/tmp/gammaToneAudio.tmp", "w");
		fwrite(initializationGammaToneAudioArray, sizeof(double), gammaToneAudioSize, gammaToneAudioFid);
		fclose(gammaToneAudioFid);
		gammaToneAudioFileID = open("/tmp/gammaToneAudio.tmp", O_RDWR);
		gammaToneAudioData = (double *)mmap(0, (sizeof(initializationGammaToneAudioArray)*gammaToneAudioSize), PROT_WRITE, MAP_SHARED , gammaToneAudioFileID, 0);
	}

	if (beamFormedAudioPortActive) {
		int beamFormedAudioSize = (nBands * frameSamples * totalBeams);
		initializationBeamFormedAudioArray = new double(beamFormedAudioSize);
		beamFormedAudioFid = fopen("/tmp/beamFormedAudio.tmp", "w");
		fwrite(initializationBeamFormedAudioArray, sizeof(double), beamFormedAudioSize, beamFormedAudioFid);
		fclose(beamFormedAudioFid);
		beamFormedAudioFileID = open("/tmp/beamFormedAudio.tmp", O_RDWR);
		beamFormedAudioData = (double *)mmap(0, (sizeof(initializationBeamFormedAudioArray)*beamFormedAudioSize), PROT_WRITE, MAP_SHARED , beamFormedAudioFileID, 0);
	}

	if (audioMapEgoPortActive) {
		audioMapEgoMatrix = new yarp::sig::Matrix();
		int audioMapEgoSize = (nBands * interpolateNSamples * 2);
		initializationAudioMapEgoArray  = new double[audioMapEgoSize];
		audioMapEgoFid = fopen("/tmp/audioMapEgo.tmp", "w");
		fwrite(initializationAudioMapEgoArray, sizeof(double), audioMapEgoSize, audioMapEgoFid);
		fclose(audioMapEgoFid);
		audioMapEgoFileID = open("/tmp/audioMapEgo.tmp", O_RDWR);
		audioMapEgoData = (double *)mmap(0, (sizeof(initializationAudioMapEgoArray)*audioMapEgoSize), PROT_WRITE, MAP_SHARED , audioMapEgoFileID, 0);
	}

	if (audioMapAloPortActive) {
		int audioMapAloSize = (nBands * interpolateNSamples * 2);
		initializationAudioMapAloArray = new double(audioMapAloSize);
		audioMapAloFid = fopen("/tmp/audioMapAlo.tmp", "w");
		fwrite(initializationAudioMapAloArray, sizeof(double), audioMapAloSize, audioMapAloFid);
		fclose(audioMapAloFid);
		audioMapAloFileID = open("/tmp/audioMapAlo.tmp", O_RDWR);
		audioMapAloData = (double *)mmap(0, (sizeof(initializationAudioMapAloArray)*audioMapAloSize), PROT_WRITE, MAP_SHARED , audioMapAloFileID, 0);
	}

	if (longTermBayesianMapPortActive) {
		int longTermBayesianMapSize = (nBands * interpolateNSamples * 2);
		initializationLongTermBayesianMapArray = new double(longTermBayesianMapSize);
		longTermBayesianMapFid = fopen("/tmp/longTermBayesianMap.tmp", "w");
		fwrite(initializationLongTermBayesianMapArray, sizeof(double), longTermBayesianMapSize, longTermBayesianMapFid);
		fclose(longTermBayesianMapFid);
		longTermBayesianMapFileID = open("/tmp/longTermBayesianMap.tmp", O_RDWR);
		longTermBayesianMapData = (double *)mmap(0, (sizeof(initializationLongTermBayesianMapArray)*longTermBayesianMapSize), PROT_WRITE, MAP_SHARED , longTermBayesianMapFileID, 0);
	}

	if (collapesedBayesianMapPortActive) {
		int collapesedBayesianMapSize = (interpolateNSamples * 2);
		initializationCollapesedBayesianMapArray = new double(collapesedBayesianMapSize);
		collapesedBayesianMapFid = fopen("/tmp/collapesedBayesianMap.tmp", "w");
		fwrite(initializationCollapesedBayesianMapArray, sizeof(double), collapesedBayesianMapSize, collapesedBayesianMapFid);
		fclose(collapesedBayesianMapFid);
		collapesedBayesianMapFileID = open("/tmp/collapesedBayesianMap.tmp", O_RDWR);
		collapesedBayesianMapData = (double *)mmap(0, (sizeof(initializationCollapesedBayesianMapArray)*collapesedBayesianMapSize), PROT_WRITE, MAP_SHARED , collapesedBayesianMapFileID, 0);
	}

	yInfo("Done Creating Memory Mapped Regions");
}


void audioMemoryMapperRateThread::loadFile(yarp::os::ResourceFinder &rf) {

	//
	// get if ports should be activated
	//
	rawAudioPortActive				=  rf.check("rawAudioPortActive",
												Value("false"),
												"Check if rawPort should be Active (str)").asString() == "true";

	gammaToneAudioPortActive		=  rf.check("gammaToneAudioPortActive",
												Value("false"),
												"Check if gammaToneAudioPort should be Active (str)").asString() == "true";

	beamFormedAudioPortActive		=  rf.check("beamFormedAudioPortActive",
												Value("false"),
												"Check if beamFormedAudioPort should be Active (str)").asString() == "true";

	audioMapEgoPortActive			=  rf.check("audioMapEgoPortActive",
												Value("false"),
												"Check if audioMapEgoPort should be Active (str)").asString() == "true";

	audioMapAloPortActive			=  rf.check("audioMapAloPortActive",
												Value("false"),
												"Check if audioMapAloPort should be Active (str)").asString() == "true";

	longTermBayesianMapPortActive	=  rf.check("longTermBayesianMapPortActive",
												Value("false"),
												"Check if longTermBayesianMapPort should be Active (str)").asString() == "true";

	collapesedBayesianMapPortActive	=  rf.check("collapesedBayesianMapPortActive",
												Value("false"),
												"Check if collapesedBayesianMapPort should be Active (str)").asString() == "true";

	yInfo("rawAudioPortActive = %d", rawAudioPortActive);
	yInfo("gammaToneAudioPortActive = %d", gammaToneAudioPortActive);
	yInfo("beamFormedAudioPortActive = %d", beamFormedAudioPortActive);
	yInfo("audioMapEgoPortActive = %d", audioMapEgoPortActive);
	yInfo("audioMapAloPortActive = %d", audioMapAloPortActive);
	yInfo("longTermBayesianMapPortActive = %d", longTermBayesianMapPortActive);
	yInfo("collapesedBayesianMapPortActive = %d", collapesedBayesianMapPortActive);


	//
	// get port names
	//
	rawAudioPortName				=  rf.check("rawAudioPortName",
												Value("/iCubAudioAttention/RawAudio:i"),
												"Check if rawPort should be Active (str)").asString();

	gammaToneAudioPortName			=  rf.check("gammaToneAudioPortName",
												Value("/iCubAudioAttention/GammaToneFilteredAudio:i"),
												"Check if gammaToneAudioPort should be Active (str)").asString();

	beamFormedAudioPortName			=  rf.check("beamFormedAudioPortName",
												Value("/iCubAudioAttention/BeamFormedAudio:i"),
												"Check if beamFormedAudioPort should be Active (str)").asString();

	audioMapEgoPortName				=  rf.check("audioMapEgoPortName",
												Value("/iCubAudioAttention/AudioMapEgo:i"),
												"Check if audioMapEgoPort should be Active (str)").asString();

	audioMapAloPortName				=  rf.check("audioMapAloPortName",
												Value("/iCubAudioAttention/AudioMapAlo:i"),
												"Check if audioMapAloPort should be Active (str)").asString();

	longTermBayesianMapPortName		=  rf.check("longTermBayesianMapPortName",
												Value("/iCubAudioAttention/LongTermBayesianMap:i"),
												"Check if longTermBayesianMapPort should be Active (str)").asString();

	collapesedBayesianMapPortName	=  rf.check("collapesedBayesianMapPortName",
												Value("/iCubAudioAttention/CollapesedBayesianMap:i"),
												"Check if collapesedBayesianMapPort should be Active (str)").asString();

	yInfo("rawAudioPortName = %s", rawAudioPortName.c_str());
	yInfo("gammaToneAudioPortName = %s", gammaToneAudioPortName.c_str());
	yInfo("beamFormedAudioPortName = %s", beamFormedAudioPortName.c_str());
	yInfo("audioMapEgoPortName = %s", audioMapEgoPortName.c_str());
	yInfo("audioMapAloPortName = %s", audioMapAloPortName.c_str());
	yInfo("longTermBayesianMapPortName = %s", longTermBayesianMapPortName.c_str());
	yInfo("collapesedBayesianMapPortName = %s", collapesedBayesianMapPortName.c_str());


	//
	// get values of audioPreprocessing
	//
	yarp::os::ResourceFinder rff;
	int argc;
	char ** argv;
	rff.setDefaultConfigFile("audioConfig.ini");		//overridden by --from parameter
	rff.setDefaultContext("icubAudioAttention/conf");	//overridden by --context parameter
	rff.configure(argc, argv);

	frameSamples		= rff.check("frameSamples",
									Value("4096"),
									"frame samples (int)").asInt();

	nBands				= rff.check("nBands",
									Value("128"),
									"numberBands (int)").asInt();

	nMics				= rff.check("nMics",
									Value("2"),
									"number mics (int)").asInt();

	interpolateNSamples	= rff.check("interpolateNSamples",
									Value("180"),
									"interpellate N samples (int)").asInt();

	micDistance			= rff.check("micDistance",
									Value("0.145"),
									"micDistance (double)").asDouble();

	C					= rff.check("C",
									Value("338"),
									"C speed of sound (int)").asInt();

	samplingRate		= rff.check("samplingRate",
									Value("48000"),
									"sampling rate of mics (int)").asInt();

	longTimeFrame		= rff.check("longBufferSize",
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


void audioMemoryMapperRateThread::memoryMapRawAudio() {

	rawAudioSoundObj = rawAudioPort.read(true);
	rawAudioPort.getEnvelope(ts);
	int currentCounter = ts.getCount();
	double currentTime = ts.getTime();
}


void audioMemoryMapperRateThread::memoryMapGammaToneAudio() {

	gammaToneAudioMatrix = gammaToneAudioPort.read(true);
}


void audioMemoryMapperRateThread::memoryMapBeamFormedAudio() {

	beamFormedAudioMatrix = beamFormedAudioPort.read(true);
}


void audioMemoryMapperRateThread::memoryMapAudioMapEgo() {

	audioMapEgoMatrix = audioMapEgoPort.read(true);
	int count = 0;

	for (int i = 0; i < interpolateNSamples * 2; i++) {
		for (int j = 0; j < nBands; j++) {
	 		audioMapEgoData[count] = *(audioMapEgoMatrix->data() + (count)) ;
	  		count++;
		}
	}
}


void audioMemoryMapperRateThread::memoryMapAudioMapAlo() {

	audioMapAloMatrix = audioMapAloPort.read(true);
	int count = 0;

	for (int i = 0; i < interpolateNSamples * 2; i++) {
		for (int j = 0; j < nBands; j++) {
	 		audioMapAloData[count] = *(audioMapAloMatrix->data() + (count));
	  		count++;
		}
	}
}


void audioMemoryMapperRateThread::memoryMapLongTermBayesianMap() {

	longTermBayesianMatrix = collapesedBayesianMapPort.read(true);
	int count = 0;

	for (int i = 0; i < interpolateNSamples * 2; i++) {
		for (int j = 0; j < nBands; j++) {
	  		longTermBayesianMapData[count] = *(longTermBayesianMatrix->data() + (count));
	  		count++;
		}
	}
}


void audioMemoryMapperRateThread::memoryMapCollapesedBayesianMap() {
	
	collapesedBayesianMatrix = collapesedBayesianMapPort.read(true);
	int count = 0;

	for (int i = 0; i < interpolateNSamples * 2; i++) {
		collapesedBayesianMapData[count] = *(collapesedBayesianMatrix->data() + (count));
		count++;
	}
}
