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
 * @file  audioBayesianMapPeriodicThread.cpp
 * @brief Implementation of the audioBayesianMapPeriodicThread (see header file).
 * =========================================================================== */

#include <iCub/audioBayesianMapPeriodicThread.h>

#define THPERIOD 0.01 // seconds.


AudioBayesianMapPeriodicThread::AudioBayesianMapPeriodicThread() : 
	PeriodicThread(THPERIOD) {

	robot = "icub";
	startTime = yarp::os::Time::now();
}


AudioBayesianMapPeriodicThread::AudioBayesianMapPeriodicThread(std::string _robot, std::string _configFile) : 
	PeriodicThread(THPERIOD) {

	robot = _robot;
	configFile = _configFile;
	startTime = yarp::os::Time::now();
}


AudioBayesianMapPeriodicThread::~AudioBayesianMapPeriodicThread() {
	
}


bool AudioBayesianMapPeriodicThread::configure(yarp::os::ResourceFinder &rf) {
	
	/* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );

	numBands = rf.findGroup("processing").check("numBands", yarp::os::Value(128), "number of frequency bands (int)"            ).asInt();
	angleRes = rf.findGroup("processing").check("angleRes", yarp::os::Value(1),   "degree resolution for single position (int)").asInt();

	bufferSize = rf.findGroup("bayesianmap").check("bufferSize", yarp::os::Value(100), "number of audio maps to remember (int)").asInt();
	
	saveMatrices = rf.findGroup("other").check("saveMatrices",  yarp::os::Value("NONE"), "if specified, will save all matrices to this dir (string)").asString();

	/* ===========================================================================
	 *  Derive additional variables given the ones above.
	 * =========================================================================== */
	numFullFieldAngles = _baseAngles * angleRes * 2;


	//-- If save matricies was specified set process all flag to true.
	processAll = (saveMatrices != "NONE");
	if (processAll) {

		//-- Expand env vars if they are present.
		saveMatrices = AudioUtil::expandEnvironmentVariables(saveMatrices);

		//-- Create a directory for the matrices.
		if (!AudioUtil::makeDirectory(saveMatrices)) {
			yInfo("Failed to create directory %s. -- Check Permissions?", saveMatrices.c_str());
			return false;
		}
	}


	/* ===========================================================================
	 *  Initialize the matrices used for data processing.
	 * =========================================================================== */
	AllocentricAudioMatrix.resize(numBands, numFullFieldAngles);
	AllocentricAudioMatrix.zero();

	ProbabilityMapMatrix.resize(numBands, numFullFieldAngles);
	ProbabilityMapMatrix.zero();

	ProbabilityAngleMatrix.resize(1, numFullFieldAngles);
	ProbabilityAngleMatrix.zero();

	clearProbabilities();


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
	yInfo( "\t                 [PROCESSING]                 "               );
	yInfo( "\t ============================================ "               );
	yInfo( "\t Number of Frequency Bands        : %d",   numBands           );
	yInfo( "\t Number of Full Field Angles      : %d",   numFullFieldAngles );
	yInfo( " " );
	yInfo( "\t                [Bayesian Map]                "               );
	yInfo( "\t ============================================ "               );
	yInfo( "\t Number of Audio Maps Stored      : %d",   bufferSize         );
	yInfo( " " );
	yInfo( "\t                    [OTHER]                   "               );
	yInfo( "\t ============================================ "               );
	yInfo( "\t Directory for Matrix Output      : %s",    processAll ? saveMatrices.c_str() : "DISABLED");
	yInfo( " " );
	
	return true;
}


bool AudioBayesianMapPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */

	if (!inAllocentricAudioPort.open(getName("/allocentricAudio:i").c_str())) {
		yError("Unable to open port for receiving an allocentric map of the auditory environment.");
		return false;
	}

	if (!outProbabilityMapPort.open(getName("/bayesianProbabilityMap:o").c_str())) {
		yError("Unable to open port for sending the probability of the location of each frequency band.");
		return false;
	}

	if (!outProbabilityAnglePort.open(getName("/bayesianProbabilityAngle:o").c_str())) {
		yError("Unable to open port for sending the probability of overall sound source at corresponding angle.");
		return false;
	}

	stopTime = yarp::os::Time::now();
	yInfo("Initialization of the processing thread correctly ended. Elapsed Time: %f.", stopTime - startTime);
	startTime = stopTime;
	
	return true;
}


void AudioBayesianMapPeriodicThread::threadRelease() {

	//-- Stop all threads.
	inAllocentricAudioPort.interrupt();
	outProbabilityMapPort.interrupt();
	outProbabilityAnglePort.interrupt();

	//-- Close the threads.
	inAllocentricAudioPort.close();
	outProbabilityMapPort.close();
	outProbabilityAnglePort.close();

	//-- Print thread stats.
	endOfProcessingStats();	
}


void AudioBayesianMapPeriodicThread::setName(std::string str) {
	this->name=str;
}


std::string AudioBayesianMapPeriodicThread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void AudioBayesianMapPeriodicThread::setInputPortName(std::string InpPort) {
	//-- Do nothing.
}


void AudioBayesianMapPeriodicThread::run() {    
	
	if (inAllocentricAudioPort.getInputCount()) {

		AudioUtil::makeTimeStamp(totalDelay, timeDelay, startTime, stopTime);
		
		//-- Get Input.
		AllocentricAudioMatrix = *inAllocentricAudioPort.read(true);
		inAllocentricAudioPort.getEnvelope(timeStamp);

		AudioUtil::makeTimeStamp(totalReading, timeReading, startTime, stopTime);

		//-- Main Loop.
		result = processing();

		AudioUtil::makeTimeStamp(totalProcessing, timeProcessing, startTime, stopTime);

		//-- Write data to outgoing ports.
		publishOutPorts();

		//-- If saving output was set, save all matrices.
		if (processAll) { saveOutPorts(); }

		AudioUtil::makeTimeStamp(totalTransmission, timeTransmission, startTime, stopTime);

		//-- Give time stats to the user.
		timeTotal  = timeDelay + timeReading + timeProcessing + timeTransmission;
		totalTime += timeTotal;
		totalIterations++;
		yInfo("End of Loop %d, TS %d:  Delay  %f  |  Reading  %f  |  Processing  %f  |  Transmission  %f  |  Total  %f  |", totalIterations, timeStamp.getCount(), timeDelay, timeReading, timeProcessing, timeTransmission, timeTotal);
	}
}


bool AudioBayesianMapPeriodicThread::processing() {

	/* ===========================================================================
	 *  Update the knowledge state given the new information from the received 
	 *    map. Store this map in the buffer, so that later this map can be
	 *    ``forgotten`` from the knowledge. If the specified buffer limit has
	 *    met, the oldest information will be removed.
	 * =========================================================================== */
	updateBayesianProbabilities (
		/* Source = */ AllocentricAudioMatrix,
		/* Target = */ ProbabilityMapMatrix, 
		/* Buffer = */ bufferedAudioMatrix,
		/* Length = */ bufferSize
	);


	/* ===========================================================================
	 *  If someone is connected to this port, collapse probability map along
	 *    the bands, so that at the end of processing it can be published.
	 * =========================================================================== */
	if (processAll || outProbabilityAnglePort.getOutputCount()) {
		collapseProbabilityMap (
			/* Source = */ ProbabilityMapMatrix,
			/* Target = */ ProbabilityAngleMatrix
		);
	}

	return true;
}


void AudioBayesianMapPeriodicThread::updateBayesianProbabilities(const yarp::sig::Matrix& CurrentAudio, yarp::sig::Matrix& ProbabilityMap, std::queue< yarp::sig::Matrix >& BufferedAudio, const int BufferLength) {
	
	//-- Add the current audio map to the buffer.
	BufferedAudio.push(CurrentAudio);

	//-- Add the current audio map to the state knowledge.
	addAudioMap(CurrentAudio, ProbabilityMap);

	//-- If the buffer is full, begin to remove audio maps.
	if (BufferedAudio.size() >= BufferLength) {

		//-- Remove an Audio Event.
		removeAudioMap(BufferedAudio.front(), ProbabilityMap);

		//-- Get rid of it.
		BufferedAudio.pop();
	}

	//-- Normalize the resulting probability map.
	normaliseMatrix(ProbabilityMap);
}


void AudioBayesianMapPeriodicThread::addAudioMap(const yarp::sig::Matrix& CurrentAudio, yarp::sig::Matrix& ProbabilityMap) {

	//-- Add the new audio to the knowledge state.
	int columnLength = ProbabilityMap.cols();

	for (int band = 0; band < numBands; band++) {
		for (int column = 0; column < columnLength; column++) {
			ProbabilityMap[band][column] *= CurrentAudio[band][column];
		}
	}
}


void AudioBayesianMapPeriodicThread::removeAudioMap(const yarp::sig::Matrix& AntiquatedAudio, yarp::sig::Matrix& ProbabilityMap) {

	//-- Remove the old audio from the knowledge state.
	int columnLength = ProbabilityMap.cols();

	for (int band = 0; band < numBands; band++) {
		for (int column = 0; column < columnLength; column++) {
			ProbabilityMap[band][column] /= AntiquatedAudio[band][column];
		}
	}
}


void AudioBayesianMapPeriodicThread::normaliseMatrix(yarp::sig::Matrix& matrix) {

	//-- Normalise each row of a yarp matrix.
	int RowLength    = matrix.rows();
	int ColumnLength = matrix.cols();

	for (int row = 0; row < RowLength; row++) {
		normaliseRow(matrix[row], ColumnLength);
	}
}


void AudioBayesianMapPeriodicThread::normaliseRow(double *MatrixRow, const int Length) {

	//-- Take the sum of an entire row, then divide each element
	//-- by it. This producees a row that sums to one.
	double sum = 0.0;
	for (int index = 0; index < Length; index++) {
		sum += MatrixRow[index];
	}

	for (int index = 0; index < Length; index++) {
		MatrixRow[index] /= sum;
	}
}


void AudioBayesianMapPeriodicThread::collapseProbabilityMap(const yarp::sig::Matrix& ProbabilityMap, yarp::sig::Matrix& ProbabilityAngles) {
	
	//-- Collapse the knowledge state across the frequency bands.
	ProbabilityAngles.resize(1, numFullFieldAngles);
	ProbabilityAngles.zero();

	for (int band = 0; band < numBands; band++) {
		for (int angle = 0; angle < numFullFieldAngles; angle++) {
			ProbabilityAngles[0][angle] += ProbabilityMap[band][angle];
		}
	}

	//-- Normalise the matrix.
	normaliseMatrix(ProbabilityAngles);
}


void AudioBayesianMapPeriodicThread::clearProbabilities() {

	//-- Reset running probabilities.
	ProbabilityMapMatrix.resize(numBands, numFullFieldAngles);
	AudioUtil::ones(ProbabilityMapMatrix);   //-- Set all positions to one.
	
	//-- Clear out the buffer.
	while (!bufferedAudioMatrix.empty()) {
		bufferedAudioMatrix.pop();
	}
}



void AudioBayesianMapPeriodicThread::publishOutPorts() {
	
	//-- Write to Active Ports.
	if (outProbabilityMapPort.getOutputCount()) {
		outProbabilityMapPort.prepare() = ProbabilityMapMatrix;
		outProbabilityMapPort.setEnvelope(timeStamp);
		outProbabilityMapPort.write();
	}

	if (outProbabilityAnglePort.getOutputCount()) {
		outProbabilityAnglePort.prepare() = ProbabilityAngleMatrix;
		outProbabilityAnglePort.setEnvelope(timeStamp);
		outProbabilityAnglePort.write();
	}
}


void AudioBayesianMapPeriodicThread::saveOutPorts() {

	AudioUtil::MatrixToFile(ProbabilityMapMatrix,   saveMatrices + "ProbabilityMap_"   +AudioUtil::leadingZeros(totalIterations,4)+".data");
	AudioUtil::MatrixToFile(ProbabilityAngleMatrix, saveMatrices + "ProbabilityAngle_" +AudioUtil::leadingZeros(totalIterations,4)+".data");
}


void AudioBayesianMapPeriodicThread::endOfProcessingStats() {

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
