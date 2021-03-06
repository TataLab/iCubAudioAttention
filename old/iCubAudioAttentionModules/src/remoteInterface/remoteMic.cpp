/*
 * Copyright: (C) 2010 RobotCub Consortium
 * Authors: Paul Fitzpatrick, Francesco Nori
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#include <stdio.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/AudioGrabberInterfaces.h>
#include <yarp/os/Property.h>
#include <yarp/os/NetInt16.h>
#include <yarp/os/ResourceFinder.h>

#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/Log.h>
#include <yarp/os/Time.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/BufferedPort.h>

//#define DEBUG

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

const double rec_seconds = 0.1;
//const int rate = 48000;           //@Rea : removed in favour to RF
//const int fixedNSample = 4096;    //@Rea : removed in favour to RF

int main(int argc, char *argv[]) {
    // Open the network
    Network yarp;

    // initialization
    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("Audio_Attention_Config.ini"); //overridden by --from parameter
    rf.setDefaultContext("audio_attention");               //overridden by --context parameter
    rf.configure(argc, argv);  


    int rate          =  rf.findGroup("sampling").check("samplingRate",    Value(48000),  "frame samples (int)"        ).asInt();
    int fixedNSample  =  rf.findGroup("sampling").check("numFrameSamples", Value(4096),   "sampling rate of mics (int)").asInt();


    //BufferedPort<Sound> p;
    Port p;
    p.open("/sender");

    // Get a portaudio read device.
    Property conf;
    conf.put("device","portaudio");
    conf.put("read", "");
    // conf.put("samples", rate * rec_seconds);
    conf.put("samples", fixedNSample);
    conf.put("rate", rate);
    yInfo("number of samples: %d", fixedNSample);
    yInfo("rate:%d", rate);

    // opening the polydriver and relative IAudioGrabberSound			
    PolyDriver poly(conf);
    IAudioGrabberSound *get;

    // Make sure we can read sound
    poly.view(get);
    if (get==NULL) {
        printf("cannot open interface");
        return 1;
    }
    else{
        printf("correctly opened the interface rate: %d, number of samples: %f, number of channels %d \n",rate, rate*rec_seconds, 2);
    }

    //Grab and send
    Sound s;
    double t1, t2, tdiff;
    
    //unsigned char* dataSound;
    //short* dataAnalysis;
#ifdef DEBUG
    int v1, v2;
    int i = 0, j = 0;
    NetInt16 v;
#endif

    get->startRecording(); //this is optional, the first get->getsound() will do this anyway.
    Stamp ts;
    while (true)
    {
      t1=yarp::os::Time::now();
      ts.update();  
      //s = p.prepare();           
      
      get->getSound(s);        

#ifdef DEBUG          
		//v1 = s.get(i,j);
  		//v = (NetInt16) v1;
  		printf("%d \n",s.get(0,0));
		printf("%d \n",s.get(0,1));
  		//v2 = s.get(i+1,j+1); 
  		//dataAnalysis = (short*) dataSound;
#endif        
      
      p.setEnvelope(ts);
      p.write(s);

      t2=yarp::os::Time::now();
      tdiff= t2-t1;
      printf("acquired %f seconds \n", tdiff);
    }
    get->stopRecording();  //stops recording.

    return 0;
}

