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
 * @file  audioPowerMapPeriodicThread.cpp
 * @brief Implementation of the audioPowerMapPeriodicThread (see header file).
 * =========================================================================== */

#include <iCub/audioPowerMapPeriodicThread.h>

#define THPERIOD 0.08 // seconds.


inline void ones(yarp::sig::Matrix& mat) {
	for (int r = 0; r < mat.rows(); r++) {
		for (int c = 0; c < mat.cols(); c++) {
			mat[r][c] = 1.0;
		}
	}
}


AudioPowerMapPeriodicThread::AudioPowerMapPeriodicThread() : 
	PeriodicThread(THPERIOD) {

	robot = "icub";
	startTime = yarp::os::Time::now();
}


AudioPowerMapPeriodicThread::AudioPowerMapPeriodicThread(std::string _robot, std::string _configFile) : 
	PeriodicThread(THPERIOD) {

	robot = _robot;
	configFile = _configFile;
	startTime = yarp::os::Time::now();
}


AudioPowerMapPeriodicThread::~AudioPowerMapPeriodicThread() {
	
}


bool AudioPowerMapPeriodicThread::configure(yarp::os::ResourceFinder &rf) {
	
	/* ===========================================================================
	 *  Pull variables for this module from the resource finder.
	 * =========================================================================== */
	yInfo( "Loading Configuration File." );

	numMics = rf.findGroup("robotspec").check("numMics", yarp::os::Value(2), "number of mics (int)").asInt();

	numBands = rf.findGroup("processing").check("numBands", yarp::os::Value(128), "number of frequency bands (int)"            ).asInt();
	angleRes = rf.findGroup("processing").check("angleRes", yarp::os::Value(1),   "degree resolution for single position (int)").asInt();

	bufferSize = rf.findGroup("powermap").check("bufferSize", yarp::os::Value(100), "number of audio maps to remember (int)").asInt();
	

	/* ===========================================================================
	 *  Derive additional variables given the ones above.
	 * =========================================================================== */
	numFullFieldAngles = _baseAngles * angleRes * 2;


	/* ===========================================================================
	 *  Initialize the matrices used for data processing.
	 * =========================================================================== */	
	inputBandPower.resize(numBands, numMics);
	inputBandPower.zero();
	
	BandPowerMatrix.resize(numBands, 1);
	BandPowerMatrix.zero();

	ProbabilityMapMatrix.resize(numBands, numFullFieldAngles);
	ProbabilityMapMatrix.zero();

	ProbabilityPowerMatrix.resize(numBands, 1);
	ProbabilityPowerMatrix.zero();

	ProbabilityPowerMapMatrix.resize(numBands, numFullFieldAngles);
	ProbabilityPowerMapMatrix.zero();

	ProbabilityPowerAngleMatrix.resize(1, numFullFieldAngles);
	ProbabilityPowerAngleMatrix.zero();

	InstantaneousPowerProbabilityMapMatrix.resize(numBands, numFullFieldAngles);
	InstantaneousPowerProbabilityMapMatrix.zero();

	InstantaneousPowerProbabilityAngleMatrix.resize(1, numFullFieldAngles);
	InstantaneousPowerProbabilityAngleMatrix.zero();

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
	yInfo( "\t               [ROBOT SPECIFIC]               "               );
	yInfo( "\t ============================================ "               );
	yInfo( "\t Number of Microphones            : %d",   numMics            );
	yInfo( " " );
	yInfo( "\t                 [PROCESSING]                 "               );
	yInfo( "\t ============================================ "               );
	yInfo( "\t Number of Frequency Bands        : %d",   numBands           );
	yInfo( "\t Number of Full Field Angles      : %d",   numFullFieldAngles );
	yInfo( " " );
	yInfo( "\t                  [Power Map]                 "               );
	yInfo( "\t ============================================ "               );
	yInfo( "\t Number of Power Maps Stored      : %d",   bufferSize         );
	yInfo( " " );

	
	return true;
}


bool AudioPowerMapPeriodicThread::threadInit() {

	/* ===========================================================================
	 *  Initialize all ports. If any fail to open, return false to 
	 *    let RFModule know initialization was unsuccessful.
	 * =========================================================================== */
	
	if (!inBandPowerPort.open(getName("/bandPower:i").c_str())) {
		yError("Unable to open port for receiving the instantaneous band power from the preprocessor module.");
		return false;
	}

	if (!inProbabilityMapPort.open(getName("/probabilityMap:i").c_str())) {
		yError("Unable to open port for receiving a probability map from the bayesian module.");
		return false;
	}

	if (!outProbabilityPowerPort.open(getName("/probabilityPower:o").c_str())) {
		yError("Unable to open port for sending the probability of power in the auditory environment.");
		return false;
	}

	if (!outProbabilityPowerMapPort.open(getName("/probabilityPowerMap:o").c_str())) {
		yError("Unable to open port for sending the combined probability power and bayesian probability map.");
		return false;
	}

	if (!outProbabilityPowerAnglePort.open(getName("/probabilityPowerAngle:o").c_str())) {
		yError("Unable to open port for sending the collapsed, combined probability power and bayesian probability map.");
		return false;
	}

	if (!outInstantaneousPowerProbabilityMapPort.open(getName("/instantaneousPowerProbabilityMap:o").c_str())) {
		yError("Unable to open port for sending the combined instantaneous power and bayesian probability map.");
		return false;
	}

	if (!outInstantaneousPowerProbabilityAnglePort.open(getName("/instantaneousPowerProbabilityAngle:o").c_str())) {
		yError("Unable to open port for sending the collapsed, combined probability power and bayesian probability map.");
		return false;
	}

	stopTime = yarp::os::Time::now();
	yInfo("Initialization of the processing thread correctly ended. Elapsed Time: %f.", stopTime - startTime);
	startTime = stopTime;
	
	return true;
}


void AudioPowerMapPeriodicThread::threadRelease() {

	//-- Stop all threads.
	inBandPowerPort.interrupt();
	inProbabilityMapPort.interrupt();
	outProbabilityPowerPort.interrupt();
	outProbabilityPowerMapPort.interrupt();
	outProbabilityPowerAnglePort.interrupt();
	outInstantaneousPowerProbabilityMapPort.interrupt();
	outInstantaneousPowerProbabilityAnglePort.interrupt();

	//-- Close the threads.
	inBandPowerPort.close();
	inProbabilityMapPort.close();
	outProbabilityPowerPort.close();
	outProbabilityPowerMapPort.close();
	outProbabilityPowerAnglePort.close();
	outInstantaneousPowerProbabilityMapPort.close();
	outInstantaneousPowerProbabilityAnglePort.close();

	//-- Print thread stats.
	endOfProcessingStats();	
}


void AudioPowerMapPeriodicThread::setName(std::string str) {
	this->name=str;
}


std::string AudioPowerMapPeriodicThread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void AudioPowerMapPeriodicThread::setInputPortName(std::string InpPort) {
	//-- Do nothing.
}


void AudioPowerMapPeriodicThread::run() {    
	
	if (inBandPowerPort.getInputCount()) {

		//-- Grab the time time difference of waiting between loops.
		stopTime    = yarp::os::Time::now();
		timeDelay   = stopTime - startTime;
		totalDelay += timeDelay;
		startTime   = stopTime;


		//-- Get Input.
		inputBandPower = *inBandPowerPort.read(true);
		inBandPowerPort.getEnvelope(timeStamp);

		if (inProbabilityMapPort.getInputCount()) {
			ProbabilityMapMatrix = *inProbabilityMapPort.read(true);
			inProbabilityMapPort.getEnvelope(timeStamp);
		}


		//-- Grab the time difference of reading input.
		stopTime      = yarp::os::Time::now();
		timeReading   = stopTime - startTime;
		totalReading += timeReading;
		startTime     = stopTime;


		//-- Main Loop.
		result = processing();


		//-- Grab the time difference of processing the input.
		stopTime         = yarp::os::Time::now();
		timeProcessing   = stopTime - startTime;
		totalProcessing += timeProcessing;
		startTime        = stopTime;


		//-- Write data to outgoing ports.
		publishOutPorts();


		//-- Grab the time delay of publishing on ports.
		stopTime           = yarp::os::Time::now();
		timeTransmission   = stopTime - startTime;
		totalTransmission += timeTransmission;
		startTime          = stopTime;


		//-- Give time stats to the user.
		timeTotal  = timeDelay + timeReading + timeProcessing + timeTransmission;
		totalTime += timeTotal;
		totalIterations++;
		yInfo("End of Loop %d:  Delay  %f  |  Reading  %f  |  Processing  %f  |  Transmission  %f  |  Total  %f  |", timeStamp.getCount(), timeDelay, timeReading, timeProcessing, timeTransmission, timeTotal);
	}
}


bool AudioPowerMapPeriodicThread::processing() {

	/* ===========================================================================
	 *  The input received for power can either be from the gammatone filter
	 *    bank or from the reduced beam former. If it is from the former, the 
	 *    power is separated into left and right channels. Take the average between
	 *    the two channels, and store them.
	 * =========================================================================== */
	BandPowerMatrix.resize(numBands, 1);
	BandPowerMatrix.zero();
	for (int band = 0; band < numBands; band++) {
		for (int mic = 0; mic < numMics; mic++) {
			BandPowerMatrix[band][0] += inputBandPower[band][mic];
		}
		BandPowerMatrix[band][0] /= (double) numMics;
	}


	/* ===========================================================================
	 *  Update the knowledge state given the new information from the received 
	 *    map. Store this map in the buffer, so that later this map can be
	 *    ``forgotten`` from the knowledge. If the specified buffer limit has
	 *    met, the oldest information will be removed.
	 * =========================================================================== */
	updateBayesianProbabilities (
		/* Target = */ ProbabilityPowerMatrix,
		/* Buffer = */ bufferedPowerMatrix,
		/* Length = */ bufferSize,
		/* Source = */ BandPowerMatrix
	);


	/* ===========================================================================
	 *  If the module has a connection to the bayesian map and a 
	 *    port is connected for providing output, combine the beliefs.
	 * =========================================================================== */
	if (inProbabilityMapPort.getInputCount()) {
		
		if (outProbabilityPowerMapPort.getOutputCount() || outProbabilityPowerAnglePort.getOutputCount()) {
		
			combineAudioPower (
				/* Combined   = */ ProbabilityPowerMapMatrix,
				/* AudioMap   = */ ProbabilityMapMatrix,
				/* AudioPower = */ ProbabilityPowerMatrix
			);

			if (outProbabilityPowerAnglePort.getOutputCount()) {
				collapseProbabilityMap (
					/* Target = */ ProbabilityPowerAngleMatrix,
					/* Source = */ ProbabilityPowerMapMatrix
				);
			}
		}


		/* ===========================================================================
		 *  OPTIONAL: If a port is connected, compute the combined 
		 *    instantaneous power with the received bayes map.
		 * =========================================================================== */
		if (outInstantaneousPowerProbabilityMapPort.getOutputCount() || outInstantaneousPowerProbabilityAnglePort.getOutputCount()) {

			combineAudioPower (
				/* Combined   = */ InstantaneousPowerProbabilityMapMatrix,
				/* AudioMap   = */ ProbabilityMapMatrix,
				/* AudioPower = */ BandPowerMatrix
			);

			if (outInstantaneousPowerProbabilityAnglePort.getOutputCount()) {
				collapseProbabilityMap (
					/* Target = */ InstantaneousPowerProbabilityAngleMatrix,
					/* Source = */ InstantaneousPowerProbabilityMapMatrix
				);
			}
		}
	}

	return true;
}


void AudioPowerMapPeriodicThread::updateBayesianProbabilities(yarp::sig::Matrix& ProbabilityPower, std::queue< yarp::sig::Matrix >& BufferedPower, const int BufferLength, const yarp::sig::Matrix& CurrentPower) {
	
	//-- Add the current power map to the buffer.
	BufferedPower.push(CurrentPower);

	//-- Add the current power map to the state knowledge.
	addAudioPower(ProbabilityPower, CurrentPower);

	//-- If the buffer is full, begin to remove power maps.
	if (BufferedPower.size() >= BufferLength) {

		//-- Remove a Power Event.
		removeAudioPower(ProbabilityPower, BufferedPower.front());

		//-- Get rid of it.
		BufferedPower.pop();
	}

	//-- Normalize the resulting probability map.
	normaliseFull(ProbabilityPower);
}


void AudioPowerMapPeriodicThread::addAudioPower(yarp::sig::Matrix& ProbabilityPower, const yarp::sig::Matrix& CurrentPower) {

	//-- Add the new power to the knowledge state.
	int columnLength = ProbabilityPower.cols();

	for (int band = 0; band < numBands; band++) {
		for (int column = 0; column < columnLength; column++) {
			ProbabilityPower[band][column] *= CurrentPower[band][column];
		}
	}
}


void AudioPowerMapPeriodicThread::removeAudioPower(yarp::sig::Matrix& ProbabilityPower, const yarp::sig::Matrix& AntiquatedPower) {

	//-- Remove the old audio from the knowledge state.
	int columnLength = ProbabilityPower.cols();

	for (int band = 0; band < numBands; band++) {
		for (int column = 0; column < columnLength; column++) {
			ProbabilityPower[band][column] /= AntiquatedPower[band][column];
		}
	}
}


void AudioPowerMapPeriodicThread::normaliseMatrix(yarp::sig::Matrix& matrix) {

	//-- Normalise each row of a yarp matrix.
	int RowLength    = matrix.rows();
	int ColumnLength = matrix.cols();

	for (int row = 0; row < RowLength; row++) {
		normaliseRow(matrix[row], ColumnLength);
	}
}


void AudioPowerMapPeriodicThread::normaliseRow(double *MatrixRow, const int Length) {

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


void AudioPowerMapPeriodicThread::normaliseFull(yarp::sig::Matrix& matrix) {

	//-- Normalise based on the sum of all rows.
	int RowLength    = matrix.rows();
	int ColumnLength = matrix.cols();

	double sum = 0.0;
	for (int row = 0; row < RowLength; row++) {
		for (int col = 0; col < ColumnLength; col++) {
			sum += matrix[row][col];
		}
	}

	for (int row = 0; row < RowLength; row++) {
		for (int col = 0; col < ColumnLength; col++) {
			matrix[row][col] /= sum;
		}
	}
}


void AudioPowerMapPeriodicThread::combineAudioPower(yarp::sig::Matrix& CombinedAudioPower, const yarp::sig::Matrix& AudioMap, const yarp::sig::Matrix& AudioPower) {

	//-- Multiply the band power by positions in the audio map.
	CombinedAudioPower.resize(numBands, numFullFieldAngles);

	double currentPower = 0.0;

	for (int band = 0; band < numBands; band++) {
		
		currentPower = AudioPower[band][0];

		for (int angle = 0; angle < numFullFieldAngles; angle++) {
			CombinedAudioPower[band][angle] = AudioMap[band][angle] * currentPower;
		}
	}

	//normaliseMatrix(CombinedAudioPower);
}

void AudioPowerMapPeriodicThread::collapseProbabilityMap(yarp::sig::Matrix& ProbabilityAngles, const yarp::sig::Matrix& ProbabilityMap) {
	
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


void AudioPowerMapPeriodicThread::clearProbabilities() {

	//-- Reset running probabilities.
	ProbabilityPowerMatrix.resize(numBands, 1);
	ones(ProbabilityPowerMatrix);   //-- Set all positions to one.
	
	//-- Clear out the buffer.
	while (!bufferedPowerMatrix.empty()) {
		bufferedPowerMatrix.pop();
	}
}



void AudioPowerMapPeriodicThread::publishOutPorts() {
	
	//-- Write to Active Ports.
	if (outProbabilityPowerPort.getOutputCount()) {
		outProbabilityPowerPort.prepare() = ProbabilityPowerMatrix;
		outProbabilityPowerPort.setEnvelope(timeStamp);
		outProbabilityPowerPort.write();
	}

	if (outProbabilityPowerMapPort.getOutputCount()) {
		outProbabilityPowerMapPort.prepare() = ProbabilityPowerMapMatrix;
		outProbabilityPowerMapPort.setEnvelope(timeStamp);
		outProbabilityPowerMapPort.write();
	}

	if (outProbabilityPowerAnglePort.getOutputCount()) {
		outProbabilityPowerAnglePort.prepare() = ProbabilityPowerAngleMatrix;
		outProbabilityPowerAnglePort.setEnvelope(timeStamp);
		outProbabilityPowerAnglePort.write();
	}

	if (outInstantaneousPowerProbabilityMapPort.getOutputCount()) {
		outInstantaneousPowerProbabilityMapPort.prepare() = InstantaneousPowerProbabilityMapMatrix;
		outInstantaneousPowerProbabilityMapPort.setEnvelope(timeStamp);
		outInstantaneousPowerProbabilityMapPort.write();
	}

	if (outInstantaneousPowerProbabilityAnglePort.getOutputCount()) {
		outInstantaneousPowerProbabilityAnglePort.prepare() = InstantaneousPowerProbabilityAngleMatrix;
		outInstantaneousPowerProbabilityAnglePort.setEnvelope(timeStamp);
		outInstantaneousPowerProbabilityAnglePort.write();
	}
}


void AudioPowerMapPeriodicThread::endOfProcessingStats() {

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
