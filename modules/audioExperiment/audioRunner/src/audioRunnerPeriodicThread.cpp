// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Department of Neuroscience - University of Lethbridge
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
 * @file  audioRunnerPeriodicThread.cpp
 * @brief Implementation of the audioRunnerPeriodicThread (see header file).
 * =========================================================================== */

#include <iCub/audioRunnerPeriodicThread.h>

#define THPERIOD 1.0 // seconds.


AudioRunnerPeriodicThread::AudioRunnerPeriodicThread() : 
	PeriodicThread(THPERIOD) {

	robot = "icub";
	startTime = yarp::os::Time::now();
}


AudioRunnerPeriodicThread::AudioRunnerPeriodicThread(std::string _robot, std::string _configFile) : 
	PeriodicThread(THPERIOD) {

	robot = _robot;
	configFile = _configFile;
	startTime = yarp::os::Time::now();
}


AudioRunnerPeriodicThread::~AudioRunnerPeriodicThread() {
}


bool AudioRunnerPeriodicThread::configure(yarp::os::ResourceFinder &rf) {
	
	/* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );

	panAngle   = rf.findGroup("robotspec").check("panAngle",   yarp::os::Value( 2),    "index of pan joint       (int)"    ).asInt16();
	numMics    = rf.findGroup("robotspec").check("numMics",    yarp::os::Value( 2),    "number of mics           (int)"    ).asInt16();

	beginTrial = rf.findGroup("experiment").check("begin",     yarp::os::Value(1),     "first trial to begin     (int)"    ).asInt16();
	endTrial   = rf.findGroup("experiment").check("end",       yarp::os::Value(5),     "last trial to end        (int)"    ).asInt16();
	filePath   = rf.findGroup("experiment").check("saveTo",    yarp::os::Value("./"),  "path for saving files    (string)" ).asString();
	movements  = rf.findGroup("experiment").check("movements", yarp::os::Value(false), "should movements be used (bool)"   ).asBool();
	PosBottle  = rf.findGroup("experiment").find( "movePos"                                                                ).asList();
	TimeBottle = rf.findGroup("experiment").find( "moveTime"                                                               ).asList();
    
	numFrameSamples = rf.findGroup("sampling").check("numFrameSamples", yarp::os::Value(4096), "number of frame samples received (int)").asInt16();

	//-- Set the first trial.
	currentTrial = beginTrial;

	//-- Ensure the audio buffer is empty.
	AudioBuffer.clear();
	PositionBuffer.clear();

	//-- Expand File path if it has environment variables.
	filePath = AudioUtil::expandEnvironmentVariables(filePath);

	//-- Create this directory if it does not exist.
	if (!AudioUtil::makeDirectory(filePath)) {
		yInfo("Could not create directory %s -- Check Permissions?", filePath.c_str());
		return false;
	}

	if (movements) {

		//-- If the key could not be found in the config file, make a fallback bottle.
		if (PosBottle  == NULL) { PosBottle  = new yarp::os::Bottle(fallback_pos);   }
		if (TimeBottle == NULL) { TimeBottle = new yarp::os::Bottle(fallback_times); }

		//-- Check that the number of positions and times are equal for the trials.
		if (PosBottle->size() != TimeBottle->size()) {
			yError("Number of elements provided to positions and time are not equal!!");
			yError(" Elements in Position : %s", PosBottle->toString().c_str());
			yError(" Elements in Time     : %s", TimeBottle->toString().c_str());
			return false;
		}
		
		//-- Allocate Space for the movements.
		numMoves = PosBottle->size();
		HeadPositions.clear(); HeadPositions.resize(numMoves);
		HeadTimes.clear();     HeadTimes.resize(numMoves);

		//-- Set the movements in the vectors.
		for (int move = 0; move < numMoves; move++) {
			HeadPositions[move] = PosBottle->get(move).asFloat64();
			HeadTimes[move]     = TimeBottle->get(move).asFloat64();
		}
	}

	/* ===========================================================================
	 *  Initialize time counters to zero.
	 * =========================================================================== */
	totalDelay        = 0.0; timeDelay        = 0.0;
	totalReading      = 0.0; timeReading      = 0.0;
	totalProcessing   = 0.0; timeProcessing   = 0.0;
	totalTransmission = 0.0; timeTransmission = 0.0;
	totalTime         = 0.0; timeTotal        = 0.0;
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
	yInfo( "\t                 [Experiment]                 "                );
	yInfo( "\t ============================================ "                );
	yInfo( "\t Beginning Trial                  : %d",       beginTrial      );
	yInfo( "\t Ending Trial                     : %d",       endTrial        );
	yInfo( "\t Trial Data Directory             : %s",       filePath.c_str());
	yInfo( "\t Robot Movements                  : %s",       movements ? "ENABLED" : "DISABLED");
	if (movements) {
	yInfo( "\t Number of Movements              : %d",       numMoves        );
	}
	yInfo( " " );
	yInfo( "\t                  [SAMPLING]                  "                );
	yInfo( "\t ============================================ "                );
	yInfo( "\t Number Samples per Frame         : %d",       numFrameSamples );
	yInfo( " " );

	if (movements) {
		yInfo( " " );
		yInfo( "\t               [MOVEMENT POLICY]              " );
		yInfo( "\t  Move#     :     Position      :    Time     " );
		yInfo( "\t ============================================ " );
		for (int move = 0; move < numMoves; move++) {
			yInfo( "\t   %s       :\t    %.2f \t:   %.2f s", 
					AudioUtil::leadingZeros(move, 2).c_str(), 
					HeadPositions[move], 
					HeadTimes[move] 
			);
		}
		yInfo( " " );
	}
	
	return true;
}


bool AudioRunnerPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */

	if (!outPlayerCommandsPort.open(getName("/commands:o").c_str())) {
		yInfo("Unable to open port for sending commands to the audio player.");
		return false;
	}

	if (!inBroadcastPort.open(getName("/broadcast:i").c_str())) {
		yInfo("Unable to open port for receiving broadcasted reports from audio player.");
		return false;
	}

	if (!inRawAudioPort.open(getName("/rawAudio:i").c_str())) {
		yError("Unable to open port for receiving raw audio from head.");
		return false;
	}

	if (movements) { 
		if (!inHeadAnglePort.open(getName("/headAngle:i").c_str())) {
			yError("Unable to open port for receiving head positions.");
			return false;
		}

		if (!outHeadMovePort.open(getName("/headAngle:o").c_str())) {
			yError("Unable to open port for sending head positions.");
			return false;
		}
	}

	stopTime = yarp::os::Time::now();
	yInfo("Initialization of the processing thread correctly ended. Elapsed Time: %f.", stopTime - startTime);
	startTime = stopTime;
	
	return true;
}


void AudioRunnerPeriodicThread::threadRelease() {

	//-- Stop all threads.
	outPlayerCommandsPort.interrupt();
	inBroadcastPort.interrupt();
	inRawAudioPort.interrupt();
	inHeadAnglePort.interrupt();
	outHeadMovePort.interrupt();

	//-- Close the threads.
	outPlayerCommandsPort.close();
	inBroadcastPort.close();
	inRawAudioPort.close();
	inHeadAnglePort.close();
	outHeadMovePort.close();

	//-- Print thread stats.
	endOfProcessingStats();	
}


void AudioRunnerPeriodicThread::setName(std::string str) {
	this->name=str;
}


std::string AudioRunnerPeriodicThread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void AudioRunnerPeriodicThread::setInputPortName(std::string InpPort) {
	//-- Do nothing.
}


void AudioRunnerPeriodicThread::run() {    

	//-- Don't do any work until everything is connected.
	if (outPlayerCommandsPort.getOutputCount()          && 
		inBroadcastPort.getInputCount()                 && 
		inRawAudioPort.getInputCount()                  && 
		(!movements || inHeadAnglePort.getInputCount()) &&
		(!movements || outHeadMovePort.getOutputCount())) {

		AudioUtil::makeTimeStamp(totalDelay, timeDelay, startTime, stopTime);

		//-- Main Loop.
		result = processing();

		AudioUtil::makeTimeStamp(totalProcessing, timeProcessing, startTime, stopTime);

		//-- Give time stats to the user.
		timeTotal  = timeDelay + timeReading + timeProcessing + timeTransmission;
		totalTime += timeTotal;
		totalIterations++;
		yInfo("End of Loop %d, ts %d:  Delay  %f  |  Processing  %f  |  Total  %f  |", totalIterations, timeStamp.getCount(), timeDelay, timeProcessing, timeTotal);

	} else {

		std::string msg = "Missing Connection to ";
		if (!outPlayerCommandsPort.getOutputCount())        { msg += "Player; ";       }
		if (!inBroadcastPort.getInputCount())               { msg += "Broadcast; ";    }
		if (!inRawAudioPort.getInputCount())                { msg += "Audio; ";        }
		if (movements && !inHeadAnglePort.getInputCount())  { msg += "HeadAngle; ";    }
		if (movements && !outHeadMovePort.getOutputCount()) { msg += "HeadPosition; "; }
		msg += ". . .";
		yInfo("%s", msg.c_str());
	}

	//-- Stop this thread when finished.
	if (currentTrial > endTrial) {

		//-- Move the head back to home at the end of a trial.
		if (movements) {
			yarp::os::Bottle& head_command = outHeadMovePort.prepare();
			head_command.clear();
			head_command.addString("move");
			head_command.addFloat64(0.0);
			outHeadMovePort.write();
		}

		yInfo("Completed All Trials . . . Good Bye!");

		yarp::os::Bottle& command = outPlayerCommandsPort.prepare();
		command.clear();
		command.addString("quit");
		outPlayerCommandsPort.write();

		this->askToStop();
	}
}


bool AudioRunnerPeriodicThread::processing() {

	if (movements) {
		/* ===========================================================================
		 *  Init the trial by sending the head to the starting position.
		 * =========================================================================== */
		currentMove = 0;
		yarp::os::Bottle& head_command = outHeadMovePort.prepare();
		head_command.clear();
		head_command.addString("move");
		head_command.addFloat64(HeadPositions[currentMove]);
		outHeadMovePort.write();
		
		currentMove++;

		//-- Sleep for five second.
		usleep(5000000);

		//-- Sleep for twenty second.
		//usleep(20000000);
	}

	/* ===========================================================================
	 *  Begin a trial by writing the trial number to the player.
	 * =========================================================================== */
	yarp::os::Bottle& command = outPlayerCommandsPort.prepare();
	command.clear();
	command.addString("trial");
	command.addInt16(currentTrial);
	outPlayerCommandsPort.write();

	int targetAt;
	double trialTimeBegin = yarp::os::Time::now();

	while (true) {
		
		//-- Move the head if time.
		if (movements && currentMove < numMoves) {
			if (trialTimeBegin + HeadTimes[currentMove] < yarp::os::Time::now()) {
				yarp::os::Bottle& head_command = outHeadMovePort.prepare();
				head_command.clear();
				head_command.addString("move");
				head_command.addFloat64(HeadPositions[currentMove]);
				outHeadMovePort.write();
				currentMove++;
			}
		}

		//-- Read in Audio.
		inputSound = inRawAudioPort.read(true);
		inRawAudioPort.getEnvelope(timeStamp);

		//-- Push a copy of the sound into the buffer.
		AudioBuffer.push_back(*inputSound);

		//-- Read the Head Angle.
		if (movements) {
			inputAngles = inHeadAnglePort.read(true);
			PositionBuffer.push_back(inputAngles->get(panAngle).asFloat64());
		}

		//-- See if Trial has finished.
		yarp::os::Bottle* reply = inBroadcastPort.read(false);
		if (reply != NULL) {
			//yInfo("%s", reply->toString().c_str());
			targetAt = reply->get(1).asInt16();
			break;
		}
	}
	
	//-- Give the filename information of the trial.
	std::string filename = filePath + 
		"yarpSound_" + 
		AudioUtil::leadingZeros(currentTrial, 4) + "_" + 
		std::to_string(targetAt)                 + "_" + 
		std::to_string(numFrameSamples)          + "_" + 
		std::to_string(numMics)                  + "_" +
		std::to_string(AudioBuffer.size())       + ".data";

	//-- Save the buffer to a file.
	saveTrial(filename);

	//-- Clear the buffer.
	AudioBuffer.clear();

	if (movements) {
		filename = filePath + 
			"yarpPan_" + 
			AudioUtil::leadingZeros(currentTrial, 4) + "_" + 
			std::to_string(targetAt)                 + "_" + 
			std::to_string(numFrameSamples)          + "_" + 
			std::to_string(numMics)                  + "_" +
			std::to_string(PositionBuffer.size())    + ".data";

		//-- Save the buffer to a file.
		savePositions(filename);

		//-- Clear the buffer.
		PositionBuffer.clear();
	}

	//-- Increment to the next trial.
	currentTrial++;

	return true;
}


void AudioRunnerPeriodicThread::saveTrial(const std::string filename) {

	std::ofstream writer(filename);

	size_t numFrames = AudioBuffer.size();

	for (int mic = 0; mic < numMics; mic++) {
		for (size_t frame = 0; frame < numFrames; frame++) {
			for (int sample = 0; sample < numFrameSamples; sample++) {
				writer << AudioBuffer[frame].get(sample, mic) << " ";
			}
		} writer << "\n";
	}

	writer.close();
}


void AudioRunnerPeriodicThread::savePositions(const std::string filename) {

	std::ofstream writer(filename);

	size_t numPositions = PositionBuffer.size();

	for (int pos = 0; pos < numPositions; pos++) {
		writer << PositionBuffer[pos] << " ";
	}
}


void AudioRunnerPeriodicThread::endOfProcessingStats() {

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
