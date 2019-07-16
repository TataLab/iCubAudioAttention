// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Austin Kothig, Francesco Rea
  * email: kothiga@uleth.ca, matthew.tata@uleth.ca, francesco.rea@iit.it
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
 * @file audioAttentionManagerRatethread.cpp
 * @brief Implementation of the eventDriven thread (see audioAttentionManagerRatethread.h).
 */

#include <iCub/audioAttentionManagerRatethread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 100 //ms

audioAttentionManagerRatethread::audioAttentionManagerRatethread():RateThread(THRATE) {
    robot = "icub";        
}

audioAttentionManagerRatethread::audioAttentionManagerRatethread(string _robot, string _configFile) : RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
}

audioAttentionManagerRatethread::~audioAttentionManagerRatethread() {
    delete inputCommand;
    delete inputSpeech;
    delete output;
}

bool audioAttentionManagerRatethread::threadInit() {

    // opening the port for speech input
    if (!inputSpeechPort.open(getName("/speech:i").c_str())) {
        yError("unable to open port to receive input from speech recogn");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    if (!outputPort.open(getName("/heading:o").c_str())) {
        yError(": unable to open port to send unmasked events ");
        return false;  // unable to open; let RFModule know so that it won't run
    }

    // prepare the input/output containers
    inputSpeech = new yarp::os::Bottle;
    output = new yarp::os::Bottle;

    // make sure the current bottle is empty
    current.clear();

    // set our states to empty strings
    speech = "";
    ciaoAwake = false;
    ciaoMove = false;
    ciaoTime = yarp::os::Time::now();

    yInfo("Initialization of the processing thread correctly ended");

    return true;
}

void audioAttentionManagerRatethread::setName(string str) {
    this->name=str;
}


std::string audioAttentionManagerRatethread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void audioAttentionManagerRatethread::setInputPortName(string InpPort) {
    
}

void audioAttentionManagerRatethread::run() {    
    
    //
    // read in from the speech recogn module
    // 
    if (inputSpeechPort.getInputCount()) {
    	// get the bottle
    	inputSpeech = inputSpeechPort.read(false);

    	if (inputSpeech != NULL) {
    		// read in the information from the bottle
    		speech = inputSpeech->toString();
    		yInfo("Message Recieved: %s", speech.c_str());
    	}
    }


    // 
    // add different messages to the output
    // based on our recienved information
    //
    stateTransition();


    //
    // prepare to send output 
    // 
    if (outputPort.getOutputCount()) {

    	// make sure there is something to send
    	if (current.size() != 0) {

    	    // write output onto the network
        	outputPort.write(current);

        	// clear out the contents of current
        	current.clear();
    	}
    }

    // timing how long the module took
	stopTime = yarp::os::Time::now();
	yInfo("elapsed time = %f\n",stopTime - startTime);
	startTime = stopTime;
}


bool audioAttentionManagerRatethread::processing(){
    // here goes the processing...
    return true;
}


void audioAttentionManagerRatethread::threadRelease() {
    // stop all threads
    inputSpeechPort.interrupt();
    outputPort.interrupt();
    
    // release the ports
    inputSpeechPort.close();
    outputPort.close();
}


void audioAttentionManagerRatethread::stateTransition() {

	// if we've heard the trigger
	// stop moving and listen
	if (speech == "ciao icub") {			
		current.addVocab(COMMAND_VOCAB_SUSPEND);	
		ciaoTime = yarp::os::Time::now();
		ciaoAwake = true;
	} 

	// check if the iCub has been "woken up"
	// listen for 5 sec, and then move to position
	if (ciaoAwake) {
	    if (yarp::os::Time::now()-ciaoTime > LISTEN) {
			ciaoMove = true;
		}

		if (ciaoMove) {
			current.addVocab(COMMAND_VOCAB_RESUME);
			ciaoAwake = false;	
			ciaoMove = false;
		}
	}

	// set speech to null
	speech = "";
}