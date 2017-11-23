//
// Created by jonas on 10/20/17.
//

#include "iCub/TensorFlowLauncher.h"

using namespace yarp::os;
using namespace std;

TensorFlowLauncher::TensorFlowLauncher(yarp::os::ResourceFinder &t_rf) {

    this->rf = t_rf;


}

void TensorFlowLauncher::launchTensorFlowRetraining(yarp::os::BufferedPort<yarp::os::Bottle> &logPort) {

    char buffer[128];
    Bottle &logs = logPort.prepare();
    vector<string> result;
    string finalCmd = "python retrain.py";



    for (std::map<string,string>::iterator it=parametersMap.begin(); it!=parametersMap.end(); ++it){
        finalCmd += " --" + it->first + " " + it->second;
    }



    //finalCmd += " 2>&1";
    FILE *pipe = popen(finalCmd.c_str(), "r");
    if (!pipe) throw runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL){
                logs.addString(static_cast<string>(buffer));
                logPort.writeStrict();
            }

        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);

}

bool TensorFlowLauncher::checkParameters() {

    for (int i = 0; i < mandatoryParameters.size(); ++i) {
        const string param = mandatoryParameters[i];

        if (rf.check(param)) {
            parametersMap[param] = rf.find(param).asString();
        }

        else{
            cout << "Your forget to specify " <<  param << endl;
            return false;
        }


    }

    for (int i = 0; i < optionalParameters.size(); ++i) {
        const string param = optionalParameters[i];

        if (rf.check(param)) {
            parametersMap[param] = rf.find(param).asString();
        }


    }

    return true;
}
