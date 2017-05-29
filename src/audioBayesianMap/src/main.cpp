#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <iostream>
#include "audioBayesianModule.h"

using namespace yarp::os;
using namespace yarp::dev;


int main(int argc, char * argv[])
{
    /* initialize yarp network */
    Network yarp;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("audioConfig.ini");    //overridden by --from parameter
    rf.setDefaultContext("icubAudioAttention/conf");    //overridden by --context parameter
    rf.configure(argc, argv);
    std::cout << "[INFO] Configuring and starting module. \n";

    if (!yarp.checkNetwork(1))
    {
        printf("[ERROR] YARP server not available!\n");
        return -1;
    }

    BayesianModule module;
    return module.runModule(rf);
}
