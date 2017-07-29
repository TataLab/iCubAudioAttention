// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata,  Lukas Grasse, Marko Ilievski
  * email: m.ilievski@uleth.ca, lukas.grasse@uleth.ca, matthew.tata@uleth.ca, francesco.rea@iit.it
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
#include <audioFilteringRateThread.h>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

#define THRATE 80 //ms

audioFilteringRateThread::audioFilteringRateThread():RateThread(THRATE) {
    robot = "icub";      

}

audioFilteringRateThread::audioFilteringRateThread(string _robot, string _configFile):RateThread(THRATE){
    robot = _robot;
    configFile = _configFile;
}

audioFilteringRateThread::~audioFilteringRateThread() {
    // do nothing
}
 
bool audioFilteringRateThread::threadInit(yarp::os::ResourceFinder &rf) {

    yarp::os::ResourceFinder rf;
    int argc;
    char ** argv;
    rf.setDefaultConfigFile("audioConfig.ini");    //overridden by --from parameter
    rf.setDefaultContext("icubAudioAttention/conf");    //overridden by --context parameter
    rf.configure(argc, argv);

    frameSamples  = rf.check("frameSamples", 
        Value("4096"), 
        "frame samples (int)").asInt();
    nBands  = rf.check("nBands", 
        Value("128"), 
        "numberBands (int)").asInt();
    nMics  = rf.check("nMics", 
        Value("2"), 
        "number mics (int)").asInt();
    interpolateNSamples  = rf.check("interpolateNSamples", 
         Value("180"), 
         "interpellate N samples (int)").asInt();
    micDistance = rf.check("micDistance", 
         Value("0.145"), 
         "micDistance (double)").asDouble();
    samplingRate = rf.check("samplingRate", 
         Value("48000"), 
         "sampling rate of mics (int)").asInt();
    longTimeFrame = rf.check("longBufferSize", 
         Value("360"), 
         "long Buffer Size (int)").asInt();
    initializeAudioProcessing(direction);

    nBeamsPerHemi  = (int)((micDistance / C) * samplingRate) - 1;
    yInfo("_beamsPerHemi %d = %f / %d * %d", nBeamsPerHemi, micDistance, C, samplingRate);
    totalBeams = nBeamsPerHemi * 2 + 1;
    yInfo("frameSamples = %d", frameSamples);
    yInfo("nBands = %d", nBands);
    yInfo("nMics = %d", nMics);
    yInfo("interpolateNSamples = %d", interpolateNSamples );
    yInfo("total beams = %d",totalBeams);
}

void audioFilteringRateThread::setName(string str) {
    this->name=str;

}

std::string audioFilteringRateThread::getName(const char* p) {
    string str(name);
    str.append(p);
    return str;
}

void audioFilteringRateThread::run() {    
    s = inPort->read(true);

      if (s!=NULL) {
          
          Sound filtered(*s);

          for(int i=0; i<NUM_PROCESS_FRAMES; i++) {
            Sound section = s->subSound((CHANNEL_SIZE/NUM_PROCESS_FRAMES)*i, (CHANNEL_SIZE/NUM_PROCESS_FRAMES)*(i+1));
            Sound filtChunk = processFrame(section);

            for(int j=0; j<filtChunk.getSamples(); j++) {
              filtered.set(filtChunk.get(j, 0), (CHANNEL_SIZE/NUM_PROCESS_FRAMES)*i+j, 0);
              filtered.set(filtChunk.get(j, 1), (CHANNEL_SIZE/NUM_PROCESS_FRAMES)*i+j, 1);
            }
          }

          outPort.write(filtered);
      }

}

bool audioFilteringRateThread::processing(){
    // here goes the processing...
    return true;
}


void audioFilteringRateThread::threadRelease() {
    // nothing

}
void initializeAudioProcessing(SphericalPointf direction) {
  webrtc::Config config;

  std::vector<Point> array_geometry;

  //Mic distance no 
  array_geometry.push_back(Point(-(micDistance/2), 0.f, 0.f));
  array_geometry.push_back(Point( (micDistance/2), 0.f, 0.f));

  beamformer = new NonlinearBeamformer(array_geometry, direction);
  beamformer->Initialize(10, samplingRate);
  beamformer->AimAt(direction);

  apm = AudioProcessing::Create(config);

  analog_level = apm->gain_control()->stream_analog_level();

  apm->high_pass_filter()->Enable(true);

  apm->echo_cancellation()->enable_drift_compensation(false);
  apm->echo_cancellation()->Enable(true);

  apm->noise_suppression()->set_level(apm->noise_suppression()->kHigh);
  apm->noise_suppression()->Enable(true);

  apm->gain_control()->set_analog_level_limits(0, 255);
  apm->gain_control()->set_mode(apm->gain_control()->kAdaptiveAnalog);
  apm->gain_control()->Enable(true);

  //apm->voice_detection()->Enable(true);
}

 Sound processFrame(Sound input) {
   //std::cout << input.getSamples() << std::endl;

   int frameDataSize = input.getSamples() * input.getChannels();

    apm->set_stream_delay_ms(0);
    apm->gain_control()->set_stream_analog_level(analog_level);

   int16_t rawBuffer[frameDataSize];
   float floatBuffer[frameDataSize];
   ChannelBuffer<float> channelBuffer(input.getSamples(), input.getChannels());
   ChannelBuffer<float> filteredChannelBuffer(input.getSamples(), input.getChannels());

   float filteredBuffer[frameDataSize];
   int16_t filteredIntBuffer[frameDataSize];

   for(int i=0; i<frameDataSize; i++) {
     if(i%2==0)
        rawBuffer[i] = input.get(i/2, 0);
     else
        rawBuffer[i] = input.get(i/2, 1);
   }

   //convert to ChannelBuffer<float>
   S16ToFloat(rawBuffer, frameDataSize, floatBuffer);
   Deinterleave(floatBuffer, input.getSamples(), input.getChannels(), channelBuffer.channels());

   StreamConfig inputConfig(SAMPLE_RATE, input.getChannels(), false);
   StreamConfig outputConfig(SAMPLE_RATE, input.getChannels(), false);

   //process
   //beamformer->ProcessChunk(channelBuffer, &channelBuffer);

   if ((webrtcErr = apm->ProcessStream(channelBuffer.channels(), inputConfig, outputConfig, filteredChannelBuffer.channels())) < 0)
    std::cerr << "error processing stream: " << webrtcErr << " " << AudioProcessing::kNoError << std::endl;

   analog_level = apm->gain_control()->stream_analog_level();
   //run beamformer


   //convert back from ChannelBuffer<float> to int
   Interleave(filteredChannelBuffer.channels(), input.getSamples(), 1, filteredBuffer);
   FloatToS16(filteredBuffer, input.getSamples(), filteredIntBuffer);


   for(int i=0; i<input.getSamples(); i++) {
       input.set(filteredIntBuffer[i], i, 0);
       input.set(filteredIntBuffer[i], i, 1);
   }

   return input;
 }
