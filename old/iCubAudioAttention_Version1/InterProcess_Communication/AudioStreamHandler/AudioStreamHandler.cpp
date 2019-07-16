 // -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2006, 2007 RobotCub Consortium
 * Authors: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */


/* 
Modified by Matt Tata (M.S.T. April 2014) to write audio to a memory mapped file in "quasi"
real time

The plan is to interact as little as possible with Paul's code so I don't break anything!
*/

#include <deque>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <yarp/os/all.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/AudioGrabberInterfaces.h>
#include <yarp/sig/SoundFile.h>

//include the header for memory mapping (M.S.T.)
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>


//some parameters for mapping the audio data file (M.S.T.)
#define kFrameSizeToWrite_bytes 4096*4
#define kNumChannels 2
#define kNumSamples 4096
#define kSampleRate 48000 //this needs to be coordinated with the portaudio yarpdev!!
#define kBytesPerSample 2
#define kNumBytes kNumSamples*kNumChannels*kBytesPerSample
#define kFrameSizeToWrite_samples kFrameSizeToWrite_bytes/kBytesPerSample/kNumChannels

int kMappedFileSize_bytes = (int) kFrameSizeToWrite_bytes * 10000;  //determines the maximum duration of a session
int kMappedFileSize_samples = (int) kMappedFileSize_bytes/kBytesPerSample;

//some globals for memory mapping the audio file (M.S.T.)
short *mappedAudioData;
int *lastFrameIndex;  //keep track of the current frame being written
int *lastSampleIndex;//memory map this counter to quickly make an index into the memory mapped audio available to other processes

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;
using namespace yarp::dev;

int padding = 0;


class Echo : public TypedReaderCallback<Sound> {
private:
    PolyDriver poly;
    IAudioRender *put;
    BufferedPort<Sound> port;
    Semaphore mutex;
    bool muted;
    bool saving;
    std::deque<Sound> sounds;
    int samples;
    int channels;

public:
    Echo() : mutex(1) {
        put = NULL;
        port.useCallback(*this);
        port.setStrict();
        muted = false;
        saving = false;
        samples = 0;
        channels = 0;
        put = NULL;
    }

    bool open(Searchable& p) {
        bool dev = true;
        if (p.check("nodevice")) {
            dev = false;
        }
        if (dev) {
            poly.open(p);
            if (!poly.isValid()) {
                printf("cannot open driver\n");
                return false;
            }
            
            if (!p.check("mute")) {
                // Make sure we can write sound
                poly.view(put);
                if (put==NULL) {
                    printf("cannot open interface\n");
                    return false;
                }
            }
        }
            
        port.setStrict(true);
        if (!port.open(p.check("name",Value("/AudioAttention/listener")).asString())) {
            printf("Communication problem\n");
            return false;
        }
        
        if (p.check("remote")) {
            Network::connect(p.check("remote",Value("/remote")).asString(),
                             port.getName());
        }

        return true;
    }

    void onRead(Sound& sound)
     {
       
        #ifdef TEST
        //this block can be used to measure time elapsed between two sound packets
        static double t1= yarp::os::Time::now();
        static double t2= yarp::os::Time::now();
        t1= yarp::os::Time::now();
        printf("onread %f\n", t2-t1);
        t2 = yarp::os::Time::now();
        #endif

        int ct = port.getPendingReads();
        //printf("pending reads %d\n", ct);
        while (ct>padding) {
            ct = port.getPendingReads();
            printf("Dropping sound packet -- %d packet(s) behind\n", ct);
            port.read();
        }
        mutex.wait();
        /*
          if (muted) {
          for (int i=0; i<sound.getChannels(); i++) {
          for (int j=0; j<sound.getSamples(); j++) {
          sound.put(0,j,i);
          }
          }
          }
        */
        if (!muted) {
            if (put!=NULL) {
                put->renderSound(sound);
            }
        }
        if (saving) {
            saveFrame(sound);
        }

        mutex.post();
        Time::yield();
    }

    void mute(bool muteFlag=true) {
        mutex.wait();
        muted = muteFlag;
        mutex.post();
    }

    void save(bool saveFlag=true) {

        mutex.wait();
        saving = saveFlag;
        mutex.post();
    }

    void saveFrame(Sound& sound) {
    	//*************************************************************
		//Matt's code:
		//added code here to write this frame into the mapped audio data file at the appropriate index

		unsigned char *rawData=(unsigned char*)malloc(kNumBytes);
		
		rawData=sound.getRawData();
		
		//sound.getRawData() returns a vector with all the left channel then all the right channel (i.e. not interleaved).  We need to rearrange to be L,R,L,R 
		//now unpack and rearrange the bytes in the frame
		
		
		short left[kNumSamples]; //temporarily store the two channels in separate vectors
		short right[kNumSamples];
	
		memcpy(left,(void *)&rawData[0],kNumSamples*2); //pull out 2 bytes for each sample (i.e. 16-bits)
		memcpy(right,(void *)&rawData[kNumBytes/2],kNumSamples*2);
		
		//now think in shorts
		short audioData[kNumSamples*2]= {0};	
		int writeHere=0;
		for(int i=0;i<kNumSamples;i++){
			
			audioData[writeHere]=left[i];
			writeHere++;
			audioData[writeHere]=right[i];
			writeHere++;
			
		}
	

		//now copy the new audio data vector into the memory mapped file at the appropriate index
		memcpy(mappedAudioData + (lastFrameIndex[0] * kNumSamples*kNumChannels),(void *)&audioData[0],kNumBytes);	
		lastSampleIndex[0]=lastFrameIndex[0]*kNumSamples;  //a backdoor sort of way to pass information to MATLAB about where to start reading in a file
		lastFrameIndex[0]++; //increment the frame counter
		
		//*****************************************************************
		//the rest of this is Paul's code    	
        
        sounds.push_back(sound);
        samples += sound.getSamples();
        channels = sound.getChannels();
        printf("  %ld sound frames buffered in memory (%ld samples)\n", 
               (long int) sounds.size(),
               (long int) samples);
    }

    bool saveFile(const char *name) {
        mutex.wait();
        saving = false;

        Sound total;
        total.resize(samples,channels);
        long int at = 0;
        while (!sounds.empty()) {
            Sound& tmp = sounds.front();
            for (int i=0; i<channels; i++) {
                for (int j=0; j<tmp.getSamples(); j++) {
                    total.set(tmp.get(j,i),at+j,i);
                }
            }
            total.setFrequency(tmp.getFrequency());
            at += tmp.getSamples();
            sounds.pop_front();
        }
        mutex.post();
        bool ok = write(total,name);
        if (ok) {
            printf("Wrote audio to %s\n", name);
        }
        samples = 0;
        channels = 0;
        return ok;
    }

    bool close() {
        port.close();
        mutex.wait(); // onRead never gets called again once it finishes
        
        //free the memory that's been mapped to write the audio data
        munmap(mappedAudioData,kMappedFileSize_samples*sizeof(short));
        
        return true;
    }
};

int main(int argc, char *argv[]) {
    yarp::os::Network yarp;
    
    
    //pre-setup some memory mapping stuff right away
    printf("\n\nInitializing memory mapped files in the /tmp directory to hold audio data\n\n");
    
    //build a vector of short zeros
    short *initialAudioArray;
    initialAudioArray = (short *)malloc(kMappedFileSize_samples*sizeof(short));
    for (int i=0;i<kMappedFileSize_samples;i++){
    	initialAudioArray[i]=(short)0;
    }
    
    //write the zeros to a file to initialize it
    FILE *fid=fopen("/tmp/AudioDataDump.dat","w");
    fwrite(initialAudioArray,sizeof(short),kMappedFileSize_samples,fid);
    fclose(fid);
    
 	//memory map the file
 	//these were declared globally so that we can get to them from the callback
    int mappedFileID=open("/tmp/AudioDataDump.dat",O_RDWR);
   	mappedAudioData=(short *)mmap(0, kMappedFileSize_bytes, PROT_READ | PROT_WRITE, MAP_SHARED , mappedFileID, 0);
	close(mappedFileID);  //the double colon forces to call the close() from outside of this namespace (because it's obscured by the method close() 

	//make a "file" that holds a single int to keep index of frames written and initialize with a zero
    FILE *fid2=fopen("/tmp/lastFrameIndex.dat","w");
    int dummyInt=0;
    fwrite(&dummyInt,sizeof(int),1,fid2);
    fclose(fid2);
    
    //make a "file" that holds a single int to keep index of the last sample written and initialize with a zero
    FILE *fid3=fopen("/tmp/lastSampleIndex.dat","w");
    int dummyInt2=0;
    fwrite(&dummyInt2,sizeof(int),1,fid3);
    fclose(fid3);
    
	//memory map the index file
	int lastFrameFileID = open("/tmp/lastFrameIndex.dat",O_RDWR);
   	lastFrameIndex=(int *)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED , lastFrameFileID, 0);
	close(lastFrameFileID);

	//memory map the last sample index file
	int lastSampleFileID = open("/tmp/lastSampleIndex.dat",O_RDWR);
   	lastSampleIndex=(int *)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED , lastSampleFileID, 0);
	close(lastSampleFileID);


	//****now you are ready to write frames into this memory****//
    
     

    // see if user has supplied audio device
    Property p;
    if (argc>1) {
        p.fromCommand(argc,argv);
    }

    // otherwise default device is "portaudio"
    if (!p.check("device")) {
        p.put("device","portaudio");
        p.put("write",1);
        p.put("delay",1);
    }

    // start the echo service running
    Echo echo;
    echo.open(p);

    // process the keyboard
    bool muted = false;
    bool saving = false;
    bool help = false;
    ConstString fname = "audio_%06d.wav";
    int ct = 0;
    bool done = false;
    while (!done) {
        if (help) {
            printf("  Press return to mute/unmute\n");
            printf("  Type \"s\" to set start/stop saving audio in memory\n");
            printf("  Type \"write filename.wav\" to write saved audio to a file\n");
            printf("  Type \"buf NUMBER\" to set buffering delay (default is 0)\n");
            printf("  Type \"write\" or \"w\" to write saved audio with same/default name\n");
            printf("  Type \"q\" to quit\n");
            printf("  Type \"help\" to see this list again\n");
            help = false;
        } else {
            printf("Type \"help\" for usage\n");
        }
        
        ConstString keys = Network::readString();
        Bottle b(keys);
        ConstString cmd = b.get(0).asString();
        if (b.size()==0) {
            muted = !muted;
            echo.mute(muted);
            printf("%s\n", muted?"Muted":"Audible again");
        } else if (cmd=="help") {
            help = true;
        } else if (cmd=="s") {

            saving = !saving;
            echo.save(saving);
            printf("%s\n", saving?"Saving":"Stopped saving");
            if (saving) {
                printf("  Type \"s\" again to stop saving\n");
            }
        } else if (cmd=="write"||cmd=="w") {
            if (b.size()==2) {
                fname = b.get(1).asString();
            }
            char buf[2560];
            sprintf(buf,fname.c_str(),ct);
            echo.saveFile(buf);
            ct++;
        } else if (cmd=="q"||cmd=="quit") {
            done = true;
        } else if (cmd=="buf"||cmd=="b") {
            padding = b.get(1).asInt();
            printf("Buffering at %d\n", padding);
        }
    }

    echo.close();

    return 0;
}

