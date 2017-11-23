#include <iostream>
#include "../include/iCub/ObjectRecognitionInferModule.h"


using namespace yarp::os;
using namespace yarp::sig;


int main(int argc, char *argv[]) {

    Network yarp;
    ObjectRecognitionInferModule module;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("objectRecognitionInfer.ini");      //overridden by --from parameter
    rf.setDefaultContext("objectRecognitionInfer");              //overridden by --context parameter
    rf.configure(argc, argv);


    yInfo("resourceFinder: %s", rf.toString().c_str());

    module.runModule(rf);
    return 0;
}
