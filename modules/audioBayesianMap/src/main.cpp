// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Department of Neuroscience - University of Lethbridge
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
 * @file  main.cpp
 * @brief main code for the Bayesian Map module.
 * =========================================================================== */

#include <iCub/audioBayesianMapModule.h> 

int main(int argc, char * argv[]) {
	
	yarp::os::Network yarp;
	AudioBayesianMapModule module; 

	yarp::os::ResourceFinder rf;
	rf.setVerbose(true);
	rf.setDefaultConfigFile("audio_attention_config.ini"); // overridden by --from parameter
	rf.setDefaultContext("audio_attention");               // overridden by --context parameter
	rf.configure(argc, argv);  
 
	module.runModule(rf);
	return 0;
}