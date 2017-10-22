// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Francesco Rea
  * email: francesco.rea@iit.it
  * Permission is granted to copy, distribute, and/or modify this program
  * under the terms of the GNU General Public License, version 2 or any
  * later version published by the Free Software Foundation.
  *
  *

  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details
*/

/**
 * @file headingAudioModule.cpp
 * @brief Implementation of the headingAudioModule (see header file).
 */

#include "iCub/headingAudioModule.h"

using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::os;
//using namespace attention::dictionary;
using namespace std;

/* 
 * Configure method. Receive a previously initialized
 * resource finder object. Use it to configure your module.
 * If you are migrating from the old Module, this is the 
 *  equivalent of the "open" method.
 */

bool headingAudioModule::configure(yarp::os::ResourceFinder &rf) {
    /* Process all parameters from both command-line and .ini file */

    /* get the module name which will form the stem of all module port names */
    moduleName            = rf.check("name", 
                           Value("/headingAudio"), 
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

    inputPortName         = rf.check("inputPortName",
			                Value(":i"),
                            "Input port name (string)").asString();

    int gain              = rf.check("gain",
                            Value(1),
                            "Gain for visualization (int)").asInt();
    std::cerr << "init gain is " << gain << std::endl;
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
    yInfo("successfully parsed config parameter");

    //initializing gazecontrollerclient
    printf("initialising gazeControllerClient \n");
    Property option;
    option.put("device","gazecontrollerclient");
    option.put("remote","/iKinGazeCtrl");
    std::string localCon("/client/gaze");
    localCon.append("simpleSaccade");
    option.put("local",localCon.c_str());

    yarp::dev::PolyDriver* clientGazeCtrl=new PolyDriver();
    clientGazeCtrl->open(option);
    igaze=NULL;

    int originalContext;

    if (clientGazeCtrl->isValid()) {
       clientGazeCtrl->view(igaze);
    }
    else {
        return false;
    }
    igaze->storeContext(&originalContext);
    igaze->blockNeckPitch(0);
    igaze->blockNeckRoll();

    yInfo("Successfully initialised the iKingaze controller");
    
    //------------------------------------------------------------------------
    
    
    //_options.portName+="/command:o";
    std::string portName   = getName("/cmd:o");//"/simpleSaccade/cmd:o";
    std::string portNameIn = getName("/cmd:i"); //"= "/simpleSaccade/cmd:i";
    std::string portAbsoluteIn = getName("/absoluteAngles:i");
    yInfo("created the necessary port names");
    _pOutPort = new Port();
    _pOutPort->open(portName.c_str());
    _pInPort = new BufferedPort<Bottle>();
    _pInPort->open(portNameIn.c_str());
    _pAbsolutePort = new BufferedPort<Matrix>();
    _pAbsolutePort->open(portAbsoluteIn.c_str());

    /*    
    Property params;
    params.fromCommand(argc, argv);
    if(params.check("help"))
    {
        fprintf(stderr, "%s --robot robotName --loop numberOfLoop", argv[0]);
    }
        
    if (!params.check("robot"))
    {
        fprintf(stderr, "Please specify the name of the robot\n");
        fprintf(stderr, "--robot name (e.g. icub)\n");
        return -1;
    }
    if (!params.check("loop"))
    {
        fprintf(stderr, "Please specify the number of repetition\n");
        fprintf(stderr, "--loop number\n");
        return -1;
    }
    */

    /*
    
    //std::string robotName = params.check("robot", Value("icub"), "robotname").asString();
    std::string robotName("icub");
    std::string remotePorts="/";
    remotePorts+=robotName;
    remotePorts+="/head"; //"/right_arm"
    yInfo("trying to connect to %s", remotePorts.c_str());
    //int nOl=atoi(params.find("loop").asString().c_str());
    //int nOl=params.find("loop").asInt();


    std::string localPorts=getName("/remote_controlboard");
    Property options;
    options.put("device", "remote_controlboard");
    options.put("local", localPorts.c_str());      //local port names
    options.put("remote", remotePorts.c_str());    //where we connect to

    // create a device
    PolyDriver robotDevice(options);
    if (!robotDevice.isValid()) {
        printf("Device not available.  Here are the known devices:\n");
        printf("%s", Drivers::factory().toString().c_str());
        return 0;
    }
    yInfo("Polydriver successfully initialised");
    

    bool ok;
    ok = robotDevice.view(pos);
    ok = ok && robotDevice.view(encs);
    if (!ok) {
        printf("Problems acquiring interfaces\n");
        return 0;
    }
    int nj=0;
    pos->getAxes(&nj);
    encoders.resize(nj);
    tmp.resize(nj);
    command.resize(nj);
    
    int i;
    for (i = 0; i < nj; i++) {
         tmp[i] = 90.0;
    }
    pos->setRefAccelerations(tmp.data());

    for (i = 0; i < nj; i++) {
        tmp[i] = 50.0;
        pos->setRefSpeed(i, tmp[i]);
    }
    yInfo("successfully set accelerations and reference speed");
    
    
    //pos->setRefSpeeds(tmp.data()))
    //fisrst zero all joints
    //
    for(i=0; i<nj; i++) {
        command[i]=0;
    }
    command[0]=0;

    //pos->positionMove(command.data());//(4,deg);
    encs->getEncoder(3, &startPos3);
    encs->getEncoder(4, &startPos4);
    printf("start position value of joint 3: %lf\n", startPos3);
    printf("start position value of joint 4: %lf\n", startPos4);
    bool first=true;
    int deltaSacc=2;
    int times=0;
    yarp::os::Bottle bot; //= _pOutPort->prepare();
    bot.clear();
    bot.addVocab(COMMAND_VOCAB_DUMP);
    bot.addVocab(COMMAND_VOCAB_ON);
    Bottle inOn;
    _pOutPort->write(bot,inOn);

    */

    Time::delay(0.1);
    
    //fprintf(stderr, "Start saccade(s), number of repetition: %d\n", nOl);
    /*
    while(times<nOl)
    {
        times++;
        
	
	command[4]=-deltaSacc;
	//moveJoints(pos, command);
    pos->positionMove(command.data());
    if(first)
    {
        double curPos;
        encs->getEncoder(4, &curPos);
    	printf("current position value of joint 4: %lf\n", curPos);
        while((curPos>=startPos4-DELTAENC) && (curPos<=startPos4+DELTAENC))
        {
    	    printf("current position value of joint 4: %lf\n", curPos);
            encs->getEncoder(4, &curPos);
        }
        bot.clear();
        bot.addVocab(COMMAND_VOCAB_SYNC);
        Bottle inStart;
        _pOutPort->write(bot,inStart);
    	printf("1st synch asked\n");
        first=false;
    }
    Time::delay(0.1);
    */

    /*
	
	command[4]=deltaSacc;
	moveJoints(pos, command);	
	command[4]=0;
	moveJoints(pos, command);
    */

	/*
	command[3]=-deltaSacc;
	moveJoints(pos, command);
		command[3]=deltaSacc;
	moveJoints(pos, command);
	command[3]=0;
	moveJoints(pos, command);
    if(times>=nOl)
    {
        double curPos;
        encs->getEncoder(3, &curPos);
    	printf("current position value of joint 3: %lf\n", curPos);
        //while((curPos<startPos3-DELTAENC) || (curPos>startPos3+DELTAENC))
        //{
	    //    command[3]=0;
    	//    printf("current position value of joint 3: %lf\n", curPos);
        //    encs->getEncoder(3, &curPos);
        //}
        bot.clear();
        bot.addVocab(COMMAND_VOCAB_SYNC);
        Bottle inEnd;
        _pOutPort->write(bot,inEnd);
    	printf("2nd synch asked\n");
    }


    }
    */
    idle = true;
    value = 0;   
    r = 0.5;  //meters
    Vector _angles(3);
    _angles(0) =  0.0;
    _angles(1) =  0.0;
    _angles(2) =  0.0;
    angles.resize(3);
    angles = _angles;
    Vector _position(3);
    _position(0) = -0.5;
    _position(1) =  0.0;
    _position(2) =  0.4;
    position.resize(3);
    position = _position;
    

    /* 
    rThread = new headingAudioRatethread(robotName, configFile);
    rThread->setName(getName().c_str());
    rThread->setGain(gain);
    //rThread->setInputPortName(inputPortName.c_str());
    rThread->start(); // this calls threadInit() and it if returns true, it then calls run()
    */

    yInfo("heading audio module initialization successfully accomplished...");

    return true ;       // let the RFModule know everything went well
                        // so that it will then run the module
}

bool headingAudioModule::interruptModule() {
    handlerPort.interrupt();
    return true;
}

bool headingAudioModule::close() {
    handlerPort.close();
    _pOutPort->close();
    _pInPort->close();
    _pAbsolutePort->close();
    //restoreContext()
    delete igaze;
    /* stop the thread */
    //yDebug("stopping the thread \n");
    //rThread->stop();
    return true;
}

bool headingAudioModule::respond(const Bottle& command, Bottle& reply) 
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
    /*else if ((command.get(0).asString()=="sus") || (command.get(0).asString()=="\"sus\"")) {
        //prioritiser->suspend();
        reply.addString("ok");
    }
    else if (command.get(0).asString()=="res" || command.get(0).asString()=="\"res\"" ) {
        //prioritiser->resume();
        reply.addString("ok");
    }
    */
    
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
    case COMMAND_VOCAB_SUS:
        rec = true;
        {
            idle = true;
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
    case COMMAND_VOCAB_RES:
    rec = true;
        {
            idle = false;
            //prioritiser->resume();
            ok = true;
        }
        break;
    case COMMAND_VOCAB_SEEK:
        rec = true;
        {
            //prioritiser->suspend();
            //prioritiser->seek(command);
            //prioritiser->resume();
            ok = true;
        }
        break;
    case COMMAND_VOCAB_FIX:
        rec = true;
        {
            switch (command.get(1).asVocab()) {
            case COMMAND_VOCAB_CENT:
                {
                    printf("Fixating in Center \n");
                    //prioritiser->fixCenter(1000);
                }
                break;
            }
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
        reply.addVocab(COMMAND_VOCAB_FAIL);
    }
    else
        reply.addVocab(COMMAND_VOCAB_OK);
    
    
    return true;
}

/* Called periodically every getPeriod() seconds */
bool headingAudioModule::updateModule()
{
    if(!idle){
        yInfo("module updating...");
        if (_pInPort->getInputCount()) {
            Bottle* b = _pInPort->read(false);
            if(b!=NULL) {
                value = b->get(0).asDouble();
                printf("got the double %f \n", value);
                encs->getEncoder(2, &startPos2);
                //printf("getEncoder3 position %f \n", startPos2);
                if ((value+startPos2 < 50) && (value+startPos2 > -50)){
                    endPos2 = startPos2 - value ;

                    command[2] = endPos2;
                    //moveJoints(pos, command);

                    // iKinGazeCtrl convention: positive angles toward robot right hand side
                    position(0) = -1 * (cos(deg2rad * endPos2) * r);
                    position(1) = -1 * (sin(deg2rad * endPos2) * r);
                    printf("sending vector %s \n", position.toString().c_str());
                    igaze->lookAtFixationPoint(position);

                    
                    //angles(0) = value;
                    //printf("sending vector %s \n", angles.toString().c_str());
                    //igaze->lookAtRelAngles(angles);
                    //}
                    //else{
                    //if (value+startPos2 > 50) {
                    //    command[2] = 50;
                    //moveJoints(pos, command);
                        
                    //}
                    //else {
                    //    command[2] = -50;
                    //moveJoints(pos, command);     
                    //}
                    //}
                }
            }
        }
        if (_pAbsolutePort->getInputCount()) {
            Matrix* m = _pAbsolutePort->read(false);
            double value[360];
            double max = 0.0;
            int pos = 0;
            if(m!=NULL) {
                for(int i = 0; i < 360; i++) {
                    value[i] = m->operator()(1,i); 
                    if(value[i] > max) {
                        max = value[i];
                        pos = i;
                    } 
                }              
            }
            double targetAzimuthAngle = (double) pos;
            Vector target(3);
            target[0] = 10.0;
            target[1] = 0.0;
            target[2] = 5.0;
            igaze->lookAtAbsAngles(target);
        
        }
    }
    return true;
}

double headingAudioModule::getPeriod()
{
    /* module periodicity (seconds), called implicitly by myModule */
    return 1;
}

