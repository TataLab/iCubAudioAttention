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
 * @file  audioPowerMapPeriodicThread.h
 * @brief Definition of a periodic thread that receives an allocentric map
 *          of the auditory probability, and the instantanious power of the 
 *          environment to build a probability of power over time. These 
 *          two maps are compined.
 * =========================================================================== */

#ifndef _AUDIO_POWER_MAP_PERIODICTHREAD_H_
#define _AUDIO_POWER_MAP_PERIODICTHREAD_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>

#include <queue>

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/os/Log.h>

class AudioPowerMapPeriodicThread : public yarp::os::PeriodicThread {

  private:

	bool result;                //-- Result of the processing.

	std::string name;           //-- Rootname of all the ports opened by this thread.
	std::string robot;          //-- Name of the robot.
	std::string configFile;     //-- Name of the configFile where the parameter of the camera are set.
	std::string inputPortName;  //-- Name of input port for incoming events, typically from aexGrabber.

	yarp::os::Stamp timeStamp;  //-- Time stamp updated by yarp network.

	double startTime;           //-- Used for keeping time and reporting temporal
    double stopTime;            //-- events to the user via command line.
	
	double timeDelay,        totalDelay;        //-- Hold on to and store time
	double timeReading,      totalReading;	    //-- events for clean display
	double timeProcessing,   totalProcessing;   //-- at the end of a loop.
	double timeTransmission, totalTransmission; //-- Include stats on the average execution
	double timeTotal,        totalTime;         //-- when the RFModule is closed.

	int    totalIterations;
	
	/* ===========================================================================
	 *  Yarp Ports for Sending and Receiving Data from this Periodic Thread.
	 * =========================================================================== */
	yarp::os::BufferedPort< yarp::sig::Matrix > inBandPowerPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > inProbabilityMapPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outProbabilityPowerPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outProbabilityPowerMapPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outProbabilityPowerAnglePort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outInstantaneousPowerProbabilityMapPort;
	yarp::os::BufferedPort< yarp::sig::Matrix > outInstantaneousPowerProbabilityAnglePort;

	/* ===========================================================================
	 *  Yarp Matrices used for Modules Computation. 
	 *    Objects passed around to encapsulated objects.
	 * =========================================================================== */
	yarp::sig::Matrix inputBandPower;
	yarp::sig::Matrix BandPowerMatrix;	
	yarp::sig::Matrix ProbabilityMapMatrix;
	yarp::sig::Matrix ProbabilityPowerMatrix;
	yarp::sig::Matrix ProbabilityPowerMapMatrix;
	yarp::sig::Matrix ProbabilityPowerAngleMatrix;
	yarp::sig::Matrix InstantaneousPowerProbabilityMapMatrix;
	yarp::sig::Matrix InstantaneousPowerProbabilityAngleMatrix;

	/* ===========================================================================
	 *  Buffer for ``remembering`` some number of states.
	 * =========================================================================== */
	std::queue< yarp::sig::Matrix > bufferedPowerMatrix;	

	/* ===========================================================================
	 *  Variables received from the resource finder.
	 * =========================================================================== */
	int numMics;
	int numBands;
	int angleRes;
	int bufferSize;

	/* ===========================================================================
	 *  Derive variables from resource finders variables.
	 * =========================================================================== */
	int numFullFieldAngles;

	/* ===========================================================================
	 *  Constant variables.
	 * =========================================================================== */
	const int _baseAngles = 180;           //-- Base number of angles in front field.


  public:

	/* ===========================================================================
	 *  Default Constructor.
	 * =========================================================================== */
	AudioPowerMapPeriodicThread();


	/* ===========================================================================
	 *  Main Constructor.
	 * 
	 * @param robotname  : Name of the robot.
	 * @param configFile : Path to the .ini configuration file.
	 * =========================================================================== */
	AudioPowerMapPeriodicThread(std::string robotname, std::string configFile);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~AudioPowerMapPeriodicThread();


	/* ===========================================================================
	 *  Configure all the parameters and return true if successful.
	 * 
	 * @param rf : Reference to the resource finder.
	 * 
	 * @return Flag for the success.
	 * =========================================================================== */
	bool configure(yarp::os::ResourceFinder &rf);


	/* ===========================================================================
	 *  Initialises the thread. Build tables and open ports.
	 * =========================================================================== */
	bool threadInit();


	/* ===========================================================================
	 *  Correctly releases the thread.
	 * =========================================================================== */
	void threadRelease();


	/* ===========================================================================
	 *  Function that sets the rootname of all the ports 
	 *    that are going to be created by the thread.
	 * 
	 * @param str : rootname.
	 * =========================================================================== */
	void setName(std::string str);
	

	/* ===========================================================================
	 *  Function that returns the original root name and 
	 *    appends another string iff passed as parameter.
	 * 
	 * @param p : pointer to the string that has to be added.
	 * 
	 * @return rootname + appended string.
	 * =========================================================================== */
	std::string getName(const char* p);


	/* ===========================================================================
	 *  Function that sets the inputPort name.
	 * =========================================================================== */
	void setInputPortName(std::string inpPrtName);


	/* ===========================================================================
	 *  Active part of the thread.
	 * =========================================================================== */
	void run(); 


	/* ===========================================================================
	 *  Method for the processing in the PeriodicThread.
	 * =========================================================================== */
	bool processing();


  private:

	/* ===========================================================================
	 *  Does it all. Performs a bayesian update on the probability power using
	 *    the new audio power provided. Afterwards the new map will be inserted 
	 *    into the power buffer. If the buffer is at it's specified capacity
	 *    the oldest item will be removed from the knowledge state, and
	 *    discarded from the buffer.
	 * 
	 * @param ProbabilityPower : The target, and source for updating the knowledge state (Number of Bands, 1).
	 * @param BufferedPower    : A buffer containing recent audio power. Oldest power are used against the knowledge state.
	 * @param BufferLength     : Used to ensure the number of samples kept is consistent.
	 * @param CurrentPower     : An audio power sample freshly recieved from the audio preprocessor (Number of Bands, 1).
	 * =========================================================================== */
	void updateBayesianProbabilities(yarp::sig::Matrix& ProbabilityPower, std::queue< yarp::sig::Matrix >& BufferedPower, const int BufferLength, const yarp::sig::Matrix& CurrentPower);


	/* ===========================================================================
	 *  Multiplies the Current Power with the Probability Power.
	 * 
	 * @param ProbabilityPower : Knowledge state of the power environment.
	 * @param CurrentPower     : New information of the power environment.
	 * =========================================================================== */
	void addAudioPower(yarp::sig::Matrix& ProbabilityPower, const yarp::sig::Matrix& CurrentPower);


	/* ===========================================================================
	 *  Divides the Antiquated Power from the Probability Power.
	 * 
	 * @param ProbabilityPower : Knowledge state of the power environment.
	 * @param AntiquatedPower  : Old information of the power environment.
	 * =========================================================================== */
	void removeAudioPower(yarp::sig::Matrix& ProbabilityPower, const yarp::sig::Matrix& AntiquatedPower);
	

	/* ===========================================================================
	 *  Normalises each row of the matrix to sum to one.
	 * 
	 * @param matrix : Matrix to be normalised.
	 * =========================================================================== */
	void normaliseMatrix(yarp::sig::Matrix& matrix);


	/* ===========================================================================
	 *  Normalises a single row of a matrix to sum to one.
	 * 
	 * @param MatrixRow : Specified Row.
	 * @param Length    : Length of the row.
	 * =========================================================================== */
	void normaliseRow(double *MatrixRow, const int Length);


	/* ===========================================================================
	 *  Normalises based on the sum of the whole matrix.
	 * 
	 * @param matrix : Matrix to be normalised.
	 * =========================================================================== */
	void normaliseFull(yarp::sig::Matrix& matrix);


	/* ===========================================================================
	 *  Perform an index wise multiplication between the two matracies.
	 * 
	 * @param CombinedAudioPower : The Target, Matrix for the result to be stored (Number of Bands, Number of Full Field Angles).
	 * @param AudioMap           : The Source of the Audio Map (Number of Bands, Number of Full Field Angles).
	 * @param AudioPower         : The Source of the Audio Power (Number of Bands, 1).
	 * =========================================================================== */
	void combineAudioPower(yarp::sig::Matrix& CombinedAudioPower, const yarp::sig::Matrix& AudioMap, const yarp::sig::Matrix& AudioPower);


	/* ===========================================================================
	 *  Collapse a Probability Map across the bands to get the overall 
	 *    probability at each angle of the knowledge state.
	 * 
	 * @param ProbabilityAngles : Angles of Probability.
	 * @param ProbabilityMap    : Knowledge state of the auditory environment.
	 * =========================================================================== */
	void collapseProbabilityMap(yarp::sig::Matrix& ProbabilityAngles, const yarp::sig::Matrix& ProbabilityMap);


	/* ===========================================================================
	 *  Flush the buffer and reset the Knowledge state to its initial values.
	 * =========================================================================== */
	void clearProbabilities();


	/* ===========================================================================
	 *  Write data to out going ports if something is connected.
	 * =========================================================================== */
	void publishOutPorts();


	/* ===========================================================================
	 *  Produce execution stats when the thread is interrupted.
	 * =========================================================================== */
	void endOfProcessingStats();
};

#endif  //_AUDIO_POWER_MAP_PERIODICTHREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
