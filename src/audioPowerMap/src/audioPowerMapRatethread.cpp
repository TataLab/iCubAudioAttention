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
        for (int samp = 0; samp < totalSamples; samp++) {
            tempvector.push_back(0.0);
        }

        currentBayesMap.push_back(tempvector);
        currentBayesPowerMap.push_back(tempvector);

        currentBandPowerMap.push_back(0.0);
        currentBayesPowerAngleMap.push_back(0.0);
    }

    //-- Allocate memory for yarp matrix.
    inBandsPowerMatrix       = new yarp::sig::Matrix(1, nBands);
    inBayesMapMatrix         = new yarp::sig::Matrix(nBands, totalSamples);
    outBayesPowerMatrix      = new yarp::sig::Matrix(nBands, totalSamples);
    outBayesPowerAngleMatrix = new yarp::sig::Matrix(1, totalSamples);

    // TODO: 
    outProbabilityPowerMatrix           = new yarp::sig::Matrix(1, nBands);
    outBayesProbabilityPowerMatrix      = new yarp::sig::Matrix(nBands, totalSamples);
    outBayesProbabilityPowerAngleMatrix = new yarp::sig::Matrix(1, totalSamples);


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


    // TODO:
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


    //-- Only do work for this section if there is a connection to this port.
    if (outBayesPowerPort->getOutputCount() || outBayesPowerAnglePort->getOutputCount()) {

        //-- Multiply the instantaneous power map by the bayes map,
        //-- to find the power power of each belief.
        //-- This needs to happen before collapsing.
        combineBayesPower();

        //-- If there is someone connect to this port, send the data.
        if (outBayesPowerPort->getOutputCount()) {
            sendBayesPower();
        }

        //-- If there is someone connect to this port, collapse
        //-- the combined bayes power map, and send the data.
        if (outBayesPowerAnglePort->getOutputCount()) {
            collapseBayesPower();
            sendBayesPowerAngle();
        }
    }


    //-- TODO: The bayesian approach of power.


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
	yInfo("loading configuration file");
	try {
		nBands               = rf.check("nBands", yarp::os::Value(128), "numberBands (int)").asInt();
		interpolateNSamples  = rf.check("interpolateNSamples", yarp::os::Value(180), "interpellate N samples (int)").asInt();
		nMics  				 = rf.check("nMics", yarp::os::Value(2), "numberBands (int)").asInt();
		
        totalSamples = interpolateNSamples * 2;

        yInfo("nBands = %d", nBands);
		yInfo("nMics = %d", nMics);
		yInfo("interpolateNSamples = %d", interpolateNSamples);
        yInfo("totalSamples = %d", totalSamples);
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
    normalizeBandPower();
}


void AudioPowerMapRatethread::setInputBayesMap() {

    //-- This takes the input matrix from the bayes module 
    //-- and copies each element into the currentBayesMap.
    int count = 0;
    for (int band = 0; band < nBands; band++) {
        for (int samp = 0; samp < totalSamples; samp++) {
            currentBayesMap[band][samp] = *(inBayesMapMatrix->data()+(count++));
        }
    }
}


void AudioPowerMapRatethread::normalizeBandPower() {

    //-- Loop through the band power input and normalize each index.
    //-- This normalization is done by summing up all the elements
    //-- together and then dividing each element in the column by the sum.
    double sum = 0.0;
    for (int band = 0; band < nBands; band++) {
        sum += currentBandPowerMap[band];
    }

    for (int band = 0; band < nBands; band++) {
        currentBandPowerMap[band] /= sum;
    }
}


void AudioPowerMapRatethread::combineBayesPower() {

    //-- Multiply the normalized 

    for (int band = 0; band < nBands; band++) {

        double currentBand = currentBandPowerMap[band];

        for (int samp = 0; samp < totalSamples; samp++) {
            currentBayesPowerMap[band][samp] = currentBayesMap[band][samp] * currentBand;
        }
    }
}


void AudioPowerMapRatethread::collapseBayesPower() {

    //-- Clear out the angle map.
    for (int samp = 0; samp < totalSamples; samp++) {
        currentBayesPowerAngleMap[samp] = 0.0;
    }

    //-- Sum up the power of each band for all samples.
    for (int band = 0; band < nBands; band++) {
        for (int samp = 0; samp < totalSamples; samp++) {
            currentBayesPowerAngleMap[samp] += currentBayesPowerMap[band][samp];
        }
    }

    //-- Normalize the values of the angle map.
    double sum = 0.0;
    for (int samp = 0; samp < totalSamples; samp++) {
        sum += currentBayesPowerAngleMap[samp];
    }

    for (int samp = 0; samp < totalSamples; samp++) {
        currentBayesPowerAngleMap[samp] /= sum;
    }
}


//--
//-- TODO: other methods for bayes-power
//--


void AudioPowerMapRatethread::sendBayesPower() {

    yarp::sig::Matrix &m = outBayesPowerPort->prepare();
    m.resize(nBands, totalSamples);

    for (int band = 0; band < nBands; band++) {

        yarp::sig::Vector tempvector(totalSamples);

        for (int samp = 0; samp < totalSamples; samp++) {
            tempvector[samp] = currentBayesPowerMap[band][samp];
        }

        m.setRow(band, tempvector);
    }

    outBayesPowerPort->setEnvelope(ts);
    outBayesPowerPort->write();
}


void AudioPowerMapRatethread::sendBayesPowerAngle() {

    yarp::sig::Matrix &m = outBayesPowerAnglePort->prepare();
    m.resize(0, totalSamples);

    yarp::sig::Vector tempvector(totalSamples);

    for (int samp = 0; samp < totalSamples; samp++) {
        tempvector[samp] = currentBayesPowerAngleMap[samp];
    }

    m.setRow(0, tempvector);

    outBayesPowerAnglePort->setEnvelope(ts);
    outBayesPowerAnglePort->write();
}