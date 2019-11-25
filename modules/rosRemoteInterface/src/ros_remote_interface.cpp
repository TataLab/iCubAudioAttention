/*
* Copyright: (C) 2019 RobotCub Consortium
* Authors: Lukas Grasse, Austin Kothig, Paul Fitzpatrick, Francesco Nori
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#include <stdio.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/AudioGrabberInterfaces.h>
#include <yarp/os/Property.h>
#include <yarp/os/NetInt16.h>
#include <yarp/os/ResourceFinder.h>
#include <typeinfo>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/Log.h>
#include <yarp/os/Time.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/BufferedPort.h>
#include <ros/ros.h>
//#define DEBUG
#include <ros_remote_interface/Sound.h>
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

int main(int argc, char *argv[]) {

    ros::init(argc, argv, "ros_remote_interface");
  
    // initialization
    int samplingRate;
    ros::param::param<int>("~sampling_rate", samplingRate, 48000);

    int numFrameSamples; 
    ros::param::param<int>("~num_frame_samples", numFrameSamples, 4096);

    int sampleBufferSize;
    ros::param::param<int>("~sample_buffer_size", sampleBufferSize, 8192);

    std::string topicName;
    ros::param::param<std::string>("~topic_name", topicName, "/rosAudioRemapper/rosAudio");

    ros::NodeHandle n;

    ros::Publisher audio_pub = n.advertise<ros_remote_interface::Sound>(topicName, 1000);

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
    while (ros::ok()) {
    
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

        ros_remote_interface::Sound msg;

        //set ros msg params to sound object params
        msg.n_samples = s.getSamples();
        msg.n_channels = s.getChannels();

        //set left channel
        msg.l_channel_data.resize(msg.n_samples);
        
        for(int i=0; i<s.getSamples(); i++)
            msg.l_channel_data[i] = s.get(i, 0);

        //if we have right channel set as well
        if(msg.n_channels > 1) {
            msg.r_channel_data.resize(msg.n_samples);

            for(int i=0; i<msg.n_samples; i++) 
                msg.r_channel_data[i] = s.get(i, 1);
        }

        msg.n_frequency = s.getFrequency();

        //set timestamp
        msg.time = ts.getTime();

        audio_pub.publish(msg);
        ros::spinOnce();

        t2 = yarp::os::Time::now();
        tdiff = t2-t1;
        yInfo(" acquired %f seconds \n", tdiff);
    }

    get->stopRecording();  //stops recording.
    
    return 0;
}
    
