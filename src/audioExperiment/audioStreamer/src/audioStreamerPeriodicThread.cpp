// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2018 Department of Neuroscience - University of Lethbridge
 * Author: Austin Kothig, Francesco Rea, Marko Ilievski, Matt Tata
 * email: kothiga@uleth.ca, francesco.reak@iit.it, marko.ilievski@uwaterloo.ca, matthew.tata@uleth.ca
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
 * @file  audioStreamerPeriodicThread.cpp
 * @brief Implementation of the audioStreamerPeriodicThread (see header file).
 * =========================================================================== */

#include <iCub/audioStreamerPeriodicThread.h>

#define THPERIOD 0.01 // seconds.


AudioStreamerPeriodicThread::AudioStreamerPeriodicThread() : 
	PeriodicThread(THPERIOD) {

	robot = "icub";
	startTime = yarp::os::Time::now();
}


AudioStreamerPeriodicThread::AudioStreamerPeriodicThread(std::string _robot, std::string _configFile) : 
	PeriodicThread(THPERIOD) {

	robot = _robot;
	configFile = _configFile;
	startTime = yarp::os::Time::now();
}


AudioStreamerPeriodicThread::~AudioStreamerPeriodicThread() {
	delete robotHead;
	delete robotPos;
	delete robotVel;
	delete robotEnc;
	delete robotMic;
	delete robotSound;
}


bool AudioStreamerPeriodicThread::configure(yarp::os::ResourceFinder &rf) {
	
	/* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );

	panAngle   = rf.findGroup("robotspec").check("panAngle",   yarp::os::Value( 2),    "index of pan joint (int)"             ).asInt();
	numMics    = rf.findGroup("robotspec").check("numMics",    yarp::os::Value( 2),    "number of mics (int)"                 ).asInt();
	minDegree  = rf.findGroup("robotspec").check("minDegree",  yarp::os::Value(-40.0), "minimum degree for the robot (double)").asDouble();
	maxDegree  = rf.findGroup("robotspec").check("maxDegree",  yarp::os::Value( 40.0), "maximum degree for the robot (double)").asDouble();
	motorSpeed = rf.findGroup("robotspec").check("motorSpeed", yarp::os::Value( 30.0), "speed for the robot motors (double)"  ).asDouble();
		
	samplingRate    = rf.findGroup("sampling").check("samplingRate",    yarp::os::Value(48000), "sampling rate of mics (int)"           ).asInt();
	numFrameSamples = rf.findGroup("sampling").check("numFrameSamples", yarp::os::Value(4096),  "number of frame samples received (int)").asInt();
	

	/* ===========================================================================
	 *  Initialize Connection to iCub Head.
	 * =========================================================================== */
	yarp::os::Property RobotHeadOptions;
	RobotHeadOptions.put("device", "remote_controlboard");
  	RobotHeadOptions.put("local",  getName("/local_controlboard"));
  	RobotHeadOptions.put("remote", "/"+robot+"/head");
	
	robotHead = new yarp::dev::PolyDriver(RobotHeadOptions);

	if (!robotHead->isValid()) {
		yInfo("Cannot Connect to Robot Head!");
		return false;
	}

	robotHead->view(robotPos);
	robotHead->view(robotVel);
	robotHead->view(robotEnc);

	if ( robotPos==NULL || robotVel==NULL || robotEnc==NULL ) {
		yInfo("Cannot get Interface to Robot Head!");
		robotHead->close();
		return false;
	}


	//-- Resize the axes.
	int numJoints = 0;
	robotPos->getAxes(&numJoints);
	
	Encoder.resize(numJoints);
	Position.resize(numJoints);
	Speed.resize(numJoints);


	//-- Set the Speed of the motors.
	for (size_t idx = 0; idx < Speed.length(); idx++) {
		Speed[idx] = motorSpeed;
	}

	robotPos->setRefSpeeds(Speed.data());


	//-- Set the initial positions to be the current encoder readings.
	robotEnc->getEncoders(Position.data());
	robotEnc->getEncoders(Encoder.data());
	

	/* ===========================================================================
	 *  Initialize Connection to Microphones on Head.
	 * =========================================================================== */
	yarp::os::Property RobotMicOptions;
	RobotMicOptions.put("device",  "portaudio");
    RobotMicOptions.put("read",    "");
    RobotMicOptions.put("samples", numFrameSamples);
    RobotMicOptions.put("rate",    samplingRate);

	robotMic = new yarp::dev::PolyDriver(RobotMicOptions);

	if (!robotMic->isValid()) {
		yInfo("Cannot Connect to Robot Mics!");
		return false;
	}

	robotMic->view(robotSound);

	if ( robotSound==NULL ) {
		yInfo("Cannot get Interface to Robot Mics!");
		robotMic->close();
		return false;
	}

	stream = true;

	/* ===========================================================================
	 *  Initialize time counters to zero.
	 * =========================================================================== */
	totalDelay        = 0.0;
	totalReading      = 0.0;
	totalProcessing   = 0.0;
	totalTransmission = 0.0;
	totalTime         = 0.0;
	totalIterations   = 0;


	/* =========================================================================== 
	 *  Print the resulting variables to the console.
	 * =========================================================================== */
	yInfo( " " );
	yInfo( "\t               [ROBOT SPECIFIC]               "                );
	yInfo( "\t ============================================ "                );
	yInfo( "\t Index of Pan Joint               : %d",       panAngle        );
	yInfo( "\t Number of Microphones            : %d",       numMics         );
	yInfo( " " );
	yInfo( "\t                  [SAMPLING]                  "                );
	yInfo( "\t ============================================ "                );
	yInfo( "\t Sampling Rate                    : %d Hz",    samplingRate    );
	yInfo( "\t Number Samples per Frame         : %d",       numFrameSamples );
	yInfo( " " );
	
	return true;
}


bool AudioStreamerPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */

	if (!outRawAudioPort.open(getName("/rawAudio:o").c_str())) {
		yError("Unable to open port for sending raw audio from head.");
		return false;
	}

	stopTime = yarp::os::Time::now();
	yInfo("Initialization of the processing thread correctly ended. Elapsed Time: %f.", stopTime - startTime);
	startTime = stopTime;
	
	return true;
}


void AudioStreamerPeriodicThread::threadRelease() {

	//-- Stop all threads.
	outRawAudioPort.interrupt();
	robotSound->stopRecording();	

	//-- Close the threads.
	outRawAudioPort.close();
	robotHead->close();
	robotMic->close();

	//-- Print thread stats.
	endOfProcessingStats();	
}


void AudioStreamerPeriodicThread::setName(std::string str) {
	this->name=str;
}


std::string AudioStreamerPeriodicThread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void AudioStreamerPeriodicThread::setInputPortName(std::string InpPort) {
	//-- Do nothing.
}


void AudioStreamerPeriodicThread::run() {    

	if (stream) {
		AudioUtil::makeTimeStamp(totalDelay, timeDelay, startTime, stopTime);

		//-- Main Loop.
		result = processing();

		AudioUtil::makeTimeStamp(totalProcessing, timeProcessing, startTime, stopTime);

		//-- Write data to outgoing ports.
		publishOutPorts();

		AudioUtil::makeTimeStamp(totalTransmission, timeTransmission, startTime, stopTime);

		//-- Give time stats to the user.
		timeTotal  = timeDelay + timeReading + timeProcessing + timeTransmission;
		totalTime += timeTotal;
		totalIterations++;
		yInfo("End of Loop %d, ts %d:  Delay  %f  |  Processing  %f  |  Transmission  %f  |  Total  %f  |", totalIterations, timeStamp.getCount(), timeDelay, timeProcessing, timeTransmission, timeTotal);
	}
}


bool AudioStreamerPeriodicThread::processing() {

	if(!robotSound->getSound(RawAudio)) {
		yError("Failed to Capture Audio!?");
		return false;
	}

	return true;
}


void AudioStreamerPeriodicThread::moveRobotHead(const double target, std::string& reply) {

	if (target > maxDegree) {
		reply += ("Max Position set to " + std::to_string(maxDegree) + "!");
		return;
	} else if (target < minDegree) {
		reply += ("Min Position set to " + std::to_string(minDegree) + "!");
		return;
	}

	//-- Pause audio streaming.
	robotSound->stopRecording();
	stream = false;

	//-- Begin moving the robot to the target.
	robotPos->positionMove(panAngle, target);

	//-- Wait for the encoder to get close enough to the target position.
	while (true) {

		//-- Get information on the encoders position
		robotEnc->getEncoders(Encoder.data());

		//-- If the encoder position is close enough, break.
		if (std::abs(target - Encoder[panAngle]) < 0.70) {
			break;
		}
	}

	//-- Continue streaming audio.
	robotSound->startRecording();
	stream = true;
}


void AudioStreamerPeriodicThread::publishOutPorts() {
	
	//-- Write to Active Ports.
	timeStamp.update();

	if (outRawAudioPort.getOutputCount()) {
		outRawAudioPort.prepare() = RawAudio;   // TODO: put this in process loop to avoid copy?
		outRawAudioPort.setEnvelope(timeStamp);
		outRawAudioPort.write();
	}
}


void AudioStreamerPeriodicThread::endOfProcessingStats() {

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
