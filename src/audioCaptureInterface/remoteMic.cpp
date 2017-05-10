/*
 * Copyright: (C) 2017 RBCS Robotics Brain and Cognitive Sciences
 * Authors: Francesco Rea
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/GazeControl.h>
#include <yarp/dev/AudioGrabberInterfaces.h>
#include <yarp/os/Property.h>
#include <yarp/os/NetInt16.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/Time.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/BufferedPort.h>

#include <iCub/audio/spatialSound.h>


// parameter for the useDeviceDriver mode
#define MICIFDEVICE "/dev/micif_dev"
#define STEREO 2
#define NUM_MICS (1 * STEREO)
#define SAMP_BUF_SIZE 4096
#define MAGIC_NUM 100
#define SAMPLERATE 16000
#define IOCTL_SAMPLERATE _IOW(MAGIC_NUM, 1, int*)

// parameter for the usePortAudio
const double rec_seconds = 0.1;
const int rate = 48000;
const int fixedNSample = 4096;

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace audio::spatialSound;

int main(int argc, char *argv[]) {
  // initialization
  bool clientGazeCtrlAvailable = false;
  bool useDeviceDriver = false;
  bool usePortAudio    = true; // set default value
  string moduleName, robotName, robotPortName;
  Sound s;
  IAudioGrabberSound *get;
  int32_t buffermicif [NUM_MICS * SAMP_BUF_SIZE] = {0};
  int32_t bufferleft [SAMP_BUF_SIZE] = {0};
  int32_t* pointermicif;
  int ExpectedReading;
  int micif_dev;
  
  // Open the network
  Network yarp;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("remoteInterface.ini");      //overridden by --from parameter
    rf.setDefaultContext("remoteInterface");     //overridden by --context parameter
    rf.configure(argc,argv);
    
    /*********************************************************/

    if(rf.check("help")) {
      printf("HELP \n");
      printf("====== \n");
      printf("--name           : changes the rootname of the module ports \n");
      printf("--robot          : changes the name of the robot where the module interfaces to  \n");
      printf("--visualFeedback : indicates whether the visual feedback is active \n");
      printf("--name           : rootname for all the connection of the module \n");
      printf("--usePortAudio   : audio input from portaudio \n");
      printf("--useDeviceDriver: audio input from device driver \n");
      printf(" \n");
      printf("press CTRL-C to stop... \n");
      return true;
    }


    /* get the module name which will form the stem of all module port names */
    moduleName             = rf.check("name",
				      Value("/remoteInterface"),
				      "module name (string)").asString();

    /* detects whether the preferrable input is from portaudio */
    if(rf.check("usePortAudio")){
      printf("acquiring from portaudio \n");
    }
    /* detects whether the preferrable input is from deviceDriver */
    if(rf.check("useDeviceDriver")){
      printf("acquiring from deviceDriver \n");
      usePortAudio = false;
      useDeviceDriver = true;
    }
    /*                                                                                                                                    * get the robot name which will form the stem of the robot ports names                                                          
     * and append the specific part and device required                                                                              
     */
    robotName             = rf.check("robot",
				     Value("icub"),
				     "Robot name (string)").asString();
    robotPortName         = "/" + robotName + "/head";
    printf("robotName: %s \n", robotName.c_str());

    /*********************************************************/
    // preparing the interface with the iKinGazeCtrl

    //initializing gazecontrollerclient
    printf("initialising gazeControllerClient \n");
    Property option;
    option.put("device","gazecontrollerclient");
    option.put("remote","/iKinGazeCtrl");
    string localCon("/remoteInterface/client/gaze");
    option.put("local",localCon.c_str());

    PolyDriver* clientGazeCtrl = new PolyDriver();
    clientGazeCtrl->open(option);
    IGazeControl* igaze = NULL;

    if (clientGazeCtrl->isValid()) {
      clientGazeCtrl->view(igaze);
      clientGazeCtrlAvailable = true;
    }
    else {
      clientGazeCtrlAvailable = false;
      yInfo("gazeClient is not active! Information on azimuth/elevation is not availble");
    }

    //igaze->storeContext(&originalContext);

    //if(blockNeckPitchValue != -1) {
    //  igaze->blockNeckPitch(blockNeckPitchValue);
    //  printf("pitch fixed at %f \n",blockNeckPitchValue);
    //}
    //else {
    //  printf("pitch free to change\n");
    //}
    
    /*********************************************************/
    //BufferedPort<Sound> p;
    Port p;
    p.open("/sender");

    if(usePortAudio) {
      // Get a portaudio read device.
      Property conf;
      conf.put("device","portaudio");
      conf.put("read", "");
      // conf.put("samples", rate * rec_seconds);
      conf.put("samples", fixedNSample);
      conf.put("rate", rate);
      PolyDriver poly(conf);


      // Make sure we can read sound
      poly.view(get);
      if (get==NULL) {
	printf("cannot open interface");
	return 1;
      }
      else{
	printf("correctly opened the interface rate: %d, number of samples: %f, number of channels %d \n",rate, rate*rec_seconds, 2);
	//Grab and send
	get->startRecording(); //this is optional, the first get->getsound() will do this anyway
      }

      //Grab and send
      Sound s;
      get->startRecording(); //this is optional, the first get->getsound() will do this anyway.
    } // end of the portAudio branch

    if(useDeviceDriver) {
      printf("reading from the device drive \n");
      // reading direclty from the device drive.
      micif_dev = -1;
      micif_dev = open("/dev/micif_dev", O_RDONLY);
      printf("read from the device \n");
      if(micif_dev<0) {
	fprintf(stderr, "Error opening %s:", MICIFDEVICE);
	return -1;
      }

      // set the sampling rate. For example to set the frame to 12kHz the following
      // ioctl(micif_dev, IOCTL_SAMPLERATE,12000);
      ioctl(micif_dev, IOCTL_SAMPLERATE, SAMPLERATE);
      
      // read from device
      //ExpectedReading = sizeof(int) * SAMP_BUF_SIZE;
      ///if(read(micif_dev, buffermicif, ExpectedReading) < 0)
      //return -1;
      //else {
      //printf("success in reading from the device \n");
      //}

    } // end of the useDeviceDriver branch 
    
    //******************************************************************************

    
    //printf("saving the file size %lu of %d \n", sizeof(data), sizeof(int16));
    
    
    //spatialSound* soundToSend= new spatialSound(4);
    //soundToSend->setNumberOfAngles(2);
    Sound* soundToSend= new Sound(4);
    soundToSend->resize(4096,2);
    soundToSend->setFrequency(SAMPLERATE);
    
    Stamp ts;
    Vector angles(3);
    Stamp  anglesStamp;
    int dimensionToRead = sizeof(int32_t) * NUM_MICS * SAMP_BUF_SIZE;
     
    while (true)
    {
      double t1=yarp::os::Time::now();
      ts.update();

      //********************************************

      //Bottle b = p.prepare();
      if(usePortAudio) {
	get->getSound(s);
      }
      if(useDeviceDriver) {
	read(micif_dev, buffermicif, dimensionToRead);
	pointermicif = &buffermicif[0];

	//if(clientGazeCtrlAvailable) {
	//   igaze->getAngles(angles,&anglesStamp);
	//   soundToSend->setAngles(angles);
	//}
	
	//prepare soundToSend
	for (int sample=0; sample < SAMP_BUF_SIZE; sample++) {
	  //s.set(int value, int sample, int channel=0)
	  //printf("value %d %d \n",sample,buffermicif[sample * NUM_MICS]);
	  //printf("value %d %d \n",sample,buffermicif[sample * NUM_MICS + 1]);
	  //soundToSend->set(buffermicif[sample * NUM_MICS],sample,0);
	  //soundToSend->set(buffermicif[sample * NUM_MICS + 1], sample,1);
	  soundToSend->set(*pointermicif, sample, 0);
	  //bufferleft[sample] = *pointermicif;
	  //int32_t v = *pointermicif;
	  //printf("%08X \n", v);
	  pointermicif++;
	  soundToSend->set(*pointermicif, sample, 1);
	  pointermicif++;
	}

	//FILE* fid = fopen("/tmp/audioFromZTurn.tmp", "w");
	//fwrite(&bufferleft[0], sizeof(int32_t), SAMP_BUF_SIZE, fid);
	//fclose(fid);
      }
      p.setEnvelope(ts);
      p.write(*soundToSend);

      //*********************************************************************

      double t2=yarp::os::Time::now();
      printf(" %d acquired %f seconds \n",ExpectedReading, t2-t1);
    }
    get->stopRecording();  //stops recording.

    return 0;
}
