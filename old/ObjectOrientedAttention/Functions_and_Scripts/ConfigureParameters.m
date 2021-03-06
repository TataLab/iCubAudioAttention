function [ P ] = ConfigureParameters( ~ )
%Set up all (or most) paramters and settings that you'll need for your
%system here and return everything in a convenient struct


display(['Setting up parameters for iCub Audio Attention using: ' mfilename('fullpath')]);

P.sendAngleToYarp = 1;  %set to 1 to send angle over yarp network %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');
P.audioAttentionRoot='~/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  note also that the frame size in samples determines the effective "refresh
%rate" of the localizer because it can't get ahead of the audiocapture
%thread so it must wait

P.c=340.29;%define speed of sound in m/s
P.D=0.145; %define distance between microphones in m
P.sampleRate = 44100;

P.frameDuration_samples = 2^14; %@48000 hz stereo 16-bit samples 10240 =  213 ms
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 
P.requiredLag_frames=0; %it might be necessary in some cases to imposes a lag behind real-time

%%%%%%%%%%%%%%%%
%some parameters for localizing
%%%%%%%%%%%%%
P.nBands=100;
P.nBeamsPerHemifield=floor( (P.D/P.c)*P.sampleRate )-1; %maximum lag in samples x2 (to sweep left and right of midline)
P.nBeams=2*P.nBeamsPerHemifield+1; %+1 includes the centre beam 
P.lags=(P.c/P.sampleRate).* (-P.nBeamsPerHemifield:P.nBeamsPerHemifield); %linear spaced distances corresponding to lags in seconds
P.angles=round(asin( (1/P.D) .* P.lags ) ); %nonlinear angles (in radians) that correspond to the lags
P.low_cf=50; % center frequencies based on Erb scale
P.high_cf=2000;
P.cfs = MakeErbCFs2(P.low_cf,P.high_cf,P.nBands);
P.frameOverlap = P.nBeamsPerHemifield;  %this gets a bit confusing: we need to pull enough data so we can run beamformer lags *past* the end of each frame
P.sizeFramePlusOverlap=P.frameDuration_samples+(P.frameOverlap*2); %this is the total size of the chunk of data we need to pull out of the buffer each time we read it




%%%%%%%%%
%for computing delta spectrum (i.e. spectrotemporal changes) we need to
%buffer frames over time.  Set up some parameters to control this
%%%%%%%%%
P.nPastSeconds = 1;  %in seconds; how much time over which to integrate previous events
P.nPastFrames=floor(P.nPastSeconds/P.frameDuration_seconds);

%%%%%
%Stimulus driven attention can capture attention.  Set a threshold
%%%%%
P.attentionCaptureThreshold=5;
P.maxObjectLifetime=1; %number of seconds to hold object file 

%%%%%%
%parameters for interacting with memory mapped audio
%%%%%
memMapFileName=[P.audioAttentionRoot '/data/AudioMemMap.tmp'];
f=dir(memMapFileName);
P.bufferSize_bytes = f.bytes; %the  buffer size is determined by AudioCapture_YARP.  Frames on that side are hard coded to be 4096 samples.  There are 4 rows by 4096 doubles x some number of frames in the  buffer.
P.bufferSize_samples = P.bufferSize_bytes / (8*4); %each sample is a 4 x 64-bit column (two audio data samples, sequence and time)

P.audioIn  = memmapfile(memMapFileName, 'Writable', false, 'format',{'double' [4 P.bufferSize_samples] 'audioD'});


end

