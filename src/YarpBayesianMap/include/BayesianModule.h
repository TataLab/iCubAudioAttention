#ifndef _BAYESIAN_MODULE_H_
#define _BAYESIAN_MODULE_H_

#include "../../Configuration/ConfigParser.h"

#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Property.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IEncoders.h>

#include <vector>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>

class BayesianModule: public yarp::os::RFModule
{
public:
    BayesianModule();
    ~BayesianModule();

    bool configure(yarp::os::ResourceFinder &rf);
    double getPeriod();
    bool updateModule();
    bool interruptModule();
    bool close();


    std::vector <std::vector <double>> getProbabilityMap();
private:
    void setAcousticMap();
    void normalizeCurrentAudioMap();
    void normalizePropabilityMap();
    void loadFile();
    void calcOffset();

    void createMemoryMappedFile();
    void memoryMapper();
    void sendAudioMap();

    //Variables need to time the update module
    struct timeval st, en;
    long mtime, seconds, useconds;


    yarp::os::BufferedPort<yarp::sig::Matrix> *inPort;
    yarp::os::BufferedPort<yarp::sig::Matrix> *outPort;

    std::vector <std::vector <double>> probabilityMap;
    std::vector <std::vector <double>> currentAudioMap;

    yarp::sig::Matrix* inputMatrix;
    yarp::sig::Matrix* outputMatrix;

    yarp::os::Property options;
    yarp::dev::PolyDriver *robotHead;
    yarp::dev::IEncoders *enc;
    yarp::os::Stamp ts;

    //Memory mapping variables
    FILE *fid;
    int mappingFileID;
    double *probabilityMapping;

    int nBands;
    std::string fileName;
    double offset;
    int interpellateNSamples;


};

#endif
