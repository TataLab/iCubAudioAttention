%Create a global struct to hold some audio parameters that need to be
%passed around globaly (this'll keep the code simple and the workspace
%clean)

global P;

P.sendAngleToYarp = 0;  %set to 1 to send angle over yarp network %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  This is independent of the size
%of the frames that are written to the audio dump file, as long as you stay
%"behind" the process that is writting the data from the iCub
%note though:  at a low-level it makes sense for this number to equate to
%an integer number of 4096 bytes (the memory page size of the OS)

P.sampleRate = 48000;
P.bitDepth_string = 'int16'; %16-bits = 2 bytes
P.bitDepth_bytes = 3;
P.numChannels=2; %stereo
P.frameDuration_samples = 10240; %@48000 hz stereo 16-bit samples 10240 =  213 ms
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 
P.frameRate = 1/P.frameDuration_seconds; %how often to compute angle in hz
P.fixedLag_samples = P.frameDuration_samples;  %how much this process should lag the audio write process

%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some parameters of the audio data file that is streaming from the
%iCub.  This is necessary for memory mapping.  It works to map a smaller
%region that the actual file, but not a larger region
%
%We have to memory map two files:  audioDataDump is the file into which the
%iCub audio stream is being written.  mostRecentFrame is the (very small)
%file that holds the updated index to the most recent frame written
P.audioDataDumpFilename = '/tmp/AudioDataDump.dat';
P.mostRecentSampleFilename='/tmp/lastSampleIndex.dat';

%%%%%%%%%%%%%%%%%%%%%%%%%%
%threshold detection
%on red iCub: P.peakThreshold = 3e5;  
%on /atom:
P.peakThreshold=10;

P.thresholdBounds = [500 2000]; %in hz, range of the periodogram to look for peaks above threshold

%%%%%%%%%%%%%%%%%%
%handle object integration/substitution
P.minTimeDelta=1; %seconds, how long between transients should we wait before a transient is registered as a new object
P.minTimeDelta_nanos=P.minTimeDelta*1000000000; %tic returns nanos on mac os
P.minTimeDelta_micros=P.minTimeDelta*1000000; %tic returns micros on linux

% %%%%%%%%%%%
% %for filtering
% %lowpass:
% 
% P.Fp_hz=5000;
% P.Fst_hz=6000;
% P.Ap=10;
% P.Ast=40;
% P.H=MakeLowPassFilter;
