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

}


bool RosAudioRemapperPeriodicThread::configure(yarp::os::ResourceFinder &rf) {

	/* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );
	
	/* ===========================================================================
	 *  Initialize time counters to zero.
	 * =========================================================================== */
	totalDelay        = 0.0;
	totalReading      = 0.0;
	totalProcessing   = 0.0;
	totalTransmission = 0.0;
	totalTime         = 0.0;
	totalIterations   = 0;

	return true;
}


bool RosAudioRemapperPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */

	if (!inRosAudioPort.open(getName("/rosAudio:i").c_str())) {
		yError("Unable to open port for receiving raw audio from ROS.");
		return false;
	}

	if (!outRawAudioPort.open(getName("/rawAudio:o").c_str())) {
		yError("Unable to open port for sending the output audio.");
		return false;
	}

	stopTime = yarp::os::Time::now();
	yInfo("Initialization of the processing thread correctly ended. Elapsed Time: %f.", stopTime - startTime);
	startTime = stopTime;

	return true;
}


void RosAudioRemapperPeriodicThread::threadRelease() {

	//-- Stop all threads.
	inRosAudioPort.interrupt();
	outRawAudioPort.interrupt();

	//-- Print thread stats.
	endOfProcessingStats();	
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
	
	if (inRosAudioPort.getInputCount()) {

		//-- Get Input.
		inputSound = inRosAudioPort.read(true);
		inRosAudioPort.getEnvelope(timeStamp);

				//-- Write to Active Ports.
		if (outRawAudioPort.getOutputCount()) {

			//-- This Matrix can be very big. Down sample if enabled.
			timeStamp.update(inputSound->time);
			outRawAudioPort.setEnvelope(timeStamp);

			outputSound->setFrequency(inputSound->n_frequency);
			outputSound->resize(inputSound->n_samples, inputSound->n_channels);

			for(int ch=0; ch < inputSound->n_channels; ch++) {
				for(int i=0; i<inputSound->n_samples; i++) {
					auto val = (ch == 0 ? inputSound->l_channel_data[i] : inputSound->r_channel_data[i]);
					outputSound->set(val, i, ch);
				}
			}
			
			outRawAudioPort.write(outputSound);
		}

		//-- Give time stats to the user.
		timeTotal  = timeDelay + timeReading + timeProcessing + timeTransmission;
		totalTime += timeTotal;
		totalIterations++;
		yInfo("End of Loop %d, TS %d: Delay  %f  |  Reading  %f  |  Processing  %f  |  Transmission  %f  |  Total  %f  |", totalIterations, timeStamp.getCount(), timeDelay, timeReading, timeProcessing, timeTransmission, timeTotal);
	}
}

void RosAudioRemapperPeriodicThread::endOfProcessingStats() {

	//-- Display Execution stats.
	yInfo(" ");
	yInfo("End of Thread . . . ");
	yInfo(" ");
	yInfo("\t Total Iterations : %d", totalIterations);
	yInfo("\t Total Time       : %.2f", totalTime);
	yInfo(" ");
	yInfo("Average Stats . . . ");
	yInfo(" ");
	yInfo("\t Delay        : %f", totalDelay        / (double) totalIterations );
	yInfo("\t Reading      : %f", totalReading      / (double) totalIterations );
	yInfo("\t Processing   : %f", totalProcessing   / (double) totalIterations );
	yInfo("\t Transmission : %f", totalTransmission / (double) totalIterations );
	yInfo("\t Loop Time    : %f", totalTime         / (double) totalIterations );
	yInfo(" ");
}
