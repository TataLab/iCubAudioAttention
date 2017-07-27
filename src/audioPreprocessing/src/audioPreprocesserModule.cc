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
	inPort->open("/iCubAudioAttention/AudioPreprocesser:i");

	outAudioMapEgoPort = new yarp::os::Port();
	outAudioMapEgoPort->open("/iCubAudioAttention/AudioMapEgo:o");

	outGammaToneAudioPort = new yarp::os::Port();
	outGammaToneAudioPort->open("/iCubAudioAttention/GammaToneFilteredAudio:o");

	outBeamFormedAudioPort= new yarp::os::Port();
	outBeamFormedAudioPort->open("/iCubAudioAttention/BeamFormedAudio:o");



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
    //calls the parser and the config file to configure the needed variables in this class
	loadFile(rf);
        
    //preparing GammatoneFilter and beamForming
	gammatoneAudioFilter = new GammatoneFilter(samplingRate, lowCf, highCf, nBands, frameSamples, nMics, false, false);
	beamForm = new BeamFormer(nBands, frameSamples, nMics, nBeamsPerHemi);

    // preparing other memory structures
    rawAudio = new float[(frameSamples * nMics)];

	for (int i = 0; i < interpolateNSamples * 2; i++) {
		std::vector<double> tempvector;
		for (int j = 0; j < nBands; j++) {
			tempvector.push_back(0);
		}
		highResolutionAudioMap.push_back(tempvector);
	}

	outAudioMap = new yarp::sig::Matrix(nBands, interpolateNSamples * 2);
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

	for (int col = 0 ; col < frameSamples; col++) {
		for(int micLoop = 0; micLoop < nMics; micLoop++)
			rawAudio[col*nMics + micLoop] = s->get(col, micLoop) / normDivid;
	}

	gammatoneAudioFilter->gammatoneFilterBank(rawAudio);
	//if (outGammaToneAudioPort->getOutputCount()) {
	//	sendGammatoneFilteredAudio(gammatoneAudioFilter->getFilteredAudio());
	//	outGammaToneAudioPort->setEnvelope(ts);
	//	outGammaToneAudioPort->write(*outGammaToneFilteredAudioMap,false);
	//}

	beamForm->inputAudio(gammatoneAudioFilter->getFilteredAudio());
	reducedBeamFormedAudioVector = beamForm->getReducedBeamAudio();
	//if (outGammaToneAudioPort->getOutputCount()) {
	//	sendGammatoneFilteredAudio(gammatoneAudioFilter->getFilteredAudio());
	//	outGammaToneAudioPort->setEnvelope(ts);
	//	outGammaToneAudioPort->write(*outGammaToneFilteredAudioMap,false);
	//}
	beamFormedAudioVector = beamForm->getBeamAudio();
	//if (outBeamFormedAudioPort->getOutputCount()) {
	//	sendBeamFormedAudio(beamFormedAudioVector);
	//	outBeamFormedAudioPort->setEnvelope(ts);
	//	outBeamFormedAudioPort->write(*outGammaToneFilteredAudioMap,false);
	//}
	linerInterpolate();

	//if (outAudioMapEgoPort->getOutputCount()) {
		sendAudioMap();
		outAudioMapEgoPort->setEnvelope(ts);
		outAudioMapEgoPort->write(*outAudioMap);
	//}

	//Timing how long the module took
	lastframe = ts.getCount();
	stopTime=Time::now();
	yInfo("elapsed time = %f\n",stopTime-startTime);
	startTime=stopTime;
	return true;
}

bool AudioPreprocesserModule::interruptModule()
{
	yInfo("Interrupting\n");
	return true;
}

bool AudioPreprocesserModule::close()
{
	yInfo("Calling close\n");
	return true;
}

void AudioPreprocesserModule::loadFile(yarp::os::ResourceFinder &rf)
{
	yInfo("loading configuration file");
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
        interpolateNSamples  = rf.check("interpolateNSamples", 
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
        yInfo("interpolateNSamples = %d", interpolateNSamples );
		yInfo("total beams = %d",totalBeams);
	}
	catch (int a) {
        yError("Error in the loading of file");
	}
	yInfo("file successfully load");

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
		yarp::sig::Vector tempV(interpolateNSamples * 2);
		for (int j = 0; j < interpolateNSamples * 2; j++)
		{
			tempV[j] = highResolutionAudioMap[j][i];
		}
		outAudioMap->setRow(i, tempV);
	}

}

inline double AudioPreprocesserModule::linerApproximation(int x, int x1, int x2, double y1, double y2)
{
	return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
}

void AudioPreprocesserModule::linerInterpolate()
{
	double offset = (interpolateNSamples / (double)totalBeams);
	for (int i = 0; i < nBands; i++)
	{
		int k = 0;
		double curroffset = 0;
		for (int j = 0; j < interpolateNSamples; j++)
		{
			if (j == (int)curroffset && k < (totalBeams - 1))
			{
				curroffset += offset;
				k++;
			}
			highResolutionAudioMap[j][i] = linerApproximation(j, curroffset - offset, curroffset , reducedBeamFormedAudioVector[k - 1][i], reducedBeamFormedAudioVector[k][i]);
		}
		curroffset = interpolateNSamples;
		for (int j = interpolateNSamples; j < interpolateNSamples * 2; j++)
		{
			if (j == (int)curroffset && k > 1)
			{
				if (j != interpolateNSamples)
					k--;
				curroffset += offset;
			}
			highResolutionAudioMap[j][i] = linerApproximation(j, curroffset - offset, curroffset, reducedBeamFormedAudioVector[k][i], reducedBeamFormedAudioVector[k - 1][i]);
		}
	}

}

double AudioPreprocesserModule::splineApproximation(double x, double x1, double y1, double x2, double y2, double x3, double y3)
{
	knotValues k = calcSplineKnots(x1, y1, x2, y2, x3, y3);

   // determine which side
   // of the center x is
   double xs1, xs2, ys1, ys2, ks1, ks2;

   if (x > x2) {
      xs1 = x2; ys1 = y2; ks1 = k.k1;
      xs2 = x3; ys2 = y3; ks2 = k.k2;
   }

   else        {
      xs1 = x1; ys1 = y1; ks1 = k.k0;
      xs2 = x2; ys2 = y2; ks2 = k.k1;  
   }

   // calculate the value of the y
   double t = (x - xs1) / (xs2 - xs1);
   double a =  ks1 * (xs2 - xs1) - (ys2 - ys1);
   double b = -ks2 * (xs2 - xs1) + (ys2 - ys1);
   double y = (1 - t) * ys1 + t * ys2 + t * (1 - t) * (a * (1 - t) + b * t);
   return y;
}

knotValues AudioPreprocesserModule::calcSplineKnots(double x1, double y1, double x2, double y2, double x3, double y3)
{
	int matSize = 3;

   yarp::sig::Matrix a(matSize,matSize), aI(matSize,matSize);
   yarp::sig::Vector b(matSize);
   /*double b[matSize];*/
   
   // get the matrix a
   a[0][0] = 2 / (x2 - x1);
   a[0][1] = 1 / (x2 - x1);
   a[0][2] = 0;
   a[1][0] = 1 / (x2 - x1);
   a[1][1] = 2 * ( (1 / (x2 - x1)) + (1 / (x3 - x2)) );
   a[1][2] = 1 / (x3 - x2);
   a[2][0] = 0;
   a[2][1] = 1 / (x3 - x2);
   a[2][2] = 2 / (x3 - x2);

   aI = yarp::math::luinv(a);

   // get matrix b
   b[0] = 3 * ( (y2 - y1) / ( (x2 - x1) * (x2 - x1) ) );
   b[1] = 3 * ( (y2 - y1) / ( (x2 - x1) * (x2 - x1) ) 
              + (y3 - y2) / ( (x3 - x2) * (x3 - x2) ) );
   b[2] = 3 * ( (y3 - y2) / ( (x3 - x2) * (x3 - x2) ) );

   // matrix ks being the knot values
   // for the spline which is aI * b
   knotValues knots;
   knots.k0 = ( aI[0][0] * b[0] ) + ( aI[0][1] * b[1] ) + ( aI[0][2] * b[2] );
   knots.k1 = ( aI[1][0] * b[0] ) + ( aI[1][1] * b[1] ) + ( aI[1][2] * b[2] );
   knots.k2 = ( aI[2][0] * b[0] ) + ( aI[2][1] * b[1] ) + ( aI[2][2] * b[2] );
   
   return knots;
}

void AudioPreprocesserModule::splineInterpolate()
{
	double offset = (interpolateNSamples / (double)totalBeams);
	for (int i = 0; i < nBands; i++)
	{
		int k = 0;
		double curroffset = 0;
		for (int j = 0; j < interpolateNSamples; j++)
		{
			if (j == (int)curroffset && k < (totalBeams - 1))
			{
				curroffset += offset;
				k++;
			}
			highResolutionAudioMap[j][i] = splineApproximation(j, curroffset - offset, reducedBeamFormedAudioVector[k - 1][i], curroffset , reducedBeamFormedAudioVector[k][i], curroffset + offset , reducedBeamFormedAudioVector[k+1][i]);
		}
		curroffset = interpolateNSamples;
		for (int j = interpolateNSamples; j < interpolateNSamples * 2; j++)
		{
			if (j == (int)curroffset && k > 1)
			{
				if (j != interpolateNSamples)
					k--;
				curroffset += offset;
			}
			highResolutionAudioMap[j][i] = splineApproximation(j, curroffset - offset, reducedBeamFormedAudioVector[k][i], curroffset, reducedBeamFormedAudioVector[k - 1][i], curroffset + offset , reducedBeamFormedAudioVector[k-2][i]);
		}
	}

}



