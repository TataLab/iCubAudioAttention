// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Francesco Rea
  * email: francesco.rea@iit.it
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
 * @file soundMonitorModule.cpp
 * @brief Implementation of the soundMonitorModule (see header file).
 */

#include "iCub/soundMonitorModule.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

/* 
 * Configure method. Receive a previously initialized
 * resource finder object. Use it to configure your module.
 * If you are migrating from the old Module, this is the 
 *  equivalent of the "open" method.
 */

bool soundMonitorModule::configure(yarp::os::ResourceFinder &rf) {
    /* Process all parameters from both command-line and .ini file */

    /* get the module name which will form the stem of all module port names */
    moduleName            = rf.check("name", 
                           Value("/soundMonitor"), 
                           "module name (string)").asString();
    /*
    * before continuing, set the module name before getting any other parameters, 
    * specifically the port names which are dependent on the module name
    */
    setName(moduleName.c_str());

    /*
    * get the robot name which will form the stem of the robot ports names
    * and append the specific part and device required
    */
    robotName             = rf.check("robot", 
                           Value("icub"), 
                           "Robot name (string)").asString();
    robotPortName         = "/" + robotName + "/head";

    inputPortName           = rf.check("inputPortName",
			                Value(":i"),
                            "Input port name (string)").asString();
    
    
    /*
    * attach a port of the same name as the module (prefixed with a /) to the module
    * so that messages received from the port are redirected to the respond method
    */
    handlerPortName =  "";
    handlerPortName += getName();         // use getName() rather than a literal 

    if (!handlerPort.open(handlerPortName.c_str())) {           
        cout << getName() << ": Unable to open port " << handlerPortName << endl;  
        return false;
    }

    attach(handlerPort);                  // attach to port
    if (rf.check("config")) {
        configFile=rf.findFile(rf.find("config").asString().c_str());
        if (configFile=="") {
            return false;
        }
    }
    else {
        configFile.clear();
    }


    /* create the thread and pass pointers to the module parameters */
    rThread = new soundMonitorRatethread(robotName, configFile);
    //rThread->setInputPortName(inputPortName.c_str());
    rThread->setName(moduleName);
    
    /* now start the thread to do the work */
    bool ret = rThread->start(); // this calls threadInit() and it if returns true, it then calls run()

    return ret;       // let the RFModule know everything went well
                        // so that it will then run the module
}

bool soundMonitorModule::interruptModule() {
    handlerPort.interrupt();
    return true;
}

bool soundMonitorModule::close() {
    handlerPort.close();
    /* stop the thread */
    yDebug("stopping the thread \n");
    rThread->stop();
    return true;
}

bool soundMonitorModule::respond(const Bottle& command, Bottle& reply) 
{
    bool ok = false;
    bool rec = false; // is the command recognized?
    string helpMessage =  string(getName().c_str()) + 
                " commands are: \n" +  
                "help \n" +
                "quit \n";
    reply.clear(); 

    if (command.get(0).asString()=="quit") {
        reply.addString("quitting");
        return false;     
    }
    else if (command.get(0).asString()=="help") {
        cout << helpMessage;
        reply.addString("ok");
    }
    mutex.wait();
    switch (command.get(0).asVocab()) {
    case COMMAND_VOCAB_HELP:
        rec = true;
        {
            reply.addVocab(Vocab::encode("many"));
            reply.addString("help");

            //reply.addString();
            reply.addString("set fn \t: general set command ");
            reply.addString("get fn \t: general get command ");
            //reply.addString();

            
            //reply.addString();
            reply.addString("seek red \t : looking for a red color object");
            reply.addString("seek rgb \t : looking for a general color object");
            reply.addString("sus  \t : suspending");
            reply.addString("res  \t : resuming");
            //reply.addString();


            ok = true;
        }
        break;
    case COMMAND_VOCAB_SUSPEND:
        rec = true;
        {
            //prioritiser->suspend();
            ok = true;
        }
        break;
    case COMMAND_VOCAB_STOP:
        rec = true;
        {
            //prioritiser->suspend();
            //prioritiser->resume();
            ok = true;
        }
        break;
    case COMMAND_VOCAB_RESUME:
    rec = true;
        {
            //prioritiser->resume();
            ok = true;
        }
        break;
    case COMMAND_VOCAB_SBEG:
        rec = true;
        {
            yInfo("Start saving the Sound");
            //prioritiser->suspend();
            //prioritiser->seek(command);
            //prioritiser->resume();
            ok = true;
        }
        break;
    case COMMAND_VOCAB_SEND:
        rec = true;
        {
            /*switch (command.get(1).asVocab()) {
            case COMMAND_VOCAB_CENT:
                {
                    printf("Fixating in Center \n");
                    //prioritiser->fixCenter(1000);
                }
                break;
            }
            */
            yInfo("Stop saving the Sound");
            ok = true;
        }
        break;
    default: {
                
    }
        break;    
    }
    mutex.post();

    if (!rec)
        ok = RFModule::respond(command,reply);
    
    if (!ok) {
        reply.clear();
        reply.addVocab(COMMAND_VOCAB_FAILED);
    }
    else
        reply.addVocab(COMMAND_VOCAB_OK);
    return true;
}

/* Called periodically every getPeriod() seconds */
bool soundMonitorModule::updateModule()
{
    return true;
}

double soundMonitorModule::getPeriod()
{
    /* module periodicity (seconds), called implicitly by myModule */
    return 1;
}

