// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include "beamFormer.h"
#include <iostream>
#include <iomanip>
#include <time.h>

int myMod(int a, int b) {
	return  a>=0 ? a % b : (a%b)+b;
}
BeamFormer::BeamFormer()
{
	fileName = "../src/YarpPreprocessing/src/loadFile.xml";
	loadFile();

	for (int i = 0; i < (getNBeamsPerHemifield * 2) + 1; i++)
	{
		std::vector<float*> tempvector;
		for (int j = 0; j < nBands; j++)
		{
			float* temparray = new float[frameSamples];
			tempvector.push_back(temparray);
		}
		beamFormedAudioVector.push_back(tempvector);
	}

	for (int i = 0; i < (getNBeamsPerHemifield * 2) + 1; i++)
	{
		std::vector<double> tempvector;
		for (int j = 0; j < nBands; j++)
		{
			double tempnumber = 0;
			tempvector.push_back(tempnumber);
		}
		reducedBeamFormedAudioVector.push_back(tempvector);
	}
	myfile.open ("output.csv");

}

BeamFormer::~BeamFormer()
{


}

void BeamFormer::inputAudio(std::vector< float* > inAudio)
{

	clearVectors();
	inputSignal = inAudio;


}

std::vector<std::vector<float*> > BeamFormer::getBeamAudio()
{
	for (int i = getNBeamsPerHemifield; i > -getNBeamsPerHemifield; i--)
	{
		for (int j = 0; j < nBands; j++)
		{
			for (int k = 0; k < frameSamples; k++)
			{
				beamFormedAudioVector[i][j][k] = inputSignal[j][k] + inputSignal[j + nBands][(k+i) % frameSamples];
			}
		}
	}

	return beamFormedAudioVector;
}

void BeamFormer::multiThreader(int i){

		for (int j = 0; j < nBands; j++)
		{
			for (int k = 0; k < frameSamples; k++)
			{

					reducedBeamFormedAudioVector[i][j] += pow((inputSignal[j][k] + inputSignal[j+nBands][myMod(k+(18-i),frameSamples)]),2);

				}
			reducedBeamFormedAudioVector[i][j] = sqrt((static_cast<double>(1)/frameSamples) * (reducedBeamFormedAudioVector[i][j]));

		}



}

std::vector<std::vector<double> > BeamFormer::getReducedBeamAudio()
{
	std::vector<std::thread> myThread;
	for (int i = 0; i < (getNBeamsPerHemifield * 2) + 1; i++)
	{
			myThread.push_back(std::thread(&BeamFormer::multiThreader,this,i));

	}
	auto othread = myThread.begin();
	while(othread != myThread.end())
	{
		othread->join();
		othread++;
	}


	return reducedBeamFormedAudioVector;
}

void BeamFormer::loadFile()
{

	ConfigParser *confPars;
	confPars = ConfigParser::getInstance(fileName);
	Config pars = (confPars->getConfig("default"));

	nBands = pars.getNBands();
	frameSamples = pars.getFrameSamples();
	nMics = pars.getNMics();
	getNBeamsPerHemifield = 19;


}

void BeamFormer::clearVectors()
{


}
