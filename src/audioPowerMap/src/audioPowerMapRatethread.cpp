// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2018  Department of Neuroscience - University of Lethbridge
  * Author: Austin Kothig, Matt Tata
  * email: kothiga@uleth.ca matthew.tata@uleth.ca
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
 * @file audioPowerMapRatethread.cpp
 * @brief Implimentation of the power map ratethread.
 */

#include "iCub/audioPowerMapRatethread.h"

AudioPowerMapRatethread::AudioPowerMapRatethread() : RateThread(THRATE) {
    robot = "icub";
}


AudioPowerMapRatethread::AudioPowerMapRatethread(std::string _robotname, std::string _configFile, yarp::os::ResourceFinder &rf) : RateThread(THRATE) {
    robot = _robotname;
	configFile = _configFile;
	loadFile(rf);
}


AudioPowerMapRatethread::~AudioPowerMapRatethread() {
    
    delete inBandsPowerPort;
    delete inBayesMapPort;
    delete outBayesPowerPort;
    delete outBayesPowerAnglePort;
    delete outProbabilityPowerPort;
    delete outBayesProbabilityPowerPort;
    delete outBayesProbabilityPowerAnglePort;

    delete inBandsPowerMatrix;
    delete inBayesMapMatrix;
    delete outBayesPowerMatrix;
    delete outBayesPowerAngleMatrix;
    delete outProbabilityPowerMatrix;
    delete outBayesProbabilityPowerMatrix;
    delete outBayesProbabilityPowerAngleMatrix;
}


bool AudioPowerMapRatethread::threadInit() {

    //-- Allocate space for the std vectors.
    for (int band = 0; band < nBands; band++) {
        std::vector<double> tempvector;
        for (int angle = 0; angle < nAngles; angle++) { 
            tempvector.push_back(1.0);
        }

        //-- 128x360
        currentBayesMap.push_back(tempvector);
        currentBayesPowerMap.push_back(tempvector);
        currentBayesProbabilityPowerMap.push_back(tempvector);

        //-- 1x128
        currentBandPowerMap.push_back(1.0);
        currentProbabilityPowerMap.push_back(1.0);
    }

    //-- 1x360
    for (int angle = 0; angle < nAngles; angle++) {
        currentBayesPowerAngleMap.push_back(1.0);
        currentBayesProbabilityPowerAngleMap.push_back(1.0);
    }
    

    //-- Allocate memory for yarp matrix.
    inBandsPowerMatrix                  = new yarp::sig::Matrix(1, nBands);
    inBayesMapMatrix                    = new yarp::sig::Matrix(nBands, nAngles);
    outBayesPowerMatrix                 = new yarp::sig::Matrix(nBands, nAngles);
    outBayesPowerAngleMatrix            = new yarp::sig::Matrix(1, nAngles);
    outProbabilityPowerMatrix           = new yarp::sig::Matrix(1, nBands);
    outBayesProbabilityPowerMatrix      = new yarp::sig::Matrix(nBands, nAngles);
    outBayesProbabilityPowerAngleMatrix = new yarp::sig::Matrix(1, nAngles);


    //--
    //-- Initialize the Ports for data to be sent and received on.
    //--
    inBandsPowerPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    if (!inBandsPowerPort->open(getName("/BandsPower:i"))) {
        yError("unable to open port to receive input of the power map.");
        return false;
    }


    inBayesMapPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    if (!inBayesMapPort->open(getName("/BayesMap:i"))) {
        yError("unable to open port to receive input of the bayes map.");
        return false;
    }


    outBayesPowerPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    if (!outBayesPowerPort->open(getName("/BayesPower:o"))) {
        yError("unable to open port to sent output of power map.");
        return false;
    }


    outBayesPowerAnglePort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    if (!outBayesPowerAnglePort->open(getName("/BayesPowerAngle:o"))) {
        yError("unable to open port to send output angles of power.");
        return false;
    }


    outProbabilityPowerPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    if (!outProbabilityPowerPort->open(getName("/ProbabilityPower:o"))) {
        yError("unable to open port to send probability of the power of bands.");
        return false;
    }


    outBayesProbabilityPowerPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    if (!outBayesProbabilityPowerPort->open(getName("/BayesProbabilityPower:o"))) {
        yError("unable to open port to send bayes map mulitplied by the probability of band power.");
        return false;
    }


    outBayesProbabilityPowerAnglePort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    if (!outBayesProbabilityPowerAnglePort->open(getName("/BayesProbabilityPowerAngle:o"))) {
        yError("unable to open port to send angles of the bayes map mulitplied by the probability of band power.");        
        return false;
    }
 
    return true;
}


void AudioPowerMapRatethread::threadRelease() {

    //-- Stop all Ports.
    inBandsPowerPort->interrupt();
    inBayesMapPort->interrupt();
    outBayesPowerPort->interrupt();
    outBayesPowerAnglePort->interrupt();
    outProbabilityPowerPort->interrupt();
    outBayesProbabilityPowerPort->interrupt();
    outBayesProbabilityPowerAnglePort->interrupt();

    //-- Release all Ports.
    inBandsPowerPort->close();
    inBayesMapPort->close();
    outBayesPowerPort->close();
    outBayesPowerAnglePort->close();
    outProbabilityPowerPort->close();
    outBayesProbabilityPowerPort->close();
    outBayesProbabilityPowerAnglePort->close();
}


void AudioPowerMapRatethread::run() {

    //-- Read in the 1 x nBands (128) matrix from  
    //-- the audio preproccessor module. The module 
    //-- will wait to receive input.
    inBandsPowerMatrix = inBandsPowerPort->read(true);

    //-- Gather the time/counter envelope that was
    //-- associated with the last message.
    inBandsPowerPort->getEnvelope(ts);

    //-- Process the band power into the input vector.
    setInputBandPower();


    //-- Read in the nBands x (2*interpolateNSamples) (128x360) 
    //-- matrix from the audio bayesian module. The module will
    //-- wait to receive input.
    inBayesMapMatrix = inBayesMapPort->read(true);

    //-- Gather the time/counter envelope that was
    //-- associated with the last message.
    inBayesMapPort->getEnvelope(ts);

    //-- Process the bayes map into the input vector.
    setInputBayesMap();


    //-- Optional: Output the Instantaneous Power Map  
    //-- Only do work for this section if there is a connection to this port.
    if (outBayesPowerPort->getOutputCount() || outBayesPowerAnglePort->getOutputCount()) {

        //-- Multiply the instantaneous power map by the bayes map,
        //-- to find the power power of each belief.
        //-- This needs to happen before collapsing.
        combineBayesPower(currentBayesPowerMap, currentBayesMap, currentBandPowerMap);

        //-- If there is someone connect to this port, send the data.
        if (outBayesPowerPort->getOutputCount()) {
            sendBayesPower();
        }

        //-- If there is someone connect to this port, collapse
        //-- the combined bayes power map, and send the data.
        if (outBayesPowerAnglePort->getOutputCount()) {
            collapseBayesPower(currentBayesPowerAngleMap, currentBayesPowerMap);
            sendBayesPowerAngle();
        }
    }


    //-- Update the bayesian probability of power at each band.
    updatePowerProbability();

    //-- If someone is connected to this port, send data.
    if (outProbabilityPowerPort->getOutputCount()) {
        sendProbabilityPower();
    }


    if (outBayesProbabilityPowerPort->getOutputCount() || outBayesProbabilityPowerAnglePort->getOutputCount()) {
        
        //-- Combine the Bayesian Localization Map together with the Bayesian Power Map.
        combineBayesPower(currentBayesProbabilityPowerMap, currentBayesMap, currentProbabilityPowerMap);

        //-- Send data if someone is connected.
        if (outBayesProbabilityPowerPort->getOutputCount()) {
            sendBayesProbabilityPower();
        }

        //-- If someone is connected to the angle map,
        //-- collapse and send data.
        if (outBayesProbabilityPowerAnglePort->getOutputCount()) {
            collapseBayesPower(currentBayesProbabilityPowerAngleMap, currentBayesProbabilityPowerMap);
            sendBayesProbabilityPowerAngle();
        }
    }

    //-- Display the time.
	stopTime = yarp::os::Time::now();
	yInfo("Count:%d Time:%f. \n", ts.getCount(),  stopTime-startTime);
	startTime = stopTime;
}


void AudioPowerMapRatethread::setName(std::string str) {
    this->name = str;
}


std::string AudioPowerMapRatethread::getName(const char* p) {
    std::string str(name);
	str.append(p);
	return str;
}


bool AudioPowerMapRatethread::processing() {
    //-- here goes the processing . . .
    return true;
}


void AudioPowerMapRatethread::loadFile(yarp::os::ResourceFinder &rf) {
    
    // import all relevant data fron the .ini file
	yInfo("Loading Configuration File.");
	try {
		nBands               = rf.findGroup("preprocessing").check("nBands", yarp::os::Value(128), "numberBands (int)").asInt();
		interpolateNSamples  = rf.findGroup("preprocessing").check("interpolateNSamples", yarp::os::Value(180), "interpolate N samples (int)").asInt();
		
        nMics  				 = rf.findGroup("sampling").check("nMics", yarp::os::Value(2), "numberBands (int)").asInt();
        
        bufferSize           = rf.findGroup("powermap").check("bufferSize", yarp::os::Value(20), "length of buffer (int)").asInt();
		
        nAngles = interpolateNSamples * 2;

        yInfo("\t nMics               = %d", nMics);
        yInfo("\t nBands              = %d", nBands);
		yInfo("\t interpolateNSamples = %d", interpolateNSamples);
        yInfo("\t nAngles            = %d", nAngles);
        yInfo("\t bufferSize          = %d", bufferSize);
	}

	catch (int a) {
		yError("Error in the loading of file");
	}

	yInfo("file successfully load");
}


void AudioPowerMapRatethread::setInputBandPower() {

    //-- This takes the input matrix for band power and 
    //-- copies each element into the currentBandPower.
    int count = 0;
    for (int band = 0; band < nBands; band++) {
        currentBandPowerMap[band] = *(inBandsPowerMatrix->data()+(count++));
    }

    //-- Normalize the band power.
    normalizeVector(currentBandPowerMap);
}


void AudioPowerMapRatethread::setInputBayesMap() {

    //-- This takes the input matrix from the bayes module 
    //-- and copies each element into the currentBayesMap.
    int count = 0;
    for (int band = 0; band < nBands; band++) {
        for (int angle = 0; angle < nAngles; angle++) {
            currentBayesMap[band][angle] = *(inBayesMapMatrix->data()+(count++));
        }
    }
}


void AudioPowerMapRatethread::updatePowerProbability() {

    //-- TODO: add functionality for creating noise map, and removing.

    //-- Add the current band power map to the running buffer of samples.
    bufferedPowerMap.push(currentBandPowerMap);

    //--
    //-- Update the running probability of power across beams.
    //-- 
    //-- Check if the buffer size is at capacity. If so, remove
    //-- the oldest sample from the running bayesian map.
    //-- 
    if (bufferedPowerMap.size() >= bufferSize) {

        removeMap(currentProbabilityPowerMap, bufferedPowerMap.front());
        bufferedPowerMap.pop();
    }

    //-- Add the current band power map to the belief.
    addMap(currentProbabilityPowerMap, currentBandPowerMap);

    //-- Normalize the probability map.
    normalizeVector(currentProbabilityPowerMap);
}


void AudioPowerMapRatethread::removeMap(std::vector <double> &probabilityPowerMap, std::vector <double> oldPowerMap) {

    //-- Loops through the running probability map and removes the belief of the oldest power map.
    for (int band = 0; band < nBands; band++) {
        probabilityPowerMap[band] /= oldPowerMap[band];
    }
}


void AudioPowerMapRatethread::addMap(std::vector <double> &probabilityPowerMap, std::vector <double> newPowerMap) {

    //-- Loops through the running probability map and adds the belief of the newest power map.
    for (int band = 0; band < nBands; band++) {
        probabilityPowerMap[band] *= newPowerMap[band];
    }
}


void AudioPowerMapRatethread::normalizeVector(std::vector<double> &targetVector) {

    //-- Loop through the length (band or angles) and normalize each index.
    //-- This normalization is done by summing up all the elements
    //-- together and then dividing each element in the column by the sum.
    unsigned int len = targetVector.size();
    double sum       = 0.0;

    for (int idx = 0; idx < len; idx++) {
        sum += targetVector[idx];
    }

    for (int idx = 0; idx < len; idx++) {
        targetVector[idx] /= sum;
    }
}


void AudioPowerMapRatethread::combineBayesPower(std::vector < std::vector <double> > &targetMap, std::vector < std::vector <double> > &bandAngleMap, std::vector <double> &bandMap) {

    //-- Multiply the band by samples map together
    //-- and store it in the target map.
    double currentBand = 0.0;
    for (int band = 0; band < nBands; band++) {

        currentBand = bandMap[band];

        for (int angle = 0; angle < nAngles; angle++) {
            targetMap[band][angle] = bandAngleMap[band][angle] * currentBand;
        }

        //-- Normalize this row.
        normalizeVector(targetMap[band]);
    }
}


void AudioPowerMapRatethread::collapseBayesPower(std::vector <double> &targetMap, std::vector < std::vector <double> > &sourceMap) {

    //-- Clear out the angle map.
    for (int angle = 0; angle < nAngles; angle++) {
        targetMap[angle] = 0.0;
    }

    //-- Sum up the power of each band for all samples.
    for (int band = 0; band < nBands; band++) {
        for (int angle = 0; angle < nAngles; angle++) {
            targetMap[angle] += sourceMap[band][angle];
        }
    }

    //-- Normalize the target.
    normalizeVector(targetMap);
}


void AudioPowerMapRatethread::sendBayesPower() {

    yarp::sig::Matrix &m = outBayesPowerPort->prepare();
    m.resize(nBands, nAngles);

    for (int band = 0; band < nBands; band++) {

        yarp::sig::Vector tempvector(nAngles);

        for (int angle = 0; angle < nAngles; angle++) {
            tempvector[angle] = currentBayesPowerMap[band][angle];
        }

        m.setRow(band, tempvector);
    }

    outBayesPowerPort->setEnvelope(ts);
    outBayesPowerPort->write();
}


void AudioPowerMapRatethread::sendBayesPowerAngle() {

    yarp::sig::Matrix &m = outBayesPowerAnglePort->prepare();
    m.resize(1, nAngles);

    yarp::sig::Vector tempvector(nAngles);

    for (int angle = 0; angle < nAngles; angle++) {
        tempvector[angle] = currentBayesPowerAngleMap[angle];
    }

    m.setRow(0, tempvector);

    outBayesPowerAnglePort->setEnvelope(ts);
    outBayesPowerAnglePort->write();
}


void AudioPowerMapRatethread::sendProbabilityPower() {

    yarp::sig::Matrix &m = outProbabilityPowerPort->prepare();
    m.resize(1, nBands);

    yarp::sig::Vector tempvector(nBands);

    for (int band = 0; band < nBands; band++) {
        tempvector[band] = currentProbabilityPowerMap[band];
    }

    m.setRow(0, tempvector);

    outProbabilityPowerPort->setEnvelope(ts);
    outProbabilityPowerPort->write();
}


void AudioPowerMapRatethread::sendBayesProbabilityPower() {

    yarp::sig::Matrix &m = outBayesProbabilityPowerPort->prepare();
    m.resize(nBands, nAngles);

    for (int band = 0; band < nBands; band++) {

        yarp::sig::Vector tempvector(nAngles);

        for (int angle = 0; angle < nAngles; angle++) {
            tempvector[angle] = currentBayesProbabilityPowerMap[band][angle];
        }

        m.setRow(band, tempvector);
    }

    outBayesProbabilityPowerPort->setEnvelope(ts);
    outBayesProbabilityPowerPort->write();
}


void AudioPowerMapRatethread::sendBayesProbabilityPowerAngle() {

    yarp::sig::Matrix &m = outBayesProbabilityPowerAnglePort->prepare();
    m.resize(1, nAngles);

    yarp::sig::Vector tempvector(nAngles);

    for (int angle = 0; angle < nAngles; angle++) {
        tempvector[angle] = currentBayesProbabilityPowerAngleMap[angle];
    }

    m.setRow(0, tempvector);

    outBayesProbabilityPowerAnglePort->setEnvelope(ts);
    outBayesProbabilityPowerAnglePort->write();
}

