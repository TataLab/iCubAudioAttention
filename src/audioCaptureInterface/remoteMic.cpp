// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
  * Author:Francesco Rea
  * email: francesco.rea@iit.it
  * Permission is granted to copy, distribute, and/or modify this program
  * under the terms of the GNU General Public License, version 2 or any
  * later version published by the Free Software Foundation.
  *
  * A copy of the license can be found at
  * http://www.robotcub.org/icub/license/gpl.txt
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details
*/


#include <stdio.h>
#include <string>
#include <cstring>
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
#define SAMPLERATE 48000
#define IOCTL_SAMPLERATE _IOW(MAGIC_NUM, 1, int*)
#define TRUE 1 
#define FALSE 0

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace audio;


// WAVE file header format
struct HEADER {
	unsigned char riff[4];						// RIFF string
	unsigned int overall_size	;				// overall size of file in bytes
	unsigned char wave[4];						// WAVE string
	unsigned char fmt_chunk_marker[4];			// fmt string with trailing null char
	unsigned int length_of_fmt;					// length of the format data
	unsigned int format_type;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned int channels;						// no.of channels
	unsigned int sample_rate;					// sampling rate (blocks per second)
	unsigned int byterate;						// SampleRate * NumChannels * BitsPerSample/8
	unsigned int block_align;					// NumChannels * BitsPerSample/8
	unsigned int bits_per_sample;				// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header [4];		// DATA string or FLLR string
	unsigned int data_size;						// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};


// parameter for the usePortAudio
const double rec_seconds = 0.1;
const int rate = 48000;
const int fixedNSample = 4096;

struct HEADER header;
FILE *ptr;
unsigned char buffer4[4];
unsigned char buffer2[2];
long size_of_each_sample;
long num_samples;
//int read;

//prototypes
bool readWavFile(string filename);
bool streamWavFile(int numberFrames, audio::Sound* soundToSend);
char* seconds_to_time(float seconds);

int main(int argc, char *argv[]) {
    // initialization
    bool clientGazeCtrlAvailable = false;
    bool useDeviceDriver = false;  // set default value
    bool usePortAudio    = true;   // set default value
    bool usePrerecorded  = false;  // set default value
    string moduleName, robotName, robotPortName;
    string prerecordedFilePath;
    yarp::sig::Sound s;
    IAudioGrabberSound *get;
    int32_t buffermicif [NUM_MICS * SAMP_BUF_SIZE] = {0};
    int32_t bufferleft [SAMP_BUF_SIZE] = {0};
    int32_t* pointermicif;
    int ExpectedReading;
    int micif_dev;
    int sampleRate;
    int bufferSize;
    int numberFrames = 0;
    
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
        printf("--sampleRate     : sets the sample rate (in Hz) \n");
        printf("--bufferSize     : sets the size of the buffer \n");
        printf("--name           : rootname for all the connection of the module \n");
        printf("--usePortAudio   : audio input from portaudio \n");
        printf("--useDeviceDriver: audio input from device driver \n");
        printf("--usePrerecorded : audio input from prerecorded file \n");
        printf(" \n");
        printf("press CTRL-C to stop... \n");
        return true;
    }
    

    /* get the module name which will form the stem of all module port names */
    moduleName             = rf.check("name",
				      Value("/audioCaptureInterface"),
				      "module name (string)").asString();



    /* detects whether the preferrable input is from portaudio */
    if(rf.check("usePortAudio")){
      printf("acquiring from portaudio \n");
      usePortAudio = true;
      useDeviceDriver = false;
      usePrerecorded = false;
    }
    /* detects whether the preferrable input is from deviceDriver */
    else if(rf.check("useDeviceDriver")){
      printf("acquiring from deviceDriver \n");
      usePortAudio = false;
      useDeviceDriver = true;
      usePrerecorded = false;
    }
    /* detects whether the preferrable input is from prerecorded file */
    else if(rf.check("usePrerecorded")){

      prerecordedFilePath             = rf.check("usePrerecorded",
					Value("audio.wav"),
					"file path (string)").asString();
      yInfo("acquiring from preRecorded file %s \n", prerecordedFilePath.c_str());
      usePortAudio = false;
      useDeviceDriver = false;
      usePrerecorded = true;
    }
    
    /*setting robot name*/
    robotName             = rf.check("robot",
				     Value("icub"),
				     "Robot name (string)").asString();
    robotPortName         = "/" + robotName + "/head";
    printf("robotName: %s \n", robotName.c_str());

    /* setting sample rate*/
    sampleRate           = rf.check("sampleRate",
				     Value(SAMPLERATE),
				     "Sample rate in Hz (integer)").asInt();
    yInfo("sampleRate: %d", sampleRate);

    /* get the preferable dimension of the frame*/
    bufferSize             = rf.check("bufferSize",
				      Value(4096),
				      "Buffer size (int)").asInt();
    yInfo("bufferSize: %d", bufferSize);
    
    // Opening the output port
    // BufferedPort<Sound> p;
    Port p;
    p.open("/audioGrabber/sender");
    

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


    if(usePortAudio) {
        yInfo("reading from the portaudio device \n");
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
        audio::Sound s;
        get->startRecording(); //this is optional, the first get->getsound() will do this anyway.
    } // end of the portAudio branch
    
    //******************************************************************************
    
    if(useDeviceDriver) {
        yInfo("reading from the device drive \n");
        // reading direclty from the device drive.
        micif_dev = -1;
        micif_dev = open("/dev/micif_dev", O_RDONLY);
        printf("read from the device \n");
        if(micif_dev<0) {
            fprintf(stderr, "Error opening %s:", MICIFDEVICE);
            return -1;
        }

        // set the sampling rate. For example to set the frame to 12kHz the following
        // e.g.: ioctl(micif_dev, IOCTL_SAMPLERATE,12000);
        ioctl(micif_dev, IOCTL_SAMPLERATE, sampleRate);
      
        // read from device
        //ExpectedReading = sizeof(int) * SAMP_BUF_SIZE;
        ///if(read(micif_dev, buffermicif, ExpectedReading) < 0)
        //return -1;
        //else {
        //printf("success in reading from the device \n");
        //}

    } // end of the useDeviceDriver branch 
    
    //******************************************************************************

    if(usePrerecorded) {
      yInfo("reading from the Prerecorded file \n");
      bool res = readWavFile(prerecordedFilePath);
      Time::delay(1);      
    } // end of the usePrerecorded branch 
    
    //******************************************************************************
 
    
    
    //spatialSound* soundToSend= new spatialSound(4);
    //soundToSend->setNumberOfAngles(2);
    audio::Sound* soundToSend= new audio::Sound(4);
    soundToSend->resize(SAMP_BUF_SIZE,STEREO);
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
        else if(useDeviceDriver) {
            int readDim = read(micif_dev, buffermicif, dimensionToRead);
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
        else if(usePrerecorded) {
            // read each sample from data chunk if PCM
            //printf("starting stream Wav...");
            if((numberFrames + 1) * SAMP_BUF_SIZE < num_samples) {
                bool ret = streamWavFile(numberFrames, soundToSend);
                Time::delay(0.080);
                numberFrames++;
            }
            else {
                yInfo("preRecorded file ended");
                Time::delay(0.080);
            }
        }
        else {
            yInfo("IDLE.....");
        }
        p.setEnvelope(ts);
        p.write(*soundToSend);
        
        //*********************************************************************
        
        double t2=yarp::os::Time::now();
        printf("acquired %f in seconds \n", t2-t1);
    }
    get->stopRecording();  //stops recording.
    
    return 0;
}


bool streamWavFile(int numberFrames, audio::Sound* soundToSend) {
    int read = 0;
    //yDebug("streamWavFile");
    if (header.format_type == 1) { // PCM
                
        long i =0;
        char data_buffer[size_of_each_sample];
        int  size_is_correct = TRUE;

        // make sure that the bytes-per-sample is completely divisible by num.of channels
        long bytes_in_each_channel = (size_of_each_sample / header.channels);
        //printf("size_of_each_sample %d bytes in each channel %d", size_of_each_sample,bytes_in_each_channel );
        
        if ((bytes_in_each_channel  * header.channels) != size_of_each_sample) {
            printf("Error: %ld x %ud <> %ld\n", bytes_in_each_channel, header.channels, size_of_each_sample);
            size_is_correct = FALSE;
        }
 
        if (size_is_correct) { 
            // the valid amplitude range for values based on the bits per sample
            long low_limit = 0l;
            long high_limit = 0l;

            switch (header.bits_per_sample) {
            case 8:
                low_limit = -128;
                high_limit = 127;
                break;
            case 16:
                low_limit = -32768;
                high_limit = 32767;
                break;
            case 32:
                low_limit = -2147483648;
                high_limit = 2147483647;
                break;
            }					

            //for (int j= 0; j < SAMP_BUF_SIZE; j++) {
            //    read = fread(data_buffer, sizeof(data_buffer),  1 , ptr);
            //}

            
            
            //printf("\n\n.Valid range for data values : %ld to %ld \n", low_limit, high_limit);
            //for (i =1; i <= num_samples; i++) {
            for  (i =1; i <= SAMP_BUF_SIZE; i++) {
                //printf("==========Sample %ld / %ld=============\n", SAMP_BUF_SIZE * numberFrames + i, num_samples);
                read = fread(data_buffer, sizeof(data_buffer), 1, ptr);
                
                if (/*read == 1*/ true) {
				
                    // dump the data read
                    unsigned int  xchannels = 0;
                    int data_in_channel = 0;

                    for (xchannels = 0; xchannels < header.channels; xchannels ++ ) {
                        
                        //printf("(size_of_each_sample %d) Channel#%d : ",size_of_each_sample, (xchannels+1));
                        
                        if (bytes_in_each_channel == 4) {
                            // // convert data from little endian in each channel sample
                            /*data_in_channel =	data_buffer[3] | 
                                (data_buffer[2]<<8) | 
                                (data_buffer[1]<<16) | 
                                (data_buffer[0]<<24);
                            */
                            
                            // convert data from little endian to big endian based on bytes in each channel sample
                            data_in_channel =	data_buffer[0 + xchannels * 4] | 
                                (data_buffer[1 + xchannels * 4]<<8) | 
                                (data_buffer[2 + xchannels * 4]<<16) | 
                                (data_buffer[3 + xchannels * 4]<<24);

                            
                            
                        }
                        else if (bytes_in_each_channel == 2) {
                            data_in_channel = data_buffer[0] |
                                (data_buffer[1] << 8);
                        }
                        else if (bytes_in_each_channel == 1) {
                            data_in_channel = data_buffer[0];
                        }
                        //printf("%d ", data_in_channel);
                        soundToSend->set(data_in_channel, i - 1, xchannels);

                        // check if value was in range
                        if (data_in_channel < low_limit || data_in_channel > high_limit) {
                            printf("**value out of range\n");
                        }

                        //printf(" | ");
                    }

                    //printf("\n");
                }
                else {
                    printf("Error reading file. %d bytes\n", read);
                    break;
                }
            } // 	for (i =1; i <= num_samples; i++) {
            //Time::delay(1);
        } // 	if (size_is_correct)                
    } //  if (header.format_type == 1)

    //printf("returning...");
    return true;
}

bool readWavFile(string filename) {
    // open file
    printf("Opening  file.. %s\n", filename.c_str());
    //string filename("audio.wav");
    ptr = fopen(filename.c_str(), "rb");
    if (ptr == NULL) {
        printf("Error opening file\n");
        return false;
    }

    int read = 0;
    
    // read header parts
    read = fread(header.riff, sizeof(header.riff), 1, ptr);
    printf("(1-4): %s \n", header.riff); 
    
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
    
    // convert little endian to big endian 4 byte int
    header.overall_size  = buffer4[0] | 
        (buffer4[1]<<8) | 
        (buffer4[2]<<16) | 
        (buffer4[3]<<24);
    
    printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header.overall_size, header.overall_size/1024);
    
    read = fread(header.wave, sizeof(header.wave), 1, ptr);
    printf("(9-12) Wave marker: %s\n", header.wave);
    
    read = fread(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, ptr);
    printf("(13-16) Fmt marker: %s\n", header.fmt_chunk_marker);
    
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
    
    // convert little endian to big endian 4 byte integer
    header.length_of_fmt = buffer4[0] |
        (buffer4[1] << 8) |
        (buffer4[2] << 16) |
        (buffer4[3] << 24);
    printf("(17-20) Length of Fmt header: %u \n", header.length_of_fmt);
    
    read = fread(buffer2, sizeof(buffer2), 1, ptr); printf("%u %u \n", buffer2[0], buffer2[1]);
    
    header.format_type = buffer2[0] | (buffer2[1] << 8);
    char format_name[10] = "";
    if (header.format_type == 1)
        strcpy(format_name,"PCM"); 
    else if (header.format_type == 6)
        strcpy(format_name, "A-law");
    else if (header.format_type == 7)
        strcpy(format_name, "Mu-law");
    
    printf("(21-22) Format type: %u %s \n", header.format_type, format_name);
    
    read = fread(buffer2, sizeof(buffer2), 1, ptr);
    printf("%u %u \n", buffer2[0], buffer2[1]);
    
    header.channels = buffer2[0] | (buffer2[1] << 8);
    printf("(23-24) Channels: %u \n", header.channels);
    
    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
    
    header.sample_rate = buffer4[0] |
        (buffer4[1] << 8) |
        (buffer4[2] << 16) |
        (buffer4[3] << 24);

    printf("(25-28) Sample rate: %u\n", header.sample_rate);

    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

    header.byterate  = buffer4[0] |
        (buffer4[1] << 8) |
        (buffer4[2] << 16) |
        (buffer4[3] << 24);
    printf("(29-32) Byte Rate: %u , Bit Rate:%u\n", header.byterate, header.byterate*8);

    read = fread(buffer2, sizeof(buffer2), 1, ptr);
    printf("%u %u \n", buffer2[0], buffer2[1]);

    header.block_align = buffer2[0] |
        (buffer2[1] << 8);
    printf("(33-34) Block Alignment: %u \n", header.block_align);

    read = fread(buffer2, sizeof(buffer2), 1, ptr);
    printf("%u %u \n", buffer2[0], buffer2[1]);

    header.bits_per_sample = buffer2[0] |
        (buffer2[1] << 8);
    printf("(35-36) Bits per sample: %u \n", header.bits_per_sample);

    read = fread(header.data_chunk_header, sizeof(header.data_chunk_header), 1, ptr);
    printf("(37-40) Data Marker: %s \n", header.data_chunk_header);

    read = fread(buffer4, sizeof(buffer4), 1, ptr);
    printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

    header.data_size = buffer4[0] |
        (buffer4[1] << 8) |
        (buffer4[2] << 16) | 
				(buffer4[3] << 24 );
    printf("(41-44) Size of data chunk: %u \n", header.data_size);


    // calculate no.of samples
    num_samples = (8 * header.data_size) / (header.channels * header.bits_per_sample);
    printf("Number of samples:%lu \n", num_samples);

    size_of_each_sample = (header.channels * header.bits_per_sample) / 8;
    printf("Size of each sample:%ld bytes\n", size_of_each_sample);

    // calculate duration of file
    float duration_in_seconds = (float) header.overall_size / header.byterate;
    printf("Approx.Duration in seconds=%f\n", duration_in_seconds);
    printf("Approx.Duration in h:m:s=%s\n", seconds_to_time(duration_in_seconds));

}


/**
 * Convert seconds into hh:mm:ss format
 * Params:
 *	seconds - seconds value
 * Returns: hms - formatted string
 **/
char* seconds_to_time(float raw_seconds) {
    char *hms;
    int hours, hours_residue, minutes, seconds, milliseconds;
    hms = (char*) malloc(100);
    
    sprintf(hms, "%f", raw_seconds);
    
    hours = (int) raw_seconds/3600;
    hours_residue = (int) raw_seconds % 3600;
    minutes = hours_residue/60;
    seconds = hours_residue % 60;
    milliseconds = 0;
    
    // get the decimal part of raw_seconds to get milliseconds
    char *pos;
    pos = strchr(hms, '.');
    int ipos = (int) (pos - hms);
    char decimalpart[15];
    memset(decimalpart, ' ', sizeof(decimalpart));
    strncpy(decimalpart, &hms[ipos+1], 3);
    milliseconds = atoi(decimalpart);	
    
    sprintf(hms, "%d:%d:%d.%d", hours, minutes, seconds, milliseconds);
    return hms;
}

