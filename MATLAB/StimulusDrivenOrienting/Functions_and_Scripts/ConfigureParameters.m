function [ P ] = ConfigureParameters( ~ )
%Set up all (or most) paramters and settings that you'll need for your
%system here and return everything in a convenient struct


display(['Setting up parameters for iCub Audio Attention using: ' mfilename('fullpath')]);

P.sendAngleToYarp = 1;  %set to 1 to send angle over yarp network %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  note also that the frame size in samples determines the effective "refresh
%rate" of the localizer because it can't get ahead of the audiocapture
%thread so it must wait

P.c=340.29;%define speed of sound in m/s
P.D=0.145; %define distance between microphones in m
P.sampleRate = 48000;

P.frameDuration_samples = 2^12; %@48000 hz stereo 16-bit samples 10240 =  213 ms
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 

%%%%%%%%%%%%%%%%
%some parameters for localizing
%%%%%%%%%%%%%
P.nBands=5;
P.nBeamsPerHemifield=floor( (P.D/P.c)*P.sampleRate )-1; %maximum lag in samples x2 (to sweep left and right of midline)
P.nBeams=2*P.nBeamsPerHemifield+1; %+1 includes the centre beam 
P.lags=(P.c/P.sampleRate).* (-P.nBeamsPerHemifield:P.nBeamsPerHemifield); %linear spaced distances corresponding to lags in seconds
P.angles=asin( (1/P.D) .* P.lags ) ; %nonlinear angles (in radians) that correspond to the lags
P.low_cf=440; % center frequencies based on Erb scale
P.high_cf=5000;
P.cfs = MakeErbCFs2(P.low_cf,P.high_cf,P.nBands);


%%%%%%%%%
%for computing delta spectrum (i.e. spectrotemporal changes) we need to
%buffer frames over time.  Set up some parameters to control this
%%%%%%%%%

P.nPastSeconds = .25;  %in seconds; how much time over which to integrate previous events
P.nPastFrames=floor(P.nPastSeconds/P.frameDuration_seconds);

%%%%%
%Stimulus driven attention can capture attention.  Set a threshold
%%%%%
P.attentionCaptureThreshold=50;

end

