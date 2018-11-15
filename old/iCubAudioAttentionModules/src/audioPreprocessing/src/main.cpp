#include <yarp/os/all.h>
#include <yarp/dev/all.h>

#include "audioPreprocesserModule.h"

int main(int argc, char * argv[]) {

    /* initialize yarp network */
    yarp::os::Network yarp;

    yarp::os::ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("audioConfig.ini");    //overridden by --from parameter
    rf.setDefaultContext("icubAudioAttention");    //overridden by --context parameter
    rf.configure(argc, argv);
    yInfo("Configuring and starting module.");

    if (!yarp.checkNetwork(1)) {
        yError("YARP server not available!");
        return -1;
    }

    AudioPreprocesserModule module;
    return module.runModule(rf);
}
