function [ P ] = ConfigureParameters( ~ )
%Set up all (or most) paramters and settings that you'll need for your
%system here and return everything in a convenient struct


display(['Setting up parameters for iCub Audio Attention using: ' mfilename('fullpath')]);

P.sendAngleToYarp = 1;  %set to 1 to send angle over yarp network %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');
P.audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  note also that the frame size in samples determines the effective "refresh
%rate" of the localizer because it can't get ahead of the audiocapture
%thread so it must wait

P.c=336;%define speed of sound in m/s (to be very accurate, adjust for elevation (lethbridge is at ~950m)
P.D=0.145; %define distance between microphones in m
P.sampleRate = 48000;
P.nMics=2;
display(['please note sampling rate is set to ' num2str(P.sampleRate) ' (iCub streams audio at 48K)']);

P.frameDuration_samples = 2^13; %divide by sample rate to get frame duration
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 
P.requiredLag_frames=0; %it might be necessary in some cases to imposes a lag behind real-time
P.numFramesInBuffer=20;  %how big of an echoic memory should we have

%%%%%%%%%%%%%%%%
%some parameters for localizing
%%%%%%%%%%%%%
P.nBands=32;  %if you're stream pre-filtered audio from AudioCaptureFilterBank_YARP then the number of bands needs to match!
P.nBeamsPerHemifield=ceil( (P.D/P.c)*P.sampleRate ); %maximum lag in samples x2 (to sweep left and right of midline)
P.nBeams=2*P.nBeamsPerHemifield+1; %+1 includes the centre beam 
P.lags=(P.c/P.sampleRate).* (-P.nBeamsPerHemifield:P.nBeamsPerHemifield); %linear spaced distances corresponding to lags in seconds
P.angles=real(asin( (1/P.D) .* P.lags )) ; %nonlinear angles (in radians) that correspond to the lags
P.low_cf=300; % center frequencies based on Erb scale
P.high_cf=3000;
P.cfs = MakeErbCFs2(P.low_cf,P.high_cf,P.nBands);
P.frameOverlap = 0;  %this gets a bit confusing: we need to pull enough data so we can run beamformer lags *past* the end of each frame
P.sizeFramePlusOverlap=P.frameDuration_samples+(P.frameOverlap*2); %this is the total size of the chunk of data we need to pull out of the buffer each time we read it
P.frameIndices=P.frameOverlap+1:(P.frameOverlap+1) + P.frameDuration_samples - 1; %the indices of the "core" frame inside the grabbed audio data

%%%%%%%%%
%for computing delta spectrum (i.e. spectrotemporal changes) we need to
%buffer frames over time.  Set up some parameters to control this
%%%%%%%%%
P.nPastSeconds = 1.5;  %in seconds; how much time over which to integrate previous events
P.nPastFrames=floor(P.nPastSeconds/P.frameDuration_seconds);

%%%%%
%Stimulus driven attention can capture attention.  Set a threshold
%%%%%
P.attentionCaptureThreshold=100;  % a minimum salience that must be exceeded for attention to be captured...this is environment sensitive and must be tuned
P.inhibitionOfCapture=2.0; %in seconds, min amount of time between successive captures
%P.salienceGain = 2; %"stickiness of attention": use this to prevent a second frame from capturing attention from the previous frame
%%%%%%
%parameters for interacting with memory mapped audio
%%%%%

%for reading unfiltered memmapped audio
memMapFileName='/tmp/AudioMemMap.tmp';
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
P.numSpaceAngles=360/P.radialResolution_degrees;
P.spaceAngles=linspace(-pi,pi-P.radialResolution_radians,P.numSpaceAngles); %the angles that point to external space; midline is the middle of the vector
P.micAngles=[P.angles pi+P.angles(2:end-1)]; %all the possible microphone coordinate angles, all the way around
P.micAngles=circshift(P.micAngles,[0 P.nBeamsPerHemifield]); %rotate the microphone angles so that midline is in the middle
P.micAngles=unwrap(P.micAngles)-2*pi; %we don't want the ugly discontinuity.  Better to run smoothly from -pi:pi
if(isempty(dir('./Functions_and_Scripts/beamPattern.mat'))) %if we don't have a saved beam pattern then we have to build it
    [beamPattern,beamMatrix]=BuildBeamPattern(P); %compute all the sensitivities of the different steering angles for different arrival angles
    save('./Functions_and_Scripts/beamPattern.mat','beamPattern'); %save it for next time
    save('./Functions_and_Scripts/beamMatrix.mat','beamMatrix'); %save it for next time
    P.beamPattern=beamPattern;
    P.beamMatrix=beamMatrix;
else
    load('./Functions_and_Scripts/beamPattern.mat');
    load('./Functions_and_Scripts/beamMatrix.mat');

    P.beamPattern=beamPattern;
    P.beamMatrix=beamMatrix;
end

P.useNoiseFloor=0;
noiseFloorFileName='/Users/Matthew/Documents/Robotics/iCubAudioAttention/data/sounds/PinkNoise_2sec_0lag_44100hz.wav';
if P.useNoiseFloor==1 %should we take the time to build and use a model of the background noise?
    if(isempty(dir('./Functions_and_Scripts/PinkNoise_1sec_0lag_weak.mat'))) %if we don't have a saved beam pattern then we have to build it
        noiseFloor=BuildNoiseFloor(P,noiseFloorFileName); %use pre-recorded noise to get an estimate of the false alarm probability
        save('./Functions_and_Scripts/PinkNoise_2sec_0lag_44100hz.mat','noiseFloor'); %save it for next time
    else
        load('./Functions_and_Scripts/PinkNoise_2sec_0lag_44100hz.mat');
        P.noiseFloor=noiseFloor;
    end
else
    noiseFloor=ones(1,P.numSpaceAngles); %if not, just divide through by ones
end

P.noiseFloor=noiseFloor;

P.learnRate=1;

P.evidenceRatios=ComputeEvidenceRatio(P); %precompute the ratio of P(B|A) to P(B)

%setup for controlling a desktop motor with arduino
P.useDesktopRobot = 0;
if(P.useDesktopRobot)
    display('Setting up motor AdaFruit motor controller on Arduino'); 
    addpath('/Users/Matthew/Documents/Robotics/DesktopRobot/StepperController');
    P.motorControl=ConfigureArduino;
end
P.fov=45; %degrees
P.fov_pixels=320; 

P.fov_indices=floor(181-P.fov/2):ceil(181+P.fov/2);  %find the "middle" in micAligned space
P.fov_angles=P.spaceAngles(P.fov_indices);
P.fov_xIndices=linspace(P.fov_indices(1), P.fov_indices(end),P.fov_pixels);
P.fov_cameraResolutionY=240; %x and y dimensions in pixels

end

