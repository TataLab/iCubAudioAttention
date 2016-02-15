function [ P ] = ConfigureParameters( ~ )
%Set up all (or most) paramters and settings that you'll need for your
%system here and return everything in a convenient struct


display(['Setting up parameters for iCub Audio Attention using: ' mfilename('fullpath')]);

P.sendAngleToYarp = 0;  %set to 1 to send angle over yarp network %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');
P.audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  note also that the frame size in samples determines the effective "refresh
%rate" of the localizer because it can't get ahead of the audiocapture
%thread so it must wait

P.c=336;%define speed of sound in m/s (to be very accurate, adjust for elevation (lethbridge is at ~950m)
P.D=0.145; %define distance between microphones in m
P.sampleRate = 44100;
P.nMics=2;
display(['please note sampling rate is set to ' num2str(P.sampleRate) ' (iCub streams audio at 48K)']);

P.frameDuration_samples = 2^12; %@48000 hz stereo 16-bit samples 10240 =  213 ms
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 
P.requiredLag_frames=0; %it might be necessary in some cases to imposes a lag behind real-time
P.numFramesInBuffer=20;  %how big of an echoic memory should we have

%%%%%%%%%%%%%%%%
%some parameters for localizing
%%%%%%%%%%%%%
P.nBands=64;  %if you're stream pre-filtered audio from AudioCaptureFilterBank_YARP then the number of bands needs to match!
P.nBeamsPerHemifield=ceil( (P.D/P.c)*P.sampleRate ); %maximum lag in samples x2 (to sweep left and right of midline)
P.nBeams=2*P.nBeamsPerHemifield+1; %+1 includes the centre beam 
P.lags=(P.c/P.sampleRate).* (-P.nBeamsPerHemifield:P.nBeamsPerHemifield); %linear spaced distances corresponding to lags in seconds
P.angles=real(asin( (1/P.D) .* P.lags )) ; %nonlinear angles (in radians) that correspond to the lags
P.low_cf=50; % center frequencies based on Erb scale
P.high_cf=10000;
P.cfs = MakeErbCFs2(P.low_cf,P.high_cf,P.nBands);
P.frameOverlap = 0;  %this gets a bit confusing: we need to pull enough data so we can run beamformer lags *past* the end of each frame
P.sizeFramePlusOverlap=P.frameDuration_samples+(P.frameOverlap*2); %this is the total size of the chunk of data we need to pull out of the buffer each time we read it
P.frameIndices=P.frameOverlap+1:(P.frameOverlap+1) + P.frameDuration_samples - 1; %the indices of the "core" frame inside the grabbed audio data


%%%%%%%%%
%for computing delta spectrum (i.e. spectrotemporal changes) we need to
%buffer frames over time.  Set up some parameters to control this
%%%%%%%%%
P.nPastSeconds = .5;  %in seconds; how much time over which to integrate previous events
P.nPastFrames=floor(P.nPastSeconds/P.frameDuration_seconds);

%%%%%
%Stimulus driven attention can capture attention.  Set a threshold
%%%%%
P.attentionCaptureThreshold=100;  % a minimum salience that must be exceeded for attention to be captured...this is environment sensitive and must be tuned
%P.salienceGain = 2; %"stickiness of attention": use this to prevent a second frame from capturing attention from the previous frame
%%%%%%
%parameters for interacting with memory mapped audio
%%%%%

%for reading unfiltered memmapped audio
memMapFileName=[P.audioAttentionRoot '/data/AudioMemMap.tmp'];
f=dir(memMapFileName);
P.bufferSize_bytes = f.bytes; %the  buffer size is determined by AudioCapture_YARP.  Frames on that side are hard coded to be 4096 samples.  There are 4 rows by 4096 doubles x some number of frames in the  buffer.
P.bufferSize_samples = P.bufferSize_bytes / (8*4); %each sample is a 4 x 64-bit column (two audio data samples, sequence and time)

P.audioIn  = memmapfile(memMapFileName, 'Writable', false, 'format',{'double' [4 P.bufferSize_samples] 'audioD'});

%%%%%
%Parameters for tuning the filterbank using PCA and envelope binding
%%%%%

P.tuningDuration_seconds=5; %aproximately how long to record pre-calibration in seconds
P.tuningDuration_frames =floor(P.tuningDuration_seconds/P.frameDuration_seconds); %exactly how many frames is this
P.varExplainedCriteria=75;  %percent of variance required to be explained by PCA and cross-spectrum envelope binding


%%%%%
%Parameters for building a probabalistic map of space
%Note there are two independent but overlapping spaces.
%These parameters are independent of all the coordinates fixed to the
%microphone array.  
%%%%
P.radialResolution_degrees=1; %degrees per possible head direction
P.radialResolution_radians=pi/180 * P.radialResolution_degrees;
P.spaceAngles=-pi/2:P.radialResolution_radians:3*pi/2-P.radialResolution_radians; %the angles that point to external space
P.numSpaceAngles=length(P.spaceAngles);
P.micAngles=[P.angles pi+P.angles(2:end-1)]; %all the possible microphone coordinate angles, all the way around

if(isempty(dir('./Functions_and_Scripts/beamPattern.mat'))) %if we don't have a saved beam pattern then we have to build it
    beamPattern=BuildBeamPattern(P); %compute all the sensitivities of the different steering angles for different arrival angles
    save('./Functions_and_Scripts/beamPattern.mat','beamPattern'); %save it for next time
    P.beamPattern=beamPattern;
else
    load('./Functions_and_Scripts/beamPattern.mat');
    P.beamPattern=beamPattern;
end

if(isempty(dir('./Functions_and_Scripts/noiseFloor.mat'))) %if we don't have a saved beam pattern then we have to build it
    noiseFloor=BuildNoiseFloor(P,'/Users/Matthew/Documents/Robotics/iCubAudioAttention/data/sounds/20-Fan-1sec_44100hz.wav'); %use pre-recorded noise to get an estimate of the false alarm probability
    save('./Functions_and_Scripts/noiseFloor.mat','noiseFloor'); %save it for next time
    P.noiseFloor=noiseFloor;
else
    load('./Functions_and_Scripts/noiseFloor.mat');
    P.noiseFloor=noiseFloor;
end

P.evidenceRatios=ComputeEvidenceRatio(P); %precompute the ratio of P(B|A) to P(B)


end

