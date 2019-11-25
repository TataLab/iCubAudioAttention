// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Department of Neuroscience - University of Lethbridge
 * Author: Lukas Grasse, Austin Kothig, Francesco Rea, Marko Ilievski, Matt Tata
 * email: lukas.grasse@uleth.ca, kothiga@uleth.ca, francesco.reak@iit.it, marko.ilievski@uwaterloo.ca, matthew.tata@uleth.ca
 * 
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

/* ===========================================================================
 * @file  rosAudioRemapperPeriodicThread.cpp
 * @brief Implementation of the rosAudioRemapperPeriodicThread (see header file).
 * =========================================================================== */

#include <iCub/rosAudioRemapperPeriodicThread.h>

#define THPERIOD 0.01 // seconds.

RosAudioRemapperPeriodicThread::RosAudioRemapperPeriodicThread() : 
	PeriodicThread(THPERIOD) {

	robot = "icub";
	startTime = yarp::os::Time::now();        
}


RosAudioRemapperPeriodicThread::RosAudioRemapperPeriodicThread(std::string _robot, std::string _configFile) : 
	PeriodicThread(THPERIOD) {

	robot = _robot;
	configFile = _configFile;
	startTime = yarp::os::Time::now();
}


RosAudioRemapperPeriodicThread::~RosAudioRemapperPeriodicThread() {
	delete outputSound;
}


bool RosAudioRemapperPeriodicThread::configure(yarp::os::ResourceFinder &rf) {
	/* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );
	
	outputSound = new yarp::sig::Sound();

	return true;
}


bool RosAudioRemapperPeriodicThread::threadInit() {
	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */
	
	if (!inRosAudioSubscriber.topic(getName("/rosAudio-@").c_str())) {
		yError("Unable to open subscriber for receiving raw audio from ROS.");
		return false;
	}

	if (!outRawAudioPort.open(getName("/remappedAudio:o").c_str())) {
		yError("Unable to open port for sending the output audio.");
		return false;
	}

	return true;
}


void RosAudioRemapperPeriodicThread::threadRelease() {
	//-- Stop all threads.
	inRosAudioSubscriber.interrupt();
	outRawAudioPort.interrupt();
}


void RosAudioRemapperPeriodicThread::setName(std::string str) {
	this->name=str;
}


std::string RosAudioRemapperPeriodicThread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void RosAudioRemapperPeriodicThread::setInputPortName(std::string InpPort) {
	//-- Do nothing.
}


void RosAudioRemapperPeriodicThread::run() {    
	//-- Get Input.
	
	inputSound = inRosAudioSubscriber.read(true);

	//-- Write to Active Ports.
	//set timestamp to original audio timestamp
	timeStamp.update(inputSound->time);
	outRawAudioPort.setEnvelope(timeStamp);

	outputSound->setFrequency(inputSound->n_frequency);
	
	outputSound->resize(inputSound->n_samples, inputSound->n_channels);

	for(int ch=0; ch < inputSound->n_channels; ch++) {
		for(int i=0; i<inputSound->n_samples; i++) {
			short int val;
			
			//set channel based on left or right
			if(ch == 0)
				val = inputSound->l_channel_data[i];
			else
				val = inputSound->r_channel_data[i];
			
			outputSound->set(val, i, ch);
		}
	}

	outRawAudioPort.write(*outputSound);
	yInfo(" ");
	yInfo("Rate      : %d", inputSound->n_frequency);
	yInfo("Samples   : %d", inputSound->n_samples);
	yInfo("Channels  : %d", inputSound->n_channels);
	yInfo("Timestamp : %f", inputSound->time);

}