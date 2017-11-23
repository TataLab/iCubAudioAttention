#include "../include/iCub/objectRecognitionTrainModule.h"
#include <iostream>


using namespace yarp::os;
using namespace yarp::sig;


int main(int argc, char *argv[]) {

    Network yarp;
    if (!yarp.checkNetwork(1.0)) {
        yInfo("Impossible to find yarp server, ensure that a yarp server is runing.");
        return 0;

    }
    objectRecognitionTrainModule module;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("objectRecognitionTrain.ini");      //overridden by --from parameter
    rf.setDefaultContext("objectRecognitionTrain");              //overridden by --context parameter


    rf.configure(argc, argv);

    yInfo("resourceFinder: %s", rf.toString().c_str());

    module.runModule(rf);


    return 0;
}
