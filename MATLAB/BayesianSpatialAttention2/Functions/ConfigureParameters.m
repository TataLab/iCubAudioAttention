function [ P ] = ConfigureParameters( ~ )
%Set up all (or most) paramters and settings that you'll need for your
%system here and return everything in a convenient struct


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  note also that the frame size in samples determines the effective "refresh
%rate" of the localizer because it can't get ahead of the audiocapture
%thread so it must wait

P.c=336.628;%define speed of sound in m/s (to be very accurate, adjust for elevation (lethbridge is at ~950m)
P.D=0.14; %define distance between microphones in m
P.sampleRate = 48000;
P.nMics=2;
display(['please note sampling rate is set to ' num2str(P.sampleRate) ' check your settings to be sure']);

P.frameDuration_samples = 4096; %divide by sample rate to get frame duration, make it evenly divisible by the frame size of the audio capture, at least as big as the frame size of audio capture, and (ideally) a power of 2
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 
P.requiredLag_samples=0; %it might be necessary in some cases to imposes a lag behind real-time
P.numFramesInBuffer=1;  %how big of an echoic memory should we have
P.subFrameDuration_samples=512;  %if you want to redivide 4096 samples into smaller frames


%%%%%%%%%%%%%%%%
%some parameters for localizing
%%%%%%%%%%%%%
P.nBands=50;  %if you're stream pre-filtered audio from AudioCaptureFilterBank_YARP then the number of bands needs to match!
P.nBeamsPerHemifield=ceil( (P.D/P.c)*P.sampleRate ); %maximum lag in samples x2 (to sweep left and right of midline)
P.nBeams=2*P.nBeamsPerHemifield+1; %+1 includes the centre beam 
P.lags=(P.c/P.sampleRate).* (-P.nBeamsPerHemifield:P.nBeamsPerHemifield); %linear spaced distances corresponding to lags in seconds
P.angles=real(asin( (1/P.D) .* P.lags )) ; %nonlinear angles (in radians) that correspond to the lags

P.low_cf=100; % center frequencies based on Erb scale
P.high_cf=5000;
P.cfs = MakeErbCFs2(P.low_cf,P.high_cf,P.nBands);  %for ERB spaced cfs
%P.cfs = linspace(P.low_cf,P.high_cf,P.nBands); %for linear spaced cfs
P.frameOverlap = 0;  %this gets a bit confusing: we need to pull enough data so we can run beamformer lags *past* the end of each frame
P.sizeFramePlusOverlap=P.frameDuration_samples+(P.frameOverlap*2); %this is the total size of the chunk of data we need to pull out of the buffer each time we read it
P.frameIndices=P.frameOverlap+1:(P.frameOverlap+1) + P.frameDuration_samples - 1; %the indices of the "core" frame inside the grabbed audio data

%%%%%%%%%
%for computing delta spectrum (i.e. spectrotemporal changes) we need to
%buffer frames over time.  Set up some parameters to control this
%%%%%%%%%
% P.nPastSeconds = 1.5;  %in seconds; how much time over which to integrate previous events
% P.nPastFrames=floor(P.nPastSeconds/P.frameDuration_seconds);

%%%%%
%Stimulus driven attention can capture attention.  Set a threshold
%%%%%
P.attentionCaptureThreshold=100;  % a minimum salience that must be exceeded for attention to be captured...this is environment sensitive and must be tuned
P.inhibitionOfCapture=10*P.frameDuration_seconds; %in seconds, min amount of time between successive captures



%%%%%%
%parameters for interacting with memory mapped input audio
%%%%%
memMapFileName_input='/tmp/AudioMemMap.tmp';
inputDir=dir(memMapFileName_input);
P.bufferSize_bytes = inputDir.bytes; %the  buffer size is determined by your audio capture method.  Frames on that side are hard coded to be 4096 samples.  There are 4 rows by 4096 doubles x some number of frames in the  buffer.
P.bufferSize_samples = P.bufferSize_bytes / (8*4); %each sample is a 4 x 64-bit column (two audio data samples, sequence and time)

P.rawAudio  = memmapfile(memMapFileName_input, 'Writable', false, 'format',{'double' [4 P.bufferSize_samples] 'audioD'});

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
P.numMicAngles=length(P.micAngles);

%setup for memory mapping output audio space images
memMapFileName_output='/tmp/AudioMemSpace.tmp';
outputMapSize=(P.numSpaceAngles*P.nBands+2)*8; %doubles, the plus 2 is for the time and sequence stamps
fid=fopen(memMapFileName_output,'w');
fwrite(fid,zeros(1,outputMapSize),'double');
fclose(fid);
P.auditorySpace=memmapfile('/tmp/audioMemSpace.tmp','Writable',true,'format',{'double',[1 2],'timeStamp';'double', [P.nBands P.numSpaceAngles],'data'});




end

