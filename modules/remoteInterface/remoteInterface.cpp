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

int main(int argc, char *argv[]) {

    // Open the network
    Network yarp;
    
    // initialization
    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultConfigFile("audio_attention_config.ini"); // overridden by --from parameter
    rf.setDefaultContext("audio_attention");               // overridden by --context parameter
    rf.configure(argc, argv);  
    
    int samplingRate     = rf.findGroup("sampling").check("samplingRate",     yarp::os::Value(48000),  "Frame samples (int)"                    ).asInt();
    int numFrameSamples  = rf.findGroup("sampling").check("numFrameSamples",  yarp::os::Value(4096),   "Sampling rate of mics (int)"            ).asInt();
    int sampleBufferSize = rf.findGroup("sampling").check("sampleBufferSize", yarp::os::Value(8192),   "Number of samples to buffer in PO (int)").asInt();

    Port p;
    p.open("/rawAudio:o");
    
    // Get a portaudio read device.
    Property conf;
    conf.put("device", "portaudioRecorder");
    conf.put("rate",    samplingRate);
    conf.put("samples", sampleBufferSize);
    

    // opening the polydriver and relative IAudioGrabberSound			
    PolyDriver poly(conf);
    IAudioGrabberSound *get;
    
    // Make sure we can read sound
    poly.view(get);
    if (get == NULL) {
        yError("cannot open interface . . .");
        return 1;
    } else {
        yInfo(" correctly opened the interface . . . \n");
        yInfo("   sampling rate      : %d ", samplingRate);
        yInfo("   number of samples  : %d ", numFrameSamples);
        yInfo("   number of channels : %d ", 2);
        yInfo(" ");
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
    while (true) {
    
        t1 = yarp::os::Time::now();
        ts.update();  

        get->getSound(s, numFrameSamples, numFrameSamples, 0.0);

        #ifdef DEBUG          
        //v1 = s.get(i,j);
        //v = (NetInt16) v1;
        printf("%d \n",s.get(0,0));
        printf("%d \n",s.get(0,1));
        //v2 = s.get(i+1,j+1); 
        //dataAnalysis = (short*) dataSound;
        #endif

        yInfo(" ");
        yInfo(" Rate      : %d ", s.getFrequency());
        yInfo(" Samples   : %ld ", s.getSamples());
        yInfo(" Channels  : %ld ", s.getChannels());

        p.setEnvelope(ts);
        p.write(s);

        t2 = yarp::os::Time::now();
        tdiff = t2-t1;
        yInfo(" acquired %f seconds \n", tdiff);
    }

    get->stopRecording();  //stops recording.
    
    return 0;
}
    
