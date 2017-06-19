#include <stdio.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/AudioGrabberInterfaces.h>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/api.h>
#include <iostream>

#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/NetInt32.h>
#include <yarp/sig/Sound.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Vector.h>

#include "webrtc/modules/audio_processing/include/audio_processing.h"
#include "webrtc/modules/interface/module_common_types.h"
#include "webrtc/common_audio/channel_buffer.h"
#include "webrtc/modules/audio_processing/beamformer/nonlinear_beamformer.h"
#include "webrtc/common_audio/include/audio_util.h"


using namespace webrtc;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

float PI = acos(-1.0);
const int CHANNEL_SIZE = 16000;
const int SAMPLE_RATE = 16000;
const int NUM_CHANNELS = 2;
const int bufferSize = CHANNEL_SIZE*NUM_CHANNELS;
const int NUM_PROCESS_FRAMES=100;

int analog_level;
int delay_ms = 20;
int voiceDetected = 0;
int webrtcErr = 0;

NonlinearBeamformer* beamformer;
AudioProcessing* apm;


void initializeAudioProcessing(SphericalPointf);
Sound processFrame(Sound input);

int main(int argc, char *argv[]) {
    // Open the network

    SphericalPointf direction(PI/2.0f, 0.0f, 10.0f);

    initializeAudioProcessing(direction);

    Network yarp;
    BufferedPort<Sound> inPort;
    Port outPort;

    inPort.open("/receiver");
    outPort.open("/filtered");

    Network::connect("/grabber", "/receiver");
    // Get an audio write device.
    // Property conf;
    // conf.put("device","portaudio");
    // conf.put("samples", "4096");
    // conf.put("write", "1");
    //
    // PolyDriver poly(conf);
    // IAudioRender *put;
    // // Make sure we can write sound
    //
    // poly.view(put);
    // if (put==NULL) {
    //     printf("cannot open interface\n");
    //     return 1;
    // }
    //Receive and render
    Sound *s;
    while (true) {

      s = inPort.read(false);

      if (s!=NULL) {
          //Sound filtered = processFrame(*s);
          //std::cout << s->getSamples() << std::endl;
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

    delete beamformer;
    delete apm;
    return 0;
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


void initializeAudioProcessing(SphericalPointf direction) {
  webrtc::Config config;

  std::vector<Point> array_geometry;

  array_geometry.push_back(Point(-0.04f, 0.f, 0.f));
  array_geometry.push_back(Point( 0.04f, 0.f, 0.f));

  beamformer = new NonlinearBeamformer(array_geometry, direction);
  beamformer->Initialize(10, SAMPLE_RATE);
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
