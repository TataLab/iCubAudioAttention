// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2018  Department of Neuroscience - University of Lethbridge
  * Author: Marko Ilievski, Austin Kothig, Matt Tata
  * email: m.ilievski@uleth.ca, kothiga@uleth.ca, matthew.tata@uleth.ca
  * Permission is granted to copy, distribute, and/or modify this program
  * under the terms of the GNU General Public License, version 2 or any
  * later version published by the Free Software Foundation.
  *
  * A copy of the license can be found at
tutorial  * http://www.robotcub.org/icub/license/gpl.txt
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details
*/

/**
 * @file main.cpp
 * @brief main code for the module.
 */

#include <yarp/os/all.h>
#include <yarp/dev/all.h>

#include "audioBayesianModule.h"

using namespace yarp::os;
using namespace yarp::dev;

int main(int argc, char * argv[]) {

	/* initialize yarp network */
	yarp::os::Network yarp;

	yarp::os::ResourceFinder rf;
	//rf.setVerbose(true);
	rf.setDefaultConfigFile("audioConfig.ini");       //overridden by --from parameter
	rf.setDefaultContext("icubAudioAttention");  //overridden by --context parameter
	rf.configure(argc, argv);
	yInfo("Configuring and starting module.\n");

	if (!yarp.checkNetwork(1)) {
		yError("YARP server not available!\n");
		return -1;
	}

	AudioBayesianModule module;
	module.runModule(rf);

	return 0;
}
