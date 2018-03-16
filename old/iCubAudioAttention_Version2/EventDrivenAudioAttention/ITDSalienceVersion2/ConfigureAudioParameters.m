%Create a global struct to hold some audio parameters that need to be
%passed around globaly (this'll keep the code simple and the workspace
%clean)

display(['Setting up parameters for iCub Audio Attention using: ' mfilename('fullpath')]);

global P;

P.sendAngleToYarp = 0;  %set to 1 to send angle over yarp network %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  note also that the frame size in samples determines the effective "refresh
%rate" of the localizer because it will wait for a new complete frame to
%come in before finding a new angle

P.c=340.29;%define speed of sound in m/s
P.D=0.1475; %define distance between microphones in m
P.sampleRate = 48000;
P.bitDepth_string = 'int16'; %16-bits = 2 bytes
P.bitDepth = 24;
P.numChannels=2; %stereo
P.frameDuration_samples = 2^13; %@48000 hz stereo 16-bit samples 10240 =  213 ms
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 
P.frameRate = 1/P.frameDuration_seconds; %how often to compute angle in hz


%%%%%%%%%%%%%%%%
%some parameters for localizing
%%%%%%%%%%%%%
P.nBands=45;
P.nAnglesPerHemifield=floor( (P.D/P.c)*P.sampleRate )-1; %maximum lag in samples x2 (to sweep left and right of midline)
P.nAngles=2*P.nAnglesPerHemifield+1; %+1 includes the centre beam 
P.lags=(P.c/P.sampleRate).* (-P.nAnglesPerHemifield:P.nAnglesPerHemifield); %linear spaced distances corresponding to lags in seconds
P.angles=asin( (1/P.D) .* P.lags ) ; %nonlinear angles (in radians) that correspond to the lags
P.low_cf=500; % center frequencies based on Erb scale
P.high_cf=5000;
P.cfs = MakeErbCFs(P.low_cf,P.high_cf,P.nBands);
P.align=true; %set this to true to realign time after filtering (Important!)

%%%%%%%%%
%the most important parameter
%%%%%%%%%

P.nPast_seconds = .5;  %in seconds; how much time over which to integrate previous events
P.nPast_frames=floor(P.nPast_seconds/P.frameDuration_seconds);

P.attentionCaptureThreshold=0;

