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
 * @file  audioBayesianMapPeriodicThread.h
 * @brief Definition of a periodic thread that receives an allocentric map
 *          of the auditory environment, and builds a Bayesian map over time.
 * =========================================================================== */

#ifndef _AUDIO_BAYESIAN_MAP_PERIODICTHREAD_H_
#define _AUDIO_BAYESIAN_MAP_PERIODICTHREAD_H_

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

#include <iCub/util/audioUtil.h>

typedef yarp::sig::Matrix                 yMatrix;
typedef yarp::os::BufferedPort< yMatrix > yMatrixBuffer;

class AudioBayesianMapPeriodicThread : public yarp::os::PeriodicThread {

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
	yMatrixBuffer inAllocentricAudioPort;
	yMatrixBuffer outProbabilityMapPort;
	yMatrixBuffer outProbabilityAnglePort;

	/* ===========================================================================
	 *  Yarp Matrices used for Modules Computation. 
	 *    Objects passed around to encapsulated objects.
	 * =========================================================================== */
	yMatrix AllocentricAudioMatrix;
	yMatrix ProbabilityMapMatrix;
	yMatrix ProbabilityAngleMatrix;

	/* ===========================================================================
	 *  Buffer for ``remembering`` some number of states.
	 * =========================================================================== */
	std::queue< yMatrix > bufferedAudioMatrix;

	/* ===========================================================================
	 *  Variables received from the resource finder.
	 * =========================================================================== */
	int numBands;
	int angleRes;
	int bufferSize;

	std::string saveMatrices;

	/* ===========================================================================
	 *  Derive variables from resource finders variables.
	 * =========================================================================== */
	int  numFullFieldAngles;
	bool processAll;

	/* ===========================================================================
	 *  Constant variables.
	 * =========================================================================== */
	const int _baseAngles = 180;           //-- Base number of angles in front field.


  public:

	/* ===========================================================================
	 *  Default Constructor.
	 * =========================================================================== */
	AudioBayesianMapPeriodicThread();


	/* ===========================================================================
	 *  Main Constructor.
	 * 
	 * @param robotname  : Name of the robot.
	 * @param configFile : Path to the .ini configuration file.
	 * =========================================================================== */
	AudioBayesianMapPeriodicThread(std::string robotname, std::string configFile);


	/* ===========================================================================
	 *  Destructor.
	 * =========================================================================== */
	~AudioBayesianMapPeriodicThread();


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


	/* ===========================================================================
	 *  Flush the buffer and reset the Knowledge state to its initial values.
	 *    For interfacing with RPC port.
	 * =========================================================================== */
	void clearProbabilities();


  private:

	/* ===========================================================================
	 *  Does it all. Performs a bayesian update on the probability map using
	 *    the new audio map provided. Afterwards the new map will be inserted 
	 *    into the audio buffer. If the buffer is at it's specified capacity
	 *    the oldest item will be removed from the knowledge state, and
	 *    discarded from the buffer.
	 * 
	 * @param CurrentAudio   : An allocentric audio map freshly recieved from the audio preprocessor (Number of Bands, Number of Full Field Angles).
	 * @param ProbabilityMap : The target, and source for updating the knowledge state (Number of Bands, Number of Full Field Angles).
	 * @param BufferedAudio  : A buffer containing recent audio maps. Oldest maps are used against the knowledge state.
	 * @param BufferLength   : Used to ensure the number of samples kept is consistent.
	 * =========================================================================== */
	void updateBayesianProbabilities(const yMatrix& CurrentAudio, yMatrix& ProbabilityMap, std::queue< yMatrix >& BufferedAudio, const int BufferLength);


	/* ===========================================================================
	 *  Multiplies the Current Audio with the Probability Map.
	 * 
	 * @param CurrentAudio   : New information of the auditory environment.
	 * @param ProbabilityMap : Knowledge state of the auditory environment.
	 * =========================================================================== */
	void addAudioMap(const yMatrix& CurrentAudio, yMatrix& ProbabilityMap);


	/* ===========================================================================
	 *  Divides the Antiquated Audio from the Probability Map.
	 * 
	 * @param AntiquatedAudio : Old information of the auditory environment.
	 * @param ProbabilityMap  : Knowledge state of the auditory environment.
	 * =========================================================================== */
	void removeAudioMap(const yMatrix& AntiquatedAudio, yMatrix& ProbabilityMap);
	

	/* ===========================================================================
	 *  Normalises each row of the matrix to sum to one.
	 * 
	 * @param matrix : matrix to be normalised.
	 * =========================================================================== */
	void normaliseMatrix(yMatrix& matrix);


	/* ===========================================================================
	 *  Normalises a single row of a matrix to sum to one.
	 * 
	 * @param MatrixRow : Specified Row.
	 * @param Length    : Length of the row.
	 * =========================================================================== */
	void normaliseRow(double *MatrixRow, const int Length);


	/* ===========================================================================
	 *  Collapse a Probability Map across the bands to get the overall 
	 *    probability at each angle of the knowledge state.
	 * 
	 * @param ProbabilityMap    : Knowledge state of the auditory environment.
	 * @param ProbabilityAngles : Angles of Probability.
	 * =========================================================================== */
	void collapseProbabilityMap(const yMatrix& ProbabilityMap, yMatrix& ProbabilityAngles);


	/* ===========================================================================
	 *  Write data to out going ports if something is connected.
	 * =========================================================================== */
	void publishOutPorts();


	/* ===========================================================================
	 *  Write data to out going ports if something is connected.
	 * =========================================================================== */
	void saveOutPorts();


	/* ===========================================================================
	 *  Produce execution stats when the thread is interrupted.
	 * =========================================================================== */
	void endOfProcessingStats();
};

#endif  //_AUDIO_BAYESIAN_MAP_PERIODICTHREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
