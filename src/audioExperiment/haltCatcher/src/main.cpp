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
 * @brief main code for catching when the experiments halt.
 * =========================================================================== */

#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <time.h>

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/Log.h>

int main(int argc, char * argv[]) {
	
    //-- Init Yarp Network.
	yarp::os::Network yarp;

    //-- Set the start time for the process.
    double firstStartTime = yarp::os::Time::now();


    /* ===========================================================================
     *  Get variables for this module from the resource finder.
     * =========================================================================== */
	yarp::os::ResourceFinder rf;
	rf.setVerbose(true);
	rf.setDefaultConfigFile("Audio_Experiment_Config.ini"); // overridden by --from parameter
	rf.setDefaultContext("audio_experiment");               // overridden by --context parameter
	rf.configure(argc, argv);  
    
    std::string moduleName;
    double waitTime;

    moduleName = rf.check("name",     yarp::os::Value("/haltCatcher"), "module name (string)").asString();
    waitTime   = rf.check("waitTime", yarp::os::Value(30.0),           "time to wait before catching halt (double)").asDouble();


    /* ===========================================================================
     *  Open all necessary ports. 
     * =========================================================================== */
    yarp::os::BufferedPort< yarp::os::Bottle > inMonitorPort;
    yarp::os::BufferedPort< yarp::os::Bottle > outMonitorPort;

    if (!inMonitorPort.open( (moduleName + ":i").c_str() )) {
        yError("Unable to open port for receiving the broadcasts to the yarp network.");
        return 0;
    }

    if (!outMonitorPort.open( (moduleName + ":o").c_str() )) {
        yError("Unable to open port for sending notices out when experiments halt or finish.");
        return 0;
    }

    yInfo("Initialization of halt catcher correctly ended. Elapsed Time: %f.", yarp::os::Time::now() - firstStartTime);


    /* ===========================================================================
     *  Main Processing Loop.
     * =========================================================================== */
    double runningTime = yarp::os::Time::now();

    while (true) {

        //-- Don't do any work until everything is connected.
        if (inMonitorPort.getInputCount()  &&
            outMonitorPort.getOutputCount()) {

            yarp::os::Bottle* command = inMonitorPort.read(false);
            if (command != NULL) {

                //-- If the command contains "Trial Complete" reset the 
                //-- timer and wait for the next notification.
                if (command->get(0).asString() == "Trial Complete") {
                    runningTime = yarp::os::Time::now();
                }

                //-- If the command contains "Good Bye!" inform the recipient of
                //-- the monitor that all trials are complete, then close this process.
                else if (command->get(0).asString() == "Good Bye!") {
                    yarp::os::Bottle& reply = outMonitorPort.prepare();
                    reply.clear();
                    reply.addString("send");
                    reply.addString("All Trials Complete.");
                    outMonitorPort.write();
                    break;
                }

            }

            //-- See if we have waited longer than the set waitTime.
            if (yarp::os::Time::now() - runningTime > waitTime) {
                yarp::os::Bottle& reply = outMonitorPort.prepare();
                reply.clear();
                reply.addString("send");
                reply.addString("Experiment Halted!!");
                outMonitorPort.write();
                break;
            }

        } else {
            
            //-- Display a message regarding what connections are missing.
            std::string msg = "Missing Connection to ";
            if (!inMonitorPort.getInputCount())   { msg += "Input Port; ";  }
            if (!outMonitorPort.getOutputCount()) { msg += "Output Port; "; }
            msg += ". . .";
            yInfo("%s", msg.c_str());

            //-- Sleep for one second.
            usleep(1000000);

            //-- Update running time if we have not been connected.
            runningTime = yarp::os::Time::now();
        }
    }

    //-- Tell the recipient we are closing everything.
    yarp::os::Bottle& reply = outMonitorPort.prepare();
    reply.clear();
    reply.addString("quit");
    outMonitorPort.write();


    /* ===========================================================================
     *  Release all ports.
     * =========================================================================== */
    inMonitorPort.interrupt();
    outMonitorPort.interrupt();

    inMonitorPort.close();
    outMonitorPort.close();


    /* ===========================================================================
     *  Display stats.
     * =========================================================================== */
    yInfo(" ");
    yInfo("End of Thread . . . ");
    yInfo(" ");
    yInfo("\t Total Time : %.2f", yarp::os::Time::now() - firstStartTime);
    yInfo(" ");


	return 0;
}