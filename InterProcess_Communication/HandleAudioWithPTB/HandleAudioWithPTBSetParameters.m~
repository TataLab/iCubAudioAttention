%preconfigure a bunch of parameters and store them in a single struct to
%keep the desktop tidy

global P;

%We have to memory map two files:  audioDataDump is the file into which the
%iCub audio stream is being written.  mostRecentFrame is the (very small)
%file that holds the updated index to the most recent frame written
P.audioDataDumpFilename = '/tmp/AudioDataDump.dat';
P.mostRecentSampleFilename='/tmp/lastSampleIndex.dat';
P.numChannels=2;
%make a  struct to hold some parameters for convenience
P.kNumSamples = 1024;  %size of chunk of audio data to capture
P.kSampleRate = 48000; %the sampling frequency of the audio hardware
P.kFrameDuration_seconds = P.kNumSamples/P.kSampleRate; %how long each chunk takes in seconds
P.use='ptb';  % 'ptb for PsychToolBox's portaudio interface (very easy)  or 'yarp' for the iCub (um, not so easy)
P.sessionDuration_seconds = 60*30;  %how long will we run for (to preallocate the audio data memory);
P.sessionDuration_samples = P.sessionDuration_seconds * P.kSampleRate;
P.frameRate=P.kSampleRate/P.kNumSamples; %rate at which audio frames should be read
P.outputBitDepth_bytes=2;
P.inputBitDepth_bytes=3; %if you're using an interface that doesn't support 16-bit you'll need to convert