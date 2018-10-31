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

#define THPERIOD 0.08 // seconds.


inline void ones(yarp::sig::Matrix& mat) {
	for (int r = 0; r < mat.rows(); r++) {
		for (int c = 0; c < mat.cols(); c++) {
			mat[r][c] = 1.0;
		}
	}
}


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

	bufferSize    = rf.findGroup("bayesianmap").check("bufferSize",    yarp::os::Value(100), "number of audio maps to remember (int)"     ).asInt();
	numOmpThreads = rf.findGroup("bayesianmap").check("numOmpThreads", yarp::os::Value(4),   "if enabled, the number of omp threads (int)").asInt();
	
	/* ===========================================================================
	 *  Derive additional variables given the ones above.
	 * =========================================================================== */
	numFullFieldAngles = _baseAngles * angleRes * 2;

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
	 *  Print the resulting variables to the console.
	 * =========================================================================== */
	yInfo( " " );
	yInfo( "\t                 [PROCESSING]                 "                                );
	yInfo( "\t ============================================ "                                );
	yInfo( "\t Number of Frequency Bands        : %d",   numBands                            );
	yInfo( "\t Number of Full Field Angles      : %d",   numFullFieldAngles                  );
	yInfo( " " );
	yInfo( "\t                [Bayesian Map]                "                                );
	yInfo( "\t ============================================ "                                );
	yInfo( "\t Number of Frequency Bands        : %d",   numBands                            );
	yInfo( "\t Number of Full Field Angles      : %d",   numFullFieldAngles                  );
	#ifdef WITH_OMP
	yInfo( "\t Number of OpenMP Threads         : %d",   numOmpThreads                       );
	#else
	yInfo( "\t Number of OpenMP Threads         : DISABLED"                                  );
	#endif
	yInfo( " " );

	return true;
}


bool AudioBayesianMapPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *   let RFModule know initialization was unsuccessful.
	 * =========================================================================== */

	if (!inAllocentricAudioPort.open(getName("/allocentricAudio:i").c_str())) {
		yError("Unable to open port for receiving an allocentric map of the auditory environment.");
		return false;
	}

	if (!outProbabilityMapPort.open(getName("/probabilityMap:o").c_str())) {
		yError("Unable to open port for sending the probability of the location of each frequency band.");
		return false;
	}

	if (!outProbabilityAnglePort.open(getName("/probabilityAngle:o").c_str())) {
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

		//-- Grab the time time difference of waiting between loops.
		stopTime  = yarp::os::Time::now();
		timeDelay = stopTime - startTime;
		startTime = stopTime;

		//-- Get Input.
		AllocentricAudioMatrix = *inAllocentricAudioPort.read(true);
		inAllocentricAudioPort.getEnvelope(timeStamp);

		//-- Grab the time difference of reading input.
		stopTime    = yarp::os::Time::now();
		timeReading = stopTime - startTime;
		startTime   = stopTime;

		//-- Main Loop.
		result = processing();

		//-- Grab the time difference of processing the input.
		stopTime       = yarp::os::Time::now();
		timeProcessing = stopTime - startTime;
		startTime      = stopTime;

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

		//-- Grab the time delay of publishing on ports.
		stopTime = yarp::os::Time::now();
		timeTransmission = stopTime - startTime;
		startTime = stopTime;

		//-- Give time stats to the user.
		timeTotal = timeDelay + timeReading + timeProcessing + timeTransmission;
		yInfo("End of Loop %d:  Delay  %f  |  Reading  %f  |  Processing  %f  |  Transmission  %f  |  Total  %f  |", timeStamp.getCount(), timeDelay, timeReading, timeProcessing, timeTransmission, timeTotal);
	}
}


bool AudioBayesianMapPeriodicThread::processing(){

	#ifdef WITH_OMP
	omp_set_num_threads(numOmpThreads);
	#endif

	updateBayesianProbabilities (
		/* Target = */ ProbabilityMapMatrix, 
		/* Buffer = */ bufferedAudioMatrix,
		/* Length = */ bufferSize,
		/* Source = */ AllocentricAudioMatrix
	);


	return true;
}


void AudioBayesianMapPeriodicThread::updateBayesianProbabilities(yarp::sig::Matrix& ProbabilityMap, std::queue< yarp::sig::Matrix >& BufferedAudio, const int BufferLength, const yarp::sig::Matrix& CurrentAudio) {
	
	//-- Add the current audio map to the buffer.
	BufferedAudio.push(CurrentAudio);

	//-- Add the current audio map to the state knowledge.
	addAudioMap(ProbabilityMap, CurrentAudio);

	//-- If the buffer is full, begin to remove audio maps.
	if (BufferedAudio.size() >= BufferLength) {

		//-- Remove an Audio Event.
		removeAudioMap(ProbabilityMap, BufferedAudio.front());

		//-- Get rid of it.
		BufferedAudio.pop();
	}

	//-- Normalize the resulting probability map.
	normaliseMatrix(ProbabilityMap);
}


void AudioBayesianMapPeriodicThread::addAudioMap(yarp::sig::Matrix& ProbabilityMap, const yarp::sig::Matrix& CurrentAudio) {

	int band, column;
	int columnLength = ProbabilityMap.cols();

	#ifdef WITH_OMP
	#pragma omp parallel      \
	 shared  (ProbabilityMap, CurrentAudio, numBands, columnLength) \
	 private (band, column)
	#pragma omp for schedule(guided)
	#endif
	for (band = 0; band < numBands; band++) {
		for (column = 0; column < columnLength; column++) {
			ProbabilityMap[band][column] *= CurrentAudio[band][column];
		}
	}
}


void AudioBayesianMapPeriodicThread::removeAudioMap(yarp::sig::Matrix& ProbabilityMap, const yarp::sig::Matrix& AntiquatedAudio) {

	int band, column;
	int columnLength = ProbabilityMap.cols();

	#ifdef WITH_OMP
	#pragma omp parallel      \
	 shared  (ProbabilityMap, AntiquatedAudio, numBands, columnLength) \
	 private (band, column)
	#pragma omp for schedule(guided)
	#endif
	for (band = 0; band < numBands; band++) {
		for (column = 0; column < columnLength; column++) {
			ProbabilityMap[band][column] /= AntiquatedAudio[band][column];
		}
	}
}


void AudioBayesianMapPeriodicThread::normaliseMatrix(yarp::sig::Matrix& matrix) {

	int RowLength, ColumnLength, row;
	RowLength    = matrix.rows();
	ColumnLength = matrix.cols();

	for (row = 0; row < RowLength; row++) {
		normaliseRow(matrix[row], ColumnLength);
	}
}


void AudioBayesianMapPeriodicThread::normaliseRow(double *MatrixRow, const int Length) {

	double sum = 0.0;
	for (int index = 0; index < Length; index++) {
		sum += MatrixRow[index];
	}

	for (int index = 0; index < Length; index++) {
		MatrixRow[index] /= sum;
	}
}

void AudioBayesianMapPeriodicThread::clearProbabilities() {

	//-- Reset running probabilities.
	ProbabilityMapMatrix.zero();
	ones(ProbabilityMapMatrix);   //-- Set all positions to one.
	
	//-- Clear out the buffer.
	while (!bufferedAudioMatrix.empty()) {
		bufferedAudioMatrix.pop();
	}
}