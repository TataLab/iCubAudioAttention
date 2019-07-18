// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include <yarp/os/Network.h>
/*#include <stdio.h>
#include <stdlib.h>
#include <math.h> 

#include <yarp/os/all.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/GazeControl.h>
#include <string>*/

#include "iCub/headingAudioModule.h"



//using namespace yarp::dev;
//using namespace yarp::sig;
using namespace yarp::os;

/*
void moveJoints(IPositionControl *_pos, Vector& _command)
{
    _pos->positionMove(_command.data());
    Time::delay(0.1);
}
*/

int main(int argc, char *argv[]) 
{
    Network yarp;
    headingAudioModule module; 

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("headingAudio.ini");    //overridden by --from parameter
    rf.setDefaultContext("headingAudio");    //overridden by --context parameter
    rf.configure(argc, argv);  
 
    module.runModule(rf);
    return 0;

    //-----------------------------------------------------------------------

    
    /*
    while(true){
        if (_pInPort->getInputCount()) {
            Bottle* b = _pInPort->read(true);
            value = b->get(0).asDouble();
            printf("got the double %f \n", value);
            encs->getEncoder(2, &startPos2);
            //printf("getEncoder3 position %f \n", startPos2);
            //if ((value+startPos2 < 50) && (value+startPos2 > -50)){
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
    */

	//Time::delay(0.1);


    /*bot.clear();
    bot.addVocab(COMMAND_VOCAB_SYNC);
    Bottle inEnd;
    _pOutPort->write(bot,inEnd);
    */

    /*
    bot.clear();
    bot.addVocab(COMMAND_VOCAB_DUMP);
    bot.addVocab(COMMAND_VOCAB_OFF);
    Bottle inOff;
    _pOutPort->write(bot,inOff);

    _pOutPort->close();
    robotDevice.close();
    //-------------------------------------------

    igaze->restoreContext(originalContext);
    delete igaze;
    */
    
    return 0;
}
