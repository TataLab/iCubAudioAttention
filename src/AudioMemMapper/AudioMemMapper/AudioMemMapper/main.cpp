//
//  main.cpp
//  AudioMemMapper
//
//  Created by Matthew Tata on 2016-02-19.
//  Copyright © 2016 Matthew Tata. All rights reserved.
//

#include <iostream>
#include <portaudio.h>

#include <sys/mman.h> //include for memory mapping
#include <fcntl.h>

const int sampleRate=48000;
const double samplePeriod=1.0/sampleRate;
const int numChannels=2; //how many channels to record from
const int bufferDuration_samples = 512;
const int bufferDuration_bytes = bufferDuration_samples * (numChannels + 2) * sizeof(double);
const int numBuffersInMemoryMap = 20;  //this is the "echoic memory" of your robot!  Your other processes will have access to this much back data
const int memoryMapSize_samples=numBuffersInMemoryMap * bufferDuration_samples;
const int memoryMapSize_bytes=memoryMapSize_samples*(numChannels+2)*sizeof(double); //plus 2 for time stamp and sample stamp


double sampleCounter=1;

typedef struct {
    double *recordedSamples;
    int  bufferIsFull=0;
}callbackDataStruct;


//define the callback
static int recordCallback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData ){
    
    callbackDataStruct *data = (callbackDataStruct*)userData;
    const float *rptr = (const float*)inputBuffer;
    double *wptr = &data->recordedSamples[0];  //assuming we're always reading exactly one inputBuffer completely and writing it completely to the memory mapped region
    
    //prevent unused variable warnings
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;
    
    double bufferTime = static_cast<double>(timeInfo->inputBufferAdcTime);
    //
    //    //transfer data out of the input buffer into your frame buffer
    for (int i=0; i<framesPerBuffer; i++) {  //by "frames" they mean samples...silly portaudio people
        
        
        *wptr++=static_cast<double>(*rptr++); //left channel
        *wptr++=static_cast<double>(*rptr++); //right channel
        
        *wptr++ = sampleCounter;  //sample stamp
        *wptr++ = bufferTime+i*samplePeriod;  //time stamp
        
        
        sampleCounter++;
    }
    
    data->bufferIsFull = 1;  //signal main thread to copy data into memory mapped region
    int finished = paContinue;
    return finished;
}



/*******************************************************************/


int main(void);
int main(void)
{
    
    
    printf("Hello World, I work!\n");
    
    //some parameters
    PaError err;
    
    
    //set up a data structure to interact with the audio stream via a callback
    callbackDataStruct data;
    
    
    //allocate memory to hold recorded samples, time stamp and sample stamp
    data.recordedSamples = (double *) malloc(bufferDuration_samples*(numChannels+2)); //plus 2 for time and sample stamps
    if (data.recordedSamples == NULL ) {
        printf("could not allocate memory for recorded samples\n");
        goto done;
    }
    for(int i=0;i<bufferDuration_samples*(numChannels+2);i++){
        data.recordedSamples[i]=(double)0.0; //fill data with initial zeros
    }
   
    //here's an empty buffer to initialize the memory map
    double empty[memoryMapSize_samples*(numChannels+2)]; //plus 2 for time and sample stamps
    for (int i=0; i<memoryMapSize_samples*(numChannels+2); i++) {
        empty[i]=(double)0.0;
    }
    
    
    //setup for memory mapping
    FILE *fid;
    fid=fopen("/tmp/AudioMemMap.tmp","w");
    fwrite(empty, sizeof(double), memoryMapSize_samples*(numChannels+2), fid); //plus 2 for time and sample stamps
    fclose(fid);
    int mappedFileID;
    mappedFileID=open("/tmp/AudioMemMap.tmp",O_RDWR);
    double *mappedAudioData;
    mappedAudioData=(double *)mmap(0, memoryMapSize_bytes, PROT_WRITE, MAP_SHARED , mappedFileID, 0);
    
    /* Initialize library before making any other calls. */
    err = Pa_Initialize();
    if( err != paNoError ) goto done;
    
    //set up parameters of the default device
    PaStreamParameters inputParameters;
    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = 2;                    /* stereo input */
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    
    //set up the stream
    PaStream *stream;
    err= Pa_OpenStream(&stream, &inputParameters,NULL,sampleRate, bufferDuration_samples, paClipOff, recordCallback, &data);
    
    //start the stream
    err = Pa_StartStream(stream);
    if (err != paNoError) goto done;
   
    int whileIsDone;
    whileIsDone=0;
    while (!whileIsDone) {
     
        if (data.bufferIsFull==1) {
            
            //reset the buffer flag
            data.bufferIsFull=0;
            
            //for quick and dirty time check
            //PaTime tic=Pa_GetStreamTime(stream);
            
            
            //shift the samples in the memory mapped region by 1 buffer length
            memcpy(&mappedAudioData[0], &mappedAudioData[bufferDuration_samples*(numChannels+2)],bufferDuration_bytes*(numBuffersInMemoryMap-1));
            
            //move data from the data structure into the memory mapped region
            memcpy(&mappedAudioData[bufferDuration_samples*(numChannels+2)*(numBuffersInMemoryMap-1)], &data.recordedSamples[0], bufferDuration_bytes);
            
            
            //PaTime toc=Pa_GetStreamTime(stream);
            
            //time it.  Make sure that we can copy this memory within one buffer duration (which we can)
            //printf("elapsed time = %f\n", toc-tic);
            
            
        }
        
    }
    
    //stop the stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        goto done;
    }
    
    
    //close the stream
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        goto done;
    }
    
    //terminate portaudio
    err = Pa_Terminate();
    if (err !=paNoError) {
        goto done;
    }
    
    
done:
    Pa_Terminate();
    if( data.recordedSamples )       /* Sure it is NULL or valid. */
        free( data.recordedSamples );
    if( err != paNoError )
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    return err;
}



