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
 * @file  audioPreprocessorModule.cpp
 * @brief Implementation of the audioPreprocessorModule (see header file).
 * =========================================================================== */

#include <iCub/audioPreprocessorModule.h>

bool AudioPreprocessorModule::configure(yarp::os::ResourceFinder &rf) {

	/* ===========================================================================
	 *  Get the module name which will form the stem of all module port names.
	 *   Next set the module name before getting any other parameters, 
	 *   specifically the port names which are dependent on the module name.
	 * =========================================================================== */
	moduleName = rf.check("name", yarp::os::Value("/audioPreprocessor"), "module name (string)").asString();
	setName(moduleName.c_str());

	/* ===========================================================================
	 *  Get the robot name which will form the stem of the robot ports names
	 *   and append the specific part and device required.
	 * =========================================================================== */
	robotName = rf.check("robot", yarp::os::Value("icub"), "Robot name (string)").asString();
	robotPortName = "/" + robotName + "/head";
	inputPortName = rf.check("inputPortName", yarp::os::Value(":i"), "Input port name (string)").asString();
	
	/* ===========================================================================
	 *  Attach a port of the same name as the module (prefixed with a /) 
	 *   to the module so that messages received from the port are 
	 *   redirected to the respond method.
	 * =========================================================================== */
	handlerPortName  = "";
	handlerPortName += getName();

	if (!handlerPort.open(handlerPortName.c_str())) {           
		yInfo("%s: Unable to open port %s", getName().c_str(), handlerPortName.c_str());
		return false;
	}

	attach(handlerPort); //-- Attach to port.
	if (rf.check("config")) {
		configFile = rf.findFile(rf.find("config").asString().c_str());
		if (configFile == "") {
			return false;
		}
	} else {
		configFile.clear();
	}

	/* =========================================================================== 
	 *  Create the thread and pass pointers to the module parameters.
	 * =========================================================================== */
	periodicThread = new AudioPreprocessorPeriodicThread(robotName, configFile);
	periodicThread->setName(getName().c_str());
	
	if (!periodicThread->configure(rf)) {
		yInfo("Unable to open Resource Finder for Periodic Thread.");
		return false;
	}
	
	/* ===========================================================================
	 *  Now start the thread to do the work. 
	 * =========================================================================== */
	periodicThread->start();

	//-- Let the RFModule know everything went 
	//-- well so that it will then run the module.
	return true ;     
}


bool AudioPreprocessorModule::interruptModule() {
	handlerPort.interrupt();
	return true;
}


bool AudioPreprocessorModule::close() {

	handlerPort.close();

	//-- Stop the thread.
	yDebug("Stopping the thread . . . \n");
	periodicThread->stop();

	//-- Release the periodic thread.
	delete periodicThread;

	return true;
}


bool AudioPreprocessorModule::respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply) {

	std::string helpMessage = std::string(getName().c_str()) + " commands are: \n" + "help \n" + "quit \n";
	reply.clear(); 

	if (command.get(0).asString() == "quit") {
		reply.addString("quitting");
		return false;
	} else if (command.get(0).asString() == "help") {
		yInfo("%s", helpMessage.c_str());
		reply.addString("ok");
	}
	
	return true;
}


double AudioPreprocessorModule::getPeriod() {
	/* Module periodicity (seconds), called implicitly by myModule. */
	return 1;
}


bool AudioPreprocessorModule::updateModule() {
	/* Called periodically every getPeriod() seconds. */
	return true;
}
