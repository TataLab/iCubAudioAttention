// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Department of Neuroscience - University of Lethbridge
 * Author: Lukas Grasse, Austin Kothig, Francesco Rea, Marko Ilievski, Matt Tata
 * email: lukas.grasse@uleth.ca, kothiga@uleth.ca, francesco.reak@iit.it, marko.ilievski@uwaterloo.ca, matthew.tata@uleth.ca
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
 * @file  rosAudioRemapperModule.h
 * @brief Module for remapping a ROS remoteInterface message to a yarp Sound. 
 * =========================================================================== */

#ifndef _ROS_AUDIO_REMAPPER_MODULE_H_
#define _ROS_AUDIO_REMAPPER_MODULE_H_

#include <iostream>
#include <string>

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Thread.h>
#include <yarp/os/Log.h>

#include <iCub/rosAudioRemapperPeriodicThread.h>

class RosAudioRemapperModule : public yarp::os::RFModule { 

  private:
  
	std::string moduleName;          //-- Name of the module.
	std::string robotName;           //-- Name of the robot .
	std::string inputPortName;       //-- Name of the input port for events.
	std::string outputPortName;      //-- Name of output port.
	std::string handlerPortName;     //-- Name of handler port.
	std::string configFile;          //-- Name of the configFile that the resource Finder will seek.
	
	yarp::os::Port handlerPort;      //-- A port to handle messages.
	
	//-- Pointer to a new thread to be created and 
	//-- started in configure() and stopped in close().
	RosAudioRemapperPeriodicThread *periodicThread; 


  public:

	/* ===========================================================================
	 *  Configure all the parameters and return true if successful.
	 * 
	 * @param rf : Reference to the resource finder.
	 * 
	 * @return Flag for the success.
	 * =========================================================================== */
	bool configure(yarp::os::ResourceFinder &rf); 

   
	/* ===========================================================================
	 *  Interrupt, e.g., the ports. 
	 * =========================================================================== */
	bool interruptModule();                    


	/* ===========================================================================
	 *  Close and shut down the tutorial.
	 * =========================================================================== */
	bool close();


	/* ===========================================================================
	 *  To respond through rpc port.
	 * 
	 * @param command : Command reference to bottle given to rpc port of module, alongwith parameters.
	 * @param reply   : Reply reference to bottle returned by the rpc port in response to command.
	 * 
	 * @return Bool flag for the success of response else termination of module.
	 * =========================================================================== */
	bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);


	/* ===========================================================================
	 *  Unimplemented.
	 * =========================================================================== */
	double getPeriod();


	/* ===========================================================================
	 *  Unimplemented
	 * =========================================================================== */ 
	bool updateModule();
};

#endif // _ROS_AUDIO_REMAPPER_MODULE_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
