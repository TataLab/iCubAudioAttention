/**
 * This class holds configuration options that can be changed using the XML
 * parser class and called using the configuration name.
 */

// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include "Config.h"
#include <math.h>

Config::Config()
{

}
Config::~Config()
{

}

int Config::getInterpellateNSamples()
{
	return interpellateNSamples;
}

std::string Config::getName()
{
	return name;
}
double Config::getC()
{
	return C;
}
double Config::getMicDistance()
{
	return micDistance;
}
int Config::getSamplingRate()
{

	return samplingRate;
}
int Config::getNMics()
{
	return nMics;
}
int Config::getNBands()
{
	return nBands;
}
int Config::getFrameDurationSamples()
{
	return frameDurationSamples;
}
int Config::getFrameDurationSeconds()
{
	return frameDurationSeconds;
}
int Config::getRequiredLagFrames()
{
	return requiredLagFrames;
}
int Config::getNumFramesInBuffer()
{
	return numFramesInBuffer;
}
int Config::getNBeamsPerHemifield()
{
	return nBeamsPerHemifield;
}
int Config::getNBeams()
{
	return nBeams;
}
int Config::getLags()
{
	return lags;
}
double Config::getAngles()
{
	return angles;
}
int Config::getLowCf()
{
	return lowCf;
}
int Config::getHighCf()
{
	return highCf;
}
int Config::getFrameOverlap()
{
	return frameOverlap;
}
int Config::getFramePlusOverlap()
{
	return framePlusOverlap;
}
int Config::getFrameIndicies()
{
	return frameIndices;
}
int Config::getNPastSeconds()
{
	return nPastSeconds;
}
int Config::getNPastFrames()
{
	return nPastFrames;
}
int Config::getAttentionCaptureThreshold()
{
	return attentionCaptureThreshold;
}
int Config::getInhibitionOfCapture()
{
	return inhibitionOfCapture;
}
double Config::getRadialResolutionDegrees()
{
	return radialResolutionDegrees;
}
double Config::getRadialResolutionRadians()
{
	return radialResolutionRadians;
}
int Config::getNumSpaceAngles()
{
	return numSpaceAngles;
}
int Config::getFrameSamples()
{
	return frameSamples;
}
bool Config::getPhaseAlign()
{
	return phaseAlign;
}
int Config::getShortBufferSize()
{
	return shortBufferSize;
}
int Config::getMediumBufferSize()
{
	return mediumBufferSize;
}
int Config::getLongBufferSize()
{
	return longBufferSize;
}
bool Config::getEnableMultiScale(){
	return enableMultiScale;
}
void Config::setPhaseAlign(std::string pa)
{
	for (int i = 0; i < pa.length(); i++)
	{
		pa[i] = std::toupper(pa[i]);

	}
	if (pa == "TRUE" || pa == "YES") phaseAlign = true;
	else phaseAlign = false;

}

void Config::setName(std::string n)
{
	name = n;
}
void Config::setC(double c)
{
	C = c;
}
void Config::setMicDistance(double md )
{
	micDistance = md;
}
void Config::setSamplingRate(int sr)
{
	std::cout << "Sampling Rate set to " << sr << std::endl;
	samplingRate = sr;
}
void Config::setNMics(int snm)
{
	nMics = snm;
}
void Config::setNBands(int nb)
{
	nBands = nb;
}
void Config::setNBeamsPerHemifield()
{
	nBeamsPerHemifield = (int)((micDistance / C) * samplingRate) - 1;
}

void Config::setFrameDurationSamples(int fds)
{
	//TODO FIX ME!
	//could be calculated so is set to private
	frameDurationSamples = fds;
}

void Config::setFrameDurationSeconds()
{
	frameDurationSamples = frameDurationSamples / samplingRate;
}

void Config::setRequiredLagFrames(int rlf)
{
	requiredLagFrames = rlf;
}

void Config::setNumFramesInBuffer(int nfib)
{
	numFramesInBuffer = nfib;
}

void Config::setNBeams()
{
	nBeams = (2 * nBeamsPerHemifield) + 1;
	std::cout << nBeams << std::endl;
}

void Config::setLags()
{
	//TODO Decode this..
	//P.lags=(P.c/P.sampleRate).* (-P.nBeamsPerHemifield:P.nBeamsPerHemifield); %linear spaced distances corresponding to lags in seconds
}

void Config::setAngles()
{
	//real(asin( (1/P.D) .* P.lags )) ; %nonlinear angles (in radians) that correspond to the lags
	//angles = asin(1/micDistance)
}

void Config::setLowCf(int lcf)
{
	lowCf = lcf;
}

void Config::setHighCf(int hcf)
{
	highCf = hcf;
}

void Config::setFrameOverlap(int fo)
{
	frameOverlap = fo;
}

void Config::setframePlusOverlap()
{
	framePlusOverlap = frameDurationSamples + (frameOverlap * 2);
}

void Config::setFrameIndicies()
{
	// /P.frameIndices=P.frameOverlap+1:(P.frameOverlap+1) + P.frameDuration_samples - 1; %the indices of the "core" frame inside the grabbed audio data
}

void Config::setNPastSeconds(int nps)
{
	nPastSeconds = nps;
}

void Config::setNPastFrames()
{
	nPastFrames = floor(nPastSeconds / frameDurationSeconds);
}

void Config::setAttentionCaptureThreshold(int act)
{
	attentionCaptureThreshold = act;
}

void Config::setInhibitionOfCapture()
{
	inhibitionOfCapture = 10 * frameDurationSeconds;
}
void Config::setRadialResolutionDegrees(double rd)
{
	radialResolutionDegrees = rd;
}
void Config::setRadialResolutionRadians()
{
	radialResolutionRadians = 	3.14159265358979323846 / 180 * radialResolutionDegrees;
}

void Config::setNumSpaceAngles()
{
	numSpaceAngles = 360 / radialResolutionDegrees;
}

void Config::setFrameSamples(int fs)
{
	frameSamples = fs;
}

void Config::setShortBufferSize(int shortBSize)
{
	shortBufferSize = shortBSize;
}

void Config::setMediumBufferSize(int mediumBSize)
{
	mediumBufferSize = mediumBSize;
}

void Config::setLongBufferSize(int longBSize)
{
	longBufferSize = longBSize;
}
void Config::setEnableMultiScale(std::string pa)
{
	for (int i = 0; i < pa.length(); i++)
	{
		pa[i] = std::toupper(pa[i]);

	}
	if (pa == "TRUE" || pa == "YES") enableMultiScale = true;
	else enableMultiScale = false;
}
void Config::calculate()
{

	setNBeamsPerHemifield();
	setNBeams();
	/*
	setFrameDurationSamples(0);
	setFrameDurationSeconds();
	
	setLags();
	setAngles();
	setframePlusOverlap();
	setFrameIndicies();
	setNPastFrames();
	setInhibitionOfCapture();
	setRadialResolutionRadians();
	setNumSpaceAngles();
	*/
}

void Config::setInterpellateNSamples(int ins)
{
	interpellateNSamples = ins;
}

void Config::printVariables()
{
	std::cout << "This Configureation is set as such:" << std::endl;
	std::cout << "-Name: " << name << std::endl;
	std::cout << "---C: " << C << std::endl;
	std::cout << "---micDistance: " << micDistance << std::endl;
	std::cout << "---samplingRate: " << samplingRate << std::endl;
	std::cout << "---nMics: " << nMics << std::endl;
	std::cout << "---nBands: " << nBands << std::endl;
	std::cout << "---frameSamples: " << frameSamples << std::endl;
	std::cout << "---frameDurationSamples: " << frameDurationSamples << std::endl;
	std::cout << "---frameDurationSeconds: " << frameDurationSeconds << std::endl;
	std::cout << "---requiredLagFrames: " << requiredLagFrames << std::endl;
	std::cout << "---numFramesInBuffer: " << numFramesInBuffer << std::endl;
	std::cout << "---nBeamsPerHemifield: " << nBeamsPerHemifield << std::endl;
	std::cout << "---nBeams: " << nBeams << std::endl;
	std::cout << "---lags: " << lags << std::endl;
	std::cout << "---angles: " << angles << std::endl;
	std::cout << "---lowCf: " << lowCf << std::endl;
	std::cout << "---highCf: " << highCf << std::endl;
	std::cout << "---frameOverlap: " << frameOverlap << std::endl;
	std::cout << "---framePlusOverlap: " << framePlusOverlap << std::endl;
	std::cout << "---frameIndices: " << frameIndices << std::endl;
	std::cout << "---nPastSeconds: " << nPastSeconds << std::endl;
	std::cout << "---nPastFrames: " << nPastFrames << std::endl;
	std::cout << "---attentionCaptureThreshold: " << attentionCaptureThreshold << std::endl;
	std::cout << "---inhibitionOfCapture: " << inhibitionOfCapture << std::endl;
	std::cout << "---radialResolutionDegrees: " << radialResolutionDegrees << std::endl;
	std::cout << "---radialResolutionRadians: " << radialResolutionRadians << std::endl;
	std::cout << "---numSpaceAngles: " << numSpaceAngles << std::endl;

}
