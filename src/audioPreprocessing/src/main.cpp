#include "audioPreprocesserModule.h"

#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <iostream>


int main(int argc, char * argv[])
{
    /* initialize yarp network */
    yarp::os::Network yarp;


    yarp::os::ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("../../app/icubAudioAttention/conf/audioConfig.ini");    //overridden by --from parameter
    rf.setDefaultContext("icubAudioAttention");    //overridden by --context parameter
    rf.configure(argc, argv);
    std::cout << "[INFO] Configuring and starting module. \n";

    if (!yarp.checkNetwork(1))
    {
        printf("[ERROR] YARP server not available!\n");
        return -1;
    }

    AudioPreprocesserModule module;
    return module.runModule(rf);
}
