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
 * @file AudioPreprocesserModule.cpp
 * @brief Implementation of the processing module
 */

#include "audioPreprocesserModule.h"
#define NUM_FRAME_SAMPLES 4096 //get this into resource finder!!!!



using namespace yarp::os;

AudioPreprocesserModule::AudioPreprocesserModule()
{
    yDebug("AudioProcesserModule");

}

AudioPreprocesserModule::~AudioPreprocesserModule()
{
	delete gammatoneAudioFilter;
	delete beamForm;
	delete rawAudio;

}

bool AudioPreprocesserModule::configure(yarp::os::ResourceFinder &rf)
{
    yInfo("Configuring the module");
	inPort = new yarp::os::BufferedPort<yarp::sig::Sound>();
	//inPort = new yarp::os::BufferedPort<audio::Sound>();
	inPort->open("/iCubAudioAttention/Preprocesser:i");

	outPort = new yarp::os::Port();
	outPort->open("/iCubAudioAttention/Preprocesser:o");

	outGammaToneFilteredAudioPort = new yarp::os::Port();
	outGammaToneFilteredAudioPort->open("/iCubAudioAttention/GammaToneFilteredAudio:o");

	if (yarp::os::Network::exists("/iCubAudioAttention/Preprocesser:i"))
	{
		if (yarp::os::Network::connect("/sender", "/iCubAudioAttention/Preprocesser:i") == false)
		{
			
			yError("Could not make connection to /sender. \nExiting. \n");
			return false;
		}
	}
	else
	{
		return false;
	}


   	//Set the file which the module uses to grab the config information
    yInfo("loading configuration file");
	//calls the parser and the config file to configure the needed variables in this class
	loadFile(rf);
    yInfo("file successfully load");
    
    //preparing GammatoneFilter and beamForming
	gammatoneAudioFilter = new GammatoneFilter(samplingRate, lowCf, highCf, nBands, frameSamples, nMics, false, false);
	beamForm = new BeamFormer(nBands, frameSamples, nMics, nBeamsPerHemi);

    // preparing other memory structures
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
	outGammaToneFilteredAudioMap = new yarp::sig::Matrix(nBands*2, frameSamples);

    
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

	inPort->getEnvelope(ts);

	if (ts.getCount() != lastframe + 1) {
		
	}

	int row = 0;
	for (int col = 0 ; col < frameSamples; col++) {
		yarp::os::NetInt32 temp_c = (yarp::os::NetInt32) s->get(col, 0);
		yarp::os::NetInt32 temp_d = (yarp::os::NetInt32) s->get(col, 1);
		rawAudio[row]   = (float) temp_c / normDivid;
		rawAudio[row + 1]	= (float) temp_d / normDivid;
		//printf("%d   %f",temp_c,rawAudio[row]);
		row += 2;
	}


	//yError("Error in the loading of file");
	gammatoneAudioFilter->gammatoneFilterBank(rawAudio);
	beamForm->inputAudio(gammatoneAudioFilter->getFilteredAudio());

	sendGammatoneFilteredAudio(gammatoneAudioFilter->getFilteredAudio());
	outGammaToneFilteredAudioPort->setEnvelope(ts);
	outGammaToneFilteredAudioPort->write(*outGammaToneFilteredAudioMap);

	reducedBeamFormedAudioVector = beamForm->getReducedBeamAudio();
	beamFormedAudioVector = beamForm->getBeamAudio();
	spineInterp();


	sendAudioMap();
	outPort->setEnvelope(ts);
	outPort->write(*outAudioMap);

	//Timing how long the module took
	lastframe = ts.getCount();
	stopTime=Time::now();
	yInfo("elapsed time = %f\n",stopTime-startTime);
	startTime=stopTime;
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


void AudioPreprocesserModule::loadFile(yarp::os::ResourceFinder &rf)
{
	
	try {
		frameSamples  = rf.check("frameSamples", 
                           Value("4096"), 
                           "frame samples (int)").asInt();
        nBands  = rf.check("nBands", 
                           Value("128"), 
                           "numberBands (int)").asInt();
        nMics  = rf.check("nMics", 
                           Value("2"), 
                           "number mics (int)").asInt();
        interpellateNSamples  = rf.check("interpellateNSamples", 
                           Value("180"), 
                           "interpellate N samples (int)").asInt();
        micDistance = rf.check("micDistance", 
                           Value("0.145"), 
                           "micDistance (double)").asDouble();
       	C = rf.check("C", 
                           Value("338"), 
                           "C speed of sound (int)").asInt();
        samplingRate = rf.check("samplingRate", 
                           Value("48000"), 
                           "sampling rate of mics (int)").asInt();
        lowCf = rf.check("lowCf", 
                           Value("1000"), 
                           "lowest center frequency(int)").asInt();
		highCf = rf.check("highCf", 
                           Value("3000"), 
                           "highest center frequency(int)").asInt();
		
		yInfo("micDistance = %f", micDistance);
        nBeamsPerHemi  = (int)((micDistance / C) * samplingRate) - 1;
        yInfo("_beamsPerHemi %d = %f / %d * %d", nBeamsPerHemi, micDistance, C, samplingRate);
        totalBeams = nBeamsPerHemi * 2 + 1;
        yInfo("frameSamples = %d", frameSamples);
        yInfo("nBands = %d", nBands);
        yInfo("nMics = %d", nMics);
        yInfo("interpellateNSamples = %d", interpellateNSamples );
		yInfo("total beams = %d",totalBeams);
	}
	catch (int a) {
        yError("Error in the loading of file");
	}


}


void AudioPreprocesserModule::sendGammatoneFilteredAudio(const std::vector<float*> &gammatoneAudio){
	for (int i = 0; i < nBands; i++)
	{
		yarp::sig::Vector tempV(frameSamples);
		for (int j = 0; j < frameSamples; j++)
		{
			tempV[j] = gammatoneAudio[i][j];
		}
		outGammaToneFilteredAudioMap->setRow(i, tempV);
	}
	for (int i = 0; i < nBands; i++)
	{
		yarp::sig::Vector tempV(frameSamples);
		for (int j = 0; j < frameSamples; j++)
		{
			tempV[j] = gammatoneAudio[i+nBands][j];
		}
		outGammaToneFilteredAudioMap->setRow(i+nBands, tempV);
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
