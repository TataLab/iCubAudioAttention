// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* 
 * Copyright (C) 2014 Robotics, Brain and Cognitive Science (RBCS)
 * Author: Rea Francesco
 * email:  francesco.rea@iit.it
 * website: www.robotcub.org
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
\defgroup headingControl headingControl
 
@ingroup icub_audio_attention  
 
The manager module for the Joint Grasping Demo developed by IIT 
and ISR. 

Copyright (C) 2010 RobotCub Consortium
 
Author: Rea Francesco 

CopyPolicy: Released under the terms of the GNU GPL v2.0.

\section intro_sec Description
This module collects the 3-d object positions estimated by the 
particle filter and sends data to the head and arm controllers 
in order to gaze at the target, reach for it and eventually 
grasp it. 
It relies on the YARP ICartesianControl interface to control 
both arms and on the YARP IGazeControl interface to control the 
gaze. 
 
Furthermore, there exists a second modality that enables to 
estimate the 3-d object position usin-g stereo vision that needs 
to be calibrated in advance relying on a feed-forward neural 
network. 
 
\section lib_sec Libraries 
- ctrlLib. 
- iKin.  
- YARP libraries. 

\section parameters_sec Parameters
None. 
 
\section portsa_sec Ports Accessed 
The robot interface is assumed to be operative; in particular, 
the ICartesianControl interface must be available. The 
\ref iKinGazeCtrl must be running.
 
\section portsc_sec Ports Created 
 
- \e /demoGraspManager_IIT_ISR/trackTarget:i receives the 3-d 
  position to track.
 
- \e /demoGraspManager_IIT_ISR/imdTargetLeft:i receives the 
  blobs list as produced by the \ref motionCUT module for the
  left eye.
 
- \e /demoGraspManager_IIT_ISR/imdTargetRight:i receives the 
  blobs list as produced by the \ref motionCUT module for the
  right eye.
 
- \e /demoGraspManager_IIT_ISR/cmdFace:o sends out commands to 
  the face expression high level interface in order to give an
  emotional representation of the current robot state.
 
- \e /demoGraspManager_IIT_ISR/gui:o sends out info to update target
  within the \ref icub_gui.

- \e /demoGraspManager_IIT_ISR/rpc remote procedure 
    call. Recognized remote commands:
    -'quit' quit the module
 
\section in_files_sec Input Data Files
None.

\section out_data_sec Output Data Files 
None. 
 
\section conf_file_sec Configuration Files
The configuration file passed through the option \e --from
should look like as follows:
 
\code 
[general]
// the robot name to connect to 
robot           icub
// the thread period [ms] 
thread_period   30
// left arm switch 
left_arm        on 
// right arm switch 
right_arm       on 
// arm trajectory execution time [s]
traj_time       2.0 
// reaching tolerance [m]
reach_tol       0.01 
// eye used 
eye             left 
// homes limbs if target detection timeout expires [s]
idle_tmo        5.0 
// enable the use of stereo vision calibrated by NN 
use_network off 
// NN configuration file 
network         network.ini 

[torso] 
// joint switch (min **) (max **) [deg]; 'min', 'max' optional 
pitch on  (max 30.0) 
roll off 
yaw on

[left_arm]
// the offset [m] to be added to the desired position  
reach_offset        0.0 -0.15 -0.05
// the offset [m] for grasping 
grasp_offset        0.0 0.0 -0.05
// perturbation given as standard deviation [m] 
grasp_sigma 0.01 0.01 0.01 
// hand orientation to be kept [axis-angle rep.] 
hand_orientation 0.064485 0.707066 0.704201 3.140572 
// enable impedance velocity mode 
impedance_velocity_mode off 
impedance_stiffness 0.5 0.5 0.5 0.2 0.1 
impedance_damping 60.0 60.0 60.0 20.0 0.0 

[right_arm]
reach_offset        0.0 0.15 -0.05
grasp_offset        0.0 0.0 -0.05
grasp_sigma	        0.01 0.01 0.01
hand_orientation    -0.012968 -0.721210 0.692595 2.917075
impedance_velocity_mode off 
impedance_stiffness 0.5 0.5 0.5 0.2 0.1 
impedance_damping 60.0 60.0 60.0 20.0 0.0 
 
[home_arm]
// home position [deg] 
poss    -30.0 30.0 0.0  45.0 0.0  0.0  0.0
// velocities to reach home positions [deg/s] 
vels    10.0  10.0 10.0 10.0 10.0 10.0 10.0

[arm_selection]
// hysteresis range added around plane y=0 [m]
hysteresis_thres 0.1

[grasp]
// ball radius [m] for still target detection 
sphere_radius   0.05 
// timeout [s] for still target detection 
sphere_tmo      3.0 
// timeout [s] to open hand after closure 
release_tmo     3.0 
// open hand positions [deg] 
open_hand       0.0 0.0 0.0   0.0   0.0 0.0 0.0   0.0   0.0 
// close hand positions [deg] 
close_hand      0.0 80.0 12.0 18.0 27.0 50.0 20.0  50.0 135.0 
// velocities to reach hand positions [deg/s] 
vels_hand       10.0 10.0  10.0 10.0 10.0 10.0 10.0 10.0  10.0 
\endcode 

\section tested_os_sec Tested OS
Windows, Linux

\author Rea Francesco
*/ 


#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <yarp/os/Network.h>
#include <yarp/os/all.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/GazeControl.h>
#include <iCub/managerThread.h>
//#include <iCub/azimuthThread.h>

#include <string>

#define COMMAND_VOCAB_ON    VOCAB2('o','n')
#define COMMAND_VOCAB_OFF   VOCAB3('o','f','f')
#define COMMAND_VOCAB_DUMP  VOCAB4('d','u','m','p')
#define COMMAND_VOCAB_SYNC  VOCAB4('s','y','n','c')

#define DELTAENC 0.0000001
#define deg2rad  3.1415/180

// general command vocab's
#define COMMAND_VOCAB_IS     VOCAB2('i','s')
#define COMMAND_VOCAB_OK     VOCAB2('o','k')

#define COMMAND_VOCAB_HELP   VOCAB4('h','e','l','p')
#define COMMAND_VOCAB_POINT  VOCAB4('p','o','i','n')
#define COMMAND_VOCAB_LOOK   VOCAB4('l','o','o','k')
#define COMMAND_VOCAB_FAILED VOCAB4('f','a','i','l')
#define COMMAND_VOCAB_TRED   VOCAB4('t','r','e','d')
#define COMMAND_VOCAB_TGRE   VOCAB4('t','g','r','e')
#define COMMAND_VOCAB_TBLU   VOCAB4('t','b','l','u')
#define COMMAND_VOCAB_FRED   VOCAB4('f','r','e','d')       // request of fovea blob color (red)
#define COMMAND_VOCAB_FBLU   VOCAB4('f','b','l','u')       // request of fovea blob color (red)
#define COMMAND_VOCAB_FGRE   VOCAB4('f','g','r','e')       // request of fovea blob color (red)
#define COMMAND_VOCAB_FRGB   VOCAB4('f','r','g','b')       // request of fovea blob color (rgb)
#define COMMAND_VOCAB_FRGB   VOCAB4('f','r','g','b')       // request of fovea blob color (rgb)


#define COMMAND_VOCAB_RED    VOCAB3('R','E','D')           // maximum dimension of the blob drawn
#define COMMAND_VOCAB_BLUE   VOCAB4('B','L','U','E')       // minimum dimension of the blob drawn
#define COMMAND_VOCAB_GREE   VOCAB4('G','R','E','E')       // minimum dimension of the bounding area
#define COMMAND_VOCAB_YEL    VOCAB3('Y','E','L')


using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::os;

YARP_DECLARE_DEVICES(icubmod)

void moveJoints(IPositionControl *_pos, Vector& _command)
{
    _pos->positionMove(_command.data());
    Time::delay(0.1);
}

class managerModule: public RFModule
{
protected:
    managerThread *thr;
    //azimuthThread *athr;
    Port           rpcPort;
    Semaphore      mutex;
    string robot;
    bool usePanOnly;
    
public:
    managerModule() {
        //athr = 0;
        thr  = 0;
    }

    bool configure(ResourceFinder &rf)
    {
        Time::turboBoost();
        

        Bottle &bGeneral=rf.findGroup("general");
        bGeneral.setMonitor(rf.getMonitor());
        robot=bGeneral.check("robot",Value("icub"),"Getting robot name").asString().c_str();
        usePanOnly=bGeneral.check("pan_only",Value("on"),"Getting the pan-head work").asString()=="on"?true:false;

        if(usePanOnly) {
            yInfo("usePanOnly activate");
            //athr = new azimuthThread(getName().c_str(),rf);
            /*if (!athr->start())
                {
                    yError("azimuthThread not started");
                    delete athr;
                    return false;
                }
            */
        }
        else {
            yInfo("wholebody controlled");
            thr=new managerThread(getName().c_str(),rf);
            if (!thr->start())
                {
                    yError("managerThread not started");
                    delete thr;    
                    return false;
                }
        }
            
        rpcPort.open(getName("/rpc"));
        attach(rpcPort);

        return true;
    }

    bool close()
    {
        rpcPort.interrupt();
        rpcPort.close();
        
        /*if(athr) {
            yInfo("stopping the thread");
            athr->stop();
            yInfo("deleting the thread");
            delete athr;
            }*/
        if(thr) {
            thr->stop();
            delete thr;
        }

        return true;
    }

    bool respond(const Bottle &command, Bottle &reply) {
        // 
        bool ok = false;
        bool rec = false; // is the command recognized?
        
        mutex.wait();
        
        switch (command.get(0).asVocab()) {
        case COMMAND_VOCAB_HELP:
            rec = true;
            {
                reply.addVocab(Vocab::encode("many"));
                reply.addString("help");
                
                //reply.addString("\n");
                reply.addString("point n \t: general point command");
                //reply.addString("\n");
                reply.addString("look  \t: general look command ");
                //reply.addString("\n");
                reply.addString("NOTE: capitalization of command name is mandatory");
                reply.addString("set Mdb : set maximum dimension allowed for blobs");
                reply.addString("set mdb : set minimum dimension allowed for blobs");
                reply.addString("set mBA : set the minimum bounding area");
                //reply.addString("\n");
                reply.addString("get Mdb : get maximum dimension allowed for blobs");
                reply.addString("get mdb : get minimum dimension allowed for blobs");
                reply.addString("get mBA : get the minimum bounding area\n");
                ok = true;
            }
            break;
            
        case COMMAND_VOCAB_LOOK:
            rec = true;
            {
                yInfo("*** LOOK command received");
                switch (command.get(1).asVocab()) {
                case COMMAND_VOCAB_RED:
                    rec = true;
                    {
                        yInfo("      RED OBJECT SEEKING");
                    }
                    break;
                case COMMAND_VOCAB_BLUE:
                    rec = true;
                    {
                        yInfo("      BLUE OBJECT SEEKING");
                    }
                    break;
                case COMMAND_VOCAB_GREE:
                    rec = true;
                    {
                        yInfo("      GREEN OBJECT SEEKING");
                    }
                    break;
                case COMMAND_VOCAB_YEL:
                    rec = true;
                    {
                        yInfo("      YELLOW OBJECT SEEKING");
                    }
                    break;
                default:{}
                }

                if(usePanOnly) {
                    
                }
                else {
                    thr->setAction("look");
                }
                

                 ok = true;

            }
            break;
            
        case COMMAND_VOCAB_POINT:
            rec = true;
            {
                if(usePanOnly) {
                    
                }
                else {
                    thr->setAction("point");
                }
                
                

                ok = true;
                
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
        
        return ok;
    } 	
    
    double getPeriod()    {
        return 1.0;
    }

    bool   updateModule() { 
        return true;
    }
};
