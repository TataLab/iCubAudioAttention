// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-


// global includes
#include <iostream>
#include <stdio.h>
#include <sys/mman.h> //include for memory mapping
#include <fcntl.h>

// include yarp
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Sound.h>
#include <yarp/os/NetInt16.h>

using namespace yarp::os;
using namespace yarp::sig;

//Global variables
const NetInt16 normDivid = 32768;  //normalised signed 16bit int into signed [-1.0,+1.0]


//TODO NEED TO GET RID OFs
const int sampleRate = 48000;
const double samplePeriod = 1.0 / sampleRate;
const int numChannels = 2; //how many channels to record from
const int bufferDuration_samples = 512;
const int bufferDuration_bytes = bufferDuration_samples * (numChannels + 2) * sizeof(double);
const int numBuffersInMemoryMap = 20;  //this is the "echoic memory" of your robot!  Your other processes will have access to this much back data
const int memoryMapSize_samples = numBuffersInMemoryMap * bufferDuration_samples;
const int memoryMapSize_bytes = memoryMapSize_samples * (numChannels + 2) * sizeof(double);

int main(int argc, char *argv[]) {

	//Declair yarp
	Network yarp;
	if (!yarp.checkNetwork())
	{
		printf("YARP server not available!\n");
		return -1;
	}

	//Creating a Resiving yarp port
	BufferedPort<Sound> bufferPort;
	bufferPort.open("/receiver");
	if (!Network::exists("/receiver")) {
		printf("receiver not exists \n");
		return -1;
	}

	//Connecting the resiving audio port with the sender on the pc104
	Network::connect("/sender", "/receiver");
	if (!Network::isConnected("/sender", "/receiver")) {
		printf("no connection \n");
		return -1;
	}

	double empty[4096 *4] = {0}; //plus 2 for time and sample stamps

	//setup for memory mapping
	FILE *fid;
	fid = fopen("/tmp/AudioMemMap.tmp", "w");
	fwrite(empty, sizeof(double), 4096 *4, fid); //plus 2 for time and sample stamps
	fclose(fid);
	int mappedFileID;
	mappedFileID = open("/tmp/AudioMemMap.tmp", O_RDWR);
	double *mappedAudioData;
	mappedAudioData = (double *)mmap(0, memoryMapSize_bytes, PROT_WRITE, MAP_SHARED , mappedFileID, 0);
	//printf("Here\n");

	  double sampleDur = 1.0 / 48000;

	//Sound stuff
	Sound* s;
	Stamp ts;
	while (true) {
		//printf("here\n");
		s = bufferPort.read(true);
		bufferPort.getEnvelope(ts);
		printf("count:%d time:%f \n", ts.getCount(), ts.getTime());

		int e0 = ts.getCount();
		double e1 = ts.getTime();
		int row = 0;
		for (int col = 0 ; col < 4096; col++) {
			NetInt16 temp_c = (NetInt16) s->get(col, 0);
			NetInt16 temp_d = (NetInt16) s->get(col, 1);
			mappedAudioData[row]        	= (double) 	temp_c / normDivid ;
			mappedAudioData[row + 1] 		= (double) 	temp_d / normDivid;
			mappedAudioData[row + 2] 		= (double) 	(e0 * 4096) + col;
			mappedAudioData[row + 3]		= (double) 	(e1 + col * sampleDur);
			row += 4;

		}
	}
	return 0;
}
