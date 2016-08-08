#ifndef _AUDIO_PREPROCESSER_MODULE_H_
#define _AUDIO_PREPROCESSER_MODULE_H_


#include "gammatonFilter.h"
#include "beamFormer.h"
#include <string>

#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/NetInt16.h>
#include <yarp/sig/Sound.h>

//Memory mapping requirements
#include <sys/mman.h>
#include <fcntl.h>

//TODO Switch this to use YARP TIME
//Required for the timing of the module
#include <sys/time.h>

using namespace yarp::os;


const NetInt16 normDivid = 32768;

class AudioPreprocesserModule: public RFModule
{

public:
	/**
	* default constructor
	*/
	AudioPreprocesserModule();

	/**
	* destructor
	*/
	~AudioPreprocesserModule();


	bool configure(ResourceFinder &rf);
	double getPeriod();
	bool updateModule();
	bool interruptModule();
	bool close();


private:
	//Colored warnings to match YARP
	std::string myerror;
	std::string myinfo;
	std::string mywarn;
	std::string myreset;

	void loadFile();
	void createMemoryMappedFile();
	void memoryMapper();
	double interpl(int x, int x1, int x2, double y1, double y2);

	/**
	* SpineInterp()
	*
	*	Taking the Audio data that is found in reducedBeamFormedAudioVector. Creates an interpolation of the data corresponding to the interpellateNSamples that was specified in the xml.
	*	The data of this function will be saved in highResolutionAudioMap.
	*/
	void spineInterp();



	int interpellateNSamples;

	std::vector < std::vector < double > > highResolutionAudioMap;


	//Variables need to time the update module
	struct timeval st, en;
	long mtime, seconds, useconds;

	//Incoming Audio Data from the iCub and remoteInterface
	yarp::os::BufferedPort<yarp::sig::Sound> *inPort;
	yarp::sig::Sound* s;
	yarp::os::Stamp ts;
	float *rawAudio;

	int frameSamples;
	int nBands;
	int totalBeams;
	int nMics;

	GammatonFilter *gammatonAudioFilter;
	BeamFormer *beamForm;
	double oldtime;
	int lastframe;
	std::vector < std::vector < float* > > beamFormedAudioVector;
	std::vector < std::vector < double > > reducedBeamFormedAudioVector;

	//TODO rename this

	std::string fileName;
	FILE *fid;
	int mappedFileID;
	double *mappedAudioData;

};

#endif
