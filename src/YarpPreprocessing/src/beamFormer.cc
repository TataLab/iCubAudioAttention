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

#include "beamFormer.h"
#include <iostream>
int myMod(int a, int b) {
	return  a >= 0 ? a % b : (a % b) + b;
}
BeamFormer::BeamFormer(int numBands, int nSamples, int numMics, int numBeamsHemifield):
nMics(numMics), frameSamples(nSamples), nBands(numBands), getNBeamsPerHemifield(numBeamsHemifield) 
{
	totalBeams = (getNBeamsPerHemifield*2) + 1;
	
	//Allocating the required vectors to create the beamFormed audio 
	for (int i = 0; i < totalBeams; i++)
	{
		std::vector<float*> tempvector;
		for (int j = 0; j < nBands; j++)
		{
			float* temparray = new float[frameSamples];
			tempvector.push_back(temparray);
		}
		beamFormedAudioVector.push_back(tempvector);
	}
	//Allocating the required vectors to create the reduced beamFormed audio 
	for (int i = 0; i < totalBeams; i++)
	{
		std::vector<double> tempvector;
		for (int j = 0; j < nBands; j++)
		{
			double tempnumber = 0;
			tempvector.push_back(tempnumber);
		}
		reducedBeamFormedAudioVector.push_back(tempvector);
	}

}

BeamFormer::~BeamFormer()
{
	for(int i = 0; i < inputSignal.size();i++){
		delete[] inputSignal[i];
	}
	for(int i = 0; i < beamFormedAudioVector.size();i++){
		for(int j = 0; j < beamFormedAudioVector[i].size();j++)
		delete[] beamFormedAudioVector[i][j];
	}
}

void BeamFormer::inputAudio(std::vector< float* > inAudio)
{
	//Sets the input audio passed into this function as inAudio.
	inputSignal = inAudio;
}

std::vector<std::vector<float*> > BeamFormer::getBeamAudio()
{
	std::vector<std::thread> myThread;
	//Starting all the threads
	for (int i = 0; i < totalBeams; i++)
	{
		myThread.push_back(std::thread(&BeamFormer::audioMultiThreadingLoop, this, i));

	}
	auto othread = myThread.begin();
	//TODO This is not a good way of joining (this method is far too slow)
	while (othread != myThread.end())
	{
		othread->join();
		othread++;
	}

	return beamFormedAudioVector;
}

std::vector<std::vector<double> > BeamFormer::getReducedBeamAudio()
{
	std::vector<std::thread> myThread;
	//Starting all the threads
	for (int i = 0; i < totalBeams; i++)
	{
		myThread.push_back(std::thread(&BeamFormer::reducedAudioMultiThreadingLoop, this, i));

	}
	auto othread = myThread.begin();
	//TODO This is not a good way of joining (this method is far too slow)
	while (othread != myThread.end())
	{
		othread->join();
		othread++;
	}

	return reducedBeamFormedAudioVector;
}

void BeamFormer::audioMultiThreadingLoop(int i) {


	for (int j = 0; j < nBands; j++)
	{
		for (int k = 0; k < frameSamples; k++)
		{
			beamFormedAudioVector[i][j][k] = inputSignal[j][k] + inputSignal[j + nBands][myMod(k + ((getNBeamsPerHemifield-1) - i), frameSamples)];
		}
	}

}
void BeamFormer::reducedAudioMultiThreadingLoop(int i) {

	for (int j = 0; j < nBands; j++)
	{
		for (int k = 0; k < frameSamples; k++)
		{
			reducedBeamFormedAudioVector[i][j] += pow((inputSignal[j][k] + inputSignal[j + nBands][myMod(k + ((getNBeamsPerHemifield-1) - i), frameSamples)]), 2);
		}
		reducedBeamFormedAudioVector[i][j] = sqrt((static_cast<double>(1) / frameSamples) * (reducedBeamFormedAudioVector[i][j]));
	}

}
