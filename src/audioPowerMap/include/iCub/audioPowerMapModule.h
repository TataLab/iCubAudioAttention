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
 * @file audioPowerMapModule.h
 * @brief Module for Power Map.
 */

#ifndef _AUDIO_POWER_MAP_MODULE_H_
#define _AUDIO_POWER_MAP_MODULE_H_

#include <yarp/os/BufferedPort.h>
#include <yarp/os/NetInt32.h>
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Stamp.h>

#include <yarp/sig/Matrix.h>
#include <yarp/sig/Sound.h>
#include <yarp/sig/Vector.h>

#include <audioPowerMapRatethread.h>

class AudioPowerMapModule : public yarp::os::RFModule {

 private:
    std::string moduleName;                  // name of the module
	std::string robotPortName;               // name of robot port
	std::string inputPortName;               // name of the input port for events
	std::string handlerPortName;             // name of handler port
	std::string robotName;                   // name of the robot
	std::string configFile;                  // name of the configFile that the resource Finder will seek
	yarp::os::Port handlerPort;              // a port to handle messages 

	AudioPowerMapRatethread* apr;            // ratethread handling the processing in the module


 public:
	bool configure(yarp::os::ResourceFinder &rf);
	bool interruptModule();
	bool close();
	double getPeriod();
	bool updateModule();
};

#endif //_AUDIO_POWER_MAP_MODULE_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
