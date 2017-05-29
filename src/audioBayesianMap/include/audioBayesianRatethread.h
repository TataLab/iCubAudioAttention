// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-


#ifndef _BAYESIAN_RATETHREAD_H_
#define _BAYESIAN_RATETHREAD_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Log.h>
#include <iostream>
#include <fstream>
#include <time.h>


class bayesianRatethread : public yarp::os::RateThread {
private:
    // void setAcousticMap();
    // void normalizeCurrentAudioMap();
    // void normalizePropabilityMap();
    // void loadFile();
    // void calcOffset();
    // void createMemoryMappedFile();
    // void memoryMapper();
    // void sendAudioMap();

        
    std::string robot;              // name of the robot
    std::string configFile;         // name of the configFile where the parameter of the camera are set
    std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber

    yarp::os::BufferedPort<yarp::sig::Matrix> *inPort;								
    yarp::os::Port *outPort;	

    yarp::sig::Matrix* inputMatrix;
    yarp::sig::Matrix* outputMatrix;

    std::vector <std::vector <double>> probabilityMap;
    std::vector <std::vector <double>> currentAudioMap;

    std::string name;                                                                // rootname of all the ports opened by this thread

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
 
    //Variables need to time the update module
    struct timeval st, en;
    long mtime, seconds, useconds;




public:
    /**
    * constructor default
    */
    bayesianRatethread();

    /**
    * constructor 
    * @param robotname name of the robot
    */
    bayesianRatethread(std::string robotname,std::string configFile);

    /**
     * destructor
     */
    ~bayesianRatethread();

    /**
    *  initialises the thread
    */
    bool threadInit();

    /**
    *  correctly releases the thread
    */
    void threadRelease();

    /**
    *  active part of the thread
    */
    void run(); 

    /**
    * function that sets the rootname of all the ports that are going to be created by the thread
    * @param str rootnma
    */
    void setName(std::string str);
    
    /**
    * function that returns the original root name and appends another string iff passed as parameter
    * @param p pointer to the string that has to be added
    * @return rootname 
    */
    std::string getName(const char* p);

    /**
    * function that sets the inputPort name
    */
    void setInputPortName(std::string inpPrtName);

     /**
     * method for the processing in the ratethread
     * @param mat matrix to be processed in the method
     **/
    bool processing(yarp::sig::Matrix *mat);

    /**
     * method for the processing in the ratethread
     **/
    bool processing(yarp::os::Bottle *b);
};

#endif 

//----- end-of-file --- ( next line intentionally left blank ) ------------------
