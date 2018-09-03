// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2018  Department of Neuroscience - University of Lethbridge
  * Author: Austin Kothig, Matt Tata
  * email: kothiga@uleth.ca matthew.tata@uleth.ca
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
 * @file audioPowerMapModule.cpp
 * @brief Implementation of the power map module.
 */

#include "audioPowerMapModule.h"

using namespace yarp::os;

bool AudioPowerMapModule::configure(yarp::os::ResourceFinder &rf) {

	yInfo("Configuring the module");

	// get the module name which will form the stem of all module port names 
	moduleName = rf.check("name", Value("/AudioPowerMap"), "module name (string)").asString();


	// before continuing, set the module name before getting any other parameters, 
	// specifically the port names which are dependent on the module name
	setName(moduleName.c_str());


	// get the robot name which will form the stem of the robot ports names
	// and append the specific part and device required
	robotName     = rf.check("robot", Value("icub"), "Robot name (string)").asString();
	robotPortName =  "/" + robotName + "/head";
	inputPortName =  rf.check("inputPortName", Value(":i"),	"Input port name (string)").asString();


	// attach a port of the same name as the module (prefixed with a /) to the module
	// so that messages received from the port are redirected to the respond method
	handlerPortName =  "";
	handlerPortName += getName();         

	if (!handlerPort.open(handlerPortName.c_str())) {           
		std::cout << getName() << ": Unable to open port " << handlerPortName << std::endl;  
		return false;
	}

	// attach to port
	attach(handlerPort);                  
	if (rf.check("config")) {
		configFile=rf.findFile(rf.find("config").asString().c_str());
		if (configFile=="") {
			return false;
		}
	}

	else {
		configFile.clear();
	}

	// create the thread and pass pointers to the module parameters
	apr = new AudioPowerMapRatethread(robotName, configFile, rf);
	apr->setName(moduleName);

	// now start the thread to do the work 
	// this calls threadInit() and it if returns true, it then calls run()
	bool ret = apr->start(); 

	// let the RFModule know everything went well
	// so that it will then run the module
	return ret;         
}


bool AudioPowerMapModule::interruptModule() {
	yInfo("Interrupting\n");
	handlerPort.interrupt();
	return true;
}


bool AudioPowerMapModule::close() {
	yInfo("Calling close\n");
	handlerPort.close();
	yDebug("stopping the thread \n");
	apr->stop();
	return true;
}


double AudioPowerMapModule::getPeriod() {
	return 0.05; 
}


bool AudioPowerMapModule::updateModule() {
	return true;
}