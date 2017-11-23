//
// Created by jonas on 10/20/17.
//

#ifndef OBJECTRECOGNITIONTRAIN_TENSORFLOWPARAMETERS_H
#define OBJECTRECOGNITIONTRAIN_TENSORFLOWPARAMETERS_H



#include <yarp/os/all.h>
#include <map>




class TensorFlowLauncher {

public:

    TensorFlowLauncher(yarp::os::ResourceFinder &rf);


    // HashMap for the differents parametersMap
    std::map<std::string, std::string> parametersMap;


    // PROCESS FUNCTIONS
    void launchTensorFlowRetraining(yarp::os::BufferedPort<yarp::os::Bottle> &t_logPort);
    bool checkParameters();

private:

    yarp::os::ResourceFinder rf;

    // List of mandatories parameters for ObjectRecogntionRetraining
    const std::vector<std::string> mandatoryParameters {
            "image_dir",
            "output_graph",
            "output_labels",
    };


    // List of optional parameters for ObjectRecogntionRetraining
    const std::vector<std::string> optionalParameters {
            "architecture",
            "learning_rate",
            "how_many_training_steps",
            "random_brightness",
            "random_scale",
            "random_crop",
            "flip_left_right",
            "model_dir",
    };


};


#endif //OBJECTRECOGNITIONTRAIN_TENSORFLOWPARAMETERS_H
