// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef _BEAM_FORMER_H_
#define _BEAM_FORMER_H_

#include "gammatonFilter.h"
#include "../../Configuration/ConfigParser.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>

class BeamFormer {
public:
	BeamFormer(std::string file);
	~BeamFormer();

	void inputAudio(std::vector< float* > inAudio);

	//TODO Should theis be static arraies since we know the size probably
	std::vector<std::vector<float*> > getBeamAudio();
	std::vector<std::vector<double> > getReducedBeamAudio();
private:

	/**
	*	loadFile
	*	Accesses the loadFile.xml that is found in the root directory of this
	*	module and load all required parameters for the beam former.
	*/
	void loadFile();
	void reducedAudioMultiThreadingLoop(int i);
	void audioMultiThreadingLoop(int i);

	std::vector< float* > inputSignal;

	std::vector < std::vector < float* > > beamFormedAudioVector;
	std::vector < std::vector < double > > reducedBeamFormedAudioVector;
	float ** beamFormedAudio;
	int nMics;
	int frameSamples;
	int nBands;

	std::string fileName;
	int getNBeamsPerHemifield;
	int totalBeams;

	std::ofstream myfile;
};

#endif
