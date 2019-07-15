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
 * @brief main code for testing the preprocessor.
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

#include <iCub/util/audioUtil.h>

int main(int argc, char * argv[]) {
	
    //-- Init Yarp Network.
	yarp::os::Network yarp;
    double firstStartTime = yarp::os::Time::now();

    /* ===========================================================================
     *  Get variables for this module from the resource finder.
     * =========================================================================== */
	yarp::os::ResourceFinder rf;
	rf.setVerbose(true);
	rf.setDefaultConfigFile("preprocessor_test_config.ini"); // overridden by --from parameter
	rf.setDefaultContext("preprocessor_test");               // overridden by --context parameter
	rf.configure(argc, argv);  
    
    //-- General.
    std::string moduleName;
    moduleName = rf.check("name", yarp::os::Value("/preprocessor_test"), "module name (string)").asString();

    //-- Sampling.
    int samplingRate, numFrameSamples;
    samplingRate     = rf.findGroup("sampling").check("samplingRate",     yarp::os::Value(48000),  "Frame samples (int)"                    ).asInt();
    numFrameSamples  = rf.findGroup("sampling").check("numFrameSamples",  yarp::os::Value(4096),   "Sampling rate of mics (int)"            ).asInt();
    
    //-- Testing.
    std::string readFrom, saveTo, connectTo, connectFrom;
    readFrom    = rf.findGroup("testing").check("readFrom",    yarp::os::Value("./in.wav"),    "wav file to read from (string)"    ).asString();
    saveTo      = rf.findGroup("testing").check("saveTo",      yarp::os::Value("./out.data"),  "raw data file to write to (string)").asString();
    connectTo   = rf.findGroup("testing").check("connectTo",   yarp::os::Value("/rawAudio:i"), "port to send wav file to (string)" ).asString();
    connectFrom = rf.findGroup("testing").check("connectFrom", yarp::os::Value("/port:o"),     "port to read data from (string)"   ).asString();

    //-- Expand environmental variables if they're present.
    if (readFrom[0] == '$') { readFrom = AudioUtil::expandEnvironmentVariables(readFrom); }
    if (saveTo[0]   == '$') { saveTo   = AudioUtil::expandEnvironmentVariables(saveTo);   }


    /* =========================================================================== 
	 *  Print the resulting variables to the console.
	 * =========================================================================== */
	yInfo( " " );
	yInfo( "\t                  [SAMPLING]                  "                    );
	yInfo( "\t ============================================ "                    );
	yInfo( "\t Number Samples per Frame         : %d",       numFrameSamples     );
	yInfo( "\t Sampling Rate                    : %d",       samplingRate        );
    yInfo( " " );
    yInfo( "\t                   [TESTING]                  "                    );
	yInfo( "\t ============================================ "                    );
	yInfo( "\t Reading From File                : %s",       readFrom.c_str()    );
    yInfo( "\t Write To File                    : %s",       saveTo.c_str()      );
    yInfo( "\t Send Read Wav File To            : %s",       connectTo.c_str()   );
    yInfo( "\t Receive Yarp Matrix From         : %s",       connectFrom.c_str() );
    yInfo( " " );


    /* ===========================================================================
     *  Open all necessary ports. 
     * =========================================================================== */
    yarp::os::BufferedPort< yarp::sig::Sound  > outTestPort;
    yarp::os::BufferedPort< yarp::sig::Matrix > inTestPort;

    if (!outTestPort.open( (moduleName + ":o").c_str() )) {
        yError("Unable to open port for sending audio to the yarp network.");
        return 0;
    }

    if (!inTestPort.open( (moduleName + ":i").c_str() )) {
        yError("Unable to open port for receiving the processed yarp matrix.");
        return 0;
    }

    yInfo("Initialization of preprocessor_test correctly ended. Elapsed Time: %f.", yarp::os::Time::now() - firstStartTime);


    /* ===========================================================================
     *  Read in the specified wav file. 
     * =========================================================================== */
    yarp::sig::Sound source;
    if (!yarp::sig::file::read(source, readFrom.c_str())) {
        yError("Unable to open specified file %s!!", readFrom.c_str());
        return 0;
    }

    yInfo( " " );
    yInfo(" \t   %s",                                        readFrom.c_str()      );
    yInfo(" \t ============================================ "                      );
    yInfo(" \t Sampling Rate                    : %d",       source.getFrequency() );
    yInfo(" \t Number of Samples                : %ld",      source.getSamples()   );
    yInfo(" \t Number of Channels               : %ld",      source.getChannels()  );
    yInfo(" \t Bytes Per Sample                 : %ld",      source.getBytesPerSample());
    yInfo( " " );


    /* ===========================================================================
     *  Connect the ports. 
     * =========================================================================== */
    yInfo("Attempting to make connections . . .");
    
    if (!yarp::os::Network::connect((moduleName + ":o").c_str(), connectTo.c_str())) {
        yError("Could not make a connection from %s to %s", (moduleName + ":o").c_str(), connectTo.c_str());
        return 0;
    }

    if (!yarp::os::Network::connect(connectFrom.c_str(), (moduleName + ":i").c_str())) {
        yError("Could not make a connection from %s to %s", connectFrom.c_str(), (moduleName + ":i").c_str());
        return 0;
    }


    /* ===========================================================================
     *  Send the data and wait for a response.
     * =========================================================================== */
    yarp::sig::Sound& outRawAudio = outTestPort.prepare();
    outRawAudio = source.subSound(0, numFrameSamples);
    outTestPort.write();
    yInfo("Wrote to port . . . Waiting for response.");

    yarp::sig::Matrix* inProcessedMatrix = inTestPort.read();
    yInfo("Got a response! Saving to file . . .");
    AudioUtil::MatrixToFile(*inProcessedMatrix, saveTo);
    

    /* ===========================================================================
     *  Release all ports.
     * =========================================================================== */
    outTestPort.interrupt();
    inTestPort.interrupt();

    outTestPort.close();
    inTestPort.close();


    /* ===========================================================================
     *  Display stats.
     * =========================================================================== */
    yInfo(" ");
    yInfo("End of Test . . . ");
    yInfo(" ");
    yInfo("\t Total Time : %.2f", yarp::os::Time::now() - firstStartTime);
    yInfo(" ");


	return 0;
}