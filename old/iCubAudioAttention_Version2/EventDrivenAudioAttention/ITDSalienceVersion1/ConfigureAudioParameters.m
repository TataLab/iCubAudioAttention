%Create a global struct to hold some audio parameters that need to be
%passed around globaly (this'll keep the code simple and the workspace
%clean)

display(['Setting up parameters for iCub Audio Attention using: ' mfilename('fullpath')]);

global P;

P.sendAngleToYarp = 0;  %set to 1 to send angle over yarp network %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%set up some timing parameters regarding reading audio
%specify the size of the frame in samples.  This is independent of the size
%of the frames that are written to the audio dump file, as long as you stay
%"behind" the process that is writting the data from the iCub
%note though:  at a low-level it makes sense for this number to equate to
%an integer number of 4096 bytes (the memory page size of the OS)

%note also that the frame size in samples determines the effective "refresh
%rate" of the localizer because it will wait for a new complete frame to
%come in before finding a new angle

P.sampleRate = 48000;
P.bitDepth_string = 'int16'; %16-bits = 2 bytes
P.bitDepth_bytes = 3;
P.numChannels=2; %stereo
P.frameDuration_samples = 2^13; %@48000 hz stereo 16-bit samples 10240 =  213 ms
P.frameDuration_seconds = P.frameDuration_samples/P.sampleRate; 
P.frameRate = 1/P.frameDuration_seconds; %how often to compute angle in hz
P.fixedLag_samples =P.frameDuration_samples;% P.frameDuration_samples;  %how much this process should lag the audio write process
P.c=340.29;%define speed of sound in m/s

P.D=0.1475; %define distance between microphones in m
P.ITDWindow=ceil(P.sampleRate*P.D/P.c)*2;  %should be divisible by 2...this is how many samples to look on each side of the midline for a peak...this should be related to the distance between the microphones
                            %for a 12 cm distance it is only linear for about 15
                             %samples on eithe side of the midline and 17 samples
                            %pins it at 90 degress.

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
%parameters for spatial section
%threshold detection
P.xcorrPeakThreshold =10;  %only adjust angle if signal exceeds this %use 4e7 for icub in room
P.numLagSpaces=10; %how many vectors of frames to 

%Parameters for frequency section
%threshold detection
P.peakThreshold=5;
P.thresholdBounds = [200 1000]; %in hz, range of the periodogram to look for peaks above threshold
P.nfft=2^12;
[~,P.periodogramFreqs]=periodogram(rand(1,P.frameDuration_samples),[],P.nfft,P.sampleRate,'one-sided'); %call periodogram once on random data just to initialize it's outputs
P.frequencyWindow=find((P.periodogramFreqs>=P.thresholdBounds(1)) & (P.periodogramFreqs<=P.thresholdBounds(2)));
P.frequencyWindow=P.frequencyWindow';%reorient for sanity
P.freqs=P.periodogramFreqs(P.frequencyWindow)';%reorient for sanity
P.numPGrams=10; %how many frames over which to compute mean and variance


%handle object integration/substitution
P.minTimeDelta=.75; %seconds, how long between transients should we wait before a transient is registered as a new object
P.minTimeDelta_nanos=P.minTimeDelta*1000000000; %tic returns nanos on mac os
P.minTimeDelta_micros=P.minTimeDelta*1000000; %tic returns micros on linux
%%%%%%%%%%%%%%


%parameters to control weighting of the lag vector, from inside toward
%center
outsideFactor=1; %how much to multiply values at eccentric lags(set this to 1 if you don't want any scaling)
insideFactor=1; %at the midline
P.weightsV=linspace(outsideFactor,insideFactor,P.ITDWindow/2); %a linear weighting vector
clear outsideFactor;
clear insideFactor;
