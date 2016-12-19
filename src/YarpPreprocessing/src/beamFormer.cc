// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include "beamFormer.h"

int myMod(int a, int b) {
	return  a >= 0 ? a % b : (a % b) + b;
}
BeamFormer::BeamFormer(std::string file)
{

	fileName = file;
	loadFile();

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

	for (int i = 0; i < totalBeams; i++)
	{
		for (int j = 0; j < nBands; j++)
		{
			delete beamFormedAudioVector[i][j];
		}
	}
}

void BeamFormer::inputAudio(std::vector< float* > inAudio)
{
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
			beamFormedAudioVector[i][j][k] = inputSignal[j][k] + inputSignal[j + nBands][myMod(k + (18 - i), frameSamples)];
		}
	}

}
void BeamFormer::reducedAudioMultiThreadingLoop(int i) {

	for (int j = 0; j < nBands; j++)
	{
		for (int k = 0; k < frameSamples; k++)
		{
			reducedBeamFormedAudioVector[i][j] += pow((inputSignal[j][k] + inputSignal[j + nBands][myMod(k + (18 - i), frameSamples)]), 2);
		}
		reducedBeamFormedAudioVector[i][j] = sqrt((static_cast<double>(1) / frameSamples) * (reducedBeamFormedAudioVector[i][j]));
	}

}

void BeamFormer::loadFile()
{

	ConfigParser *confPars;
	try {
		confPars = ConfigParser::getInstance(fileName);
		Config pars = (confPars->getConfig("default"));
		nBands = pars.getNBands();
		frameSamples = pars.getFrameSamples();
		nMics = pars.getNMics();
		getNBeamsPerHemifield = 19;
		totalBeams = 39;
	}
	catch (int a) {

		
	}

}
