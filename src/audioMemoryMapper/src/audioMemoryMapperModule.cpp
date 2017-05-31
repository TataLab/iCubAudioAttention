// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Marko Ilievski
  * email: m.ilievski@uleth.ca, matthew.tata@uleth.ca, francesco.rea@iit.it
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
 * @file audioMemoryMapperModule.cpp
 * @brief Implementation of the processing module
 */

#include "audioMemoryMapperModule.h"


audioMemoryMapperModule::audioMemoryMapperModule()
{
    yDebug("AudioProcesserModule");

}

audioMemoryMapperModule::~audioMemoryMapperModule()
{

}

bool audioMemoryMapperModule::configure(yarp::os::ResourceFinder &rf)
{
   
}

double audioMemoryMapperModule::getPeriod()
{
	// TODO Should this all ways stay this low
	return 0.05;
}

bool audioMemoryMapperModule::updateModule()
{


}

bool audioMemoryMapperModule::interruptModule()
{
	fprintf(stderr, "[WARN] Interrupting\n");
	return true;
}

bool audioMemoryMapperModule::close()
{
	fprintf(stderr, "[INFO] Calling close\n");
	return true;
}

