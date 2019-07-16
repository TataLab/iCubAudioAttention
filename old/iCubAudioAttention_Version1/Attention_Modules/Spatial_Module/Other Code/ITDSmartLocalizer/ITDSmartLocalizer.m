%ITD Smart Localizer

%periodically grab audio data out of AudioD and then:
%1) load a pair of vectors that represent harmonic structure of interest

%2) boost those harmonics in the fft: not implemented yet

%3) compute the angle to the
%most prominent source using GCC-PHAT

ITDSmartLocalizerConfig;  %call the script that sets up the P parameter structure

if (P.sendAngleToYarp==1)   %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');
    %connect with Yarp network
    port = OpenYarpWritePort;
    display('press any key when ready');
    waitforbuttonpress;
end

%load the peak frequencies you want to boost
%eventually we'll use object files here
tempP=open('/Users/Matthew/Desktop/leftHarmonics.dat');
leftHarmonics=tempP.leftHarmonics;
tempP=open('/Users/Matthew/Desktop/rightHarmonics.dat');
rightHarmonics=tempP.rightHarmonics;
clear tempP;

%we're given a pair of arrays with 2 rows and n columns.  The top rows
%are the magnitudes of the FFT coefficients of the target spectrum,
%normalized to run between 0 and 1

%we need to map these n columns onto the appropriate region of the fft
%that's computed by SmartComputeAngle, which might be a different length
%and should be a different offset (that is, coefficent 1 in the
%harmonic structure vectors doesn't correspond to element 1 in the fft

%we use the frequencies (in hz) reported in the second row of the
%harmonics arrays to build new vectors with the right dimensions

%we precompute this since it's the same on every frame, but once this is
%adapted to real-time update the target object this has to go into the
%frame loop...probably in a subfunction that also maps the object file

L=P.frameDuration_samples/2+1;  %this will be the number of bins to the nyquist
binWidth=(P.sampleRate/2)/L; %hz per bin in the fft
firstBin=floor(binWidth*leftHarmonics(2,1));  %the frequency of the first bin in the harmonics arrays
lastBin=floor(binWidth*leftHarmonics(2,end));
harmonicsL=lastBin-firstBin+1; %how many bins *in the current fft* will the harmonics vector span

%now build new harmonics vectors with the appropriate length and offset and
%gain ... note we're given normalized coefficients
newLeftHarmonics=P.gain.*resample(leftHarmonics(1,:),harmonicsL,length(leftHarmonics(1,:)));
newRightHarmonics=P.gain.*resample(rightHarmonics(1,:),harmonicsL,length(rightHarmonics(1,:)));

%add a bottom row containing the indices for each coefficient
bottomRow=firstBin:lastBin;
newLeftHarmonics=[newLeftHarmonics;bottomRow];
newRightHarmonics=[newRightHarmonics;bottomRow];

doneLooping=0;
exceedsThreshold=0;

%%%%%%%%%%%%%%%%
%preset some plotting parameters if you want 
%to plot the angle
xlim([-1 1]);
ylim([-1 1]);
hold off;
x=1;
y=0;

previousAngle=0.0;  %scope this outside of the loop
newAngle = 0.0;
newAngle_deg=newAngle/pi  * (180);


%memory map the input-level file...this is the raw audio signal
%coming from the audio hardware...also initialize an index to keep track of
%which frame is the most recent available frame to read
global audioD;
global sampleD;
[audioD,sampleD]=OpenAudioInputData;
tempMostRecentSample=sampleD.Data(1,1).f;
%tempMostRecentSample=48000;  %uncomment if you want to work "offline" by reading data from the 2nd seconde of the audio dump file
currentFrameIndex = tempMostRecentSample - (P.frameDuration_samples - 1) - P.fixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
currentFrameTime = tic;  %grab the current time

%Mostly this is useful for streaming output
%silentFrame=zeros(P.outputNumChans,P.frameDuration_samples);  %we'll dump these zeros into a frame if it's below threshold and then stream this out


while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
    
    %     %a simple trigger
    %     %check if this frame exceeds amplitude threshold, if it doesn't don't
    %     %do anything with it
    %     exceedsThreshold=0;
    %     if(max(frame(1,:) > P.peakThreshold) || max(frame(2,:))>P.peakThreshold)
    %         exceedsThreshold=1;
    %         display('That frame exceeded threshold');
    %     end
    %
    
    
    [newAngle,gcc_inv_shifted,trigger]=SmartComputeAngle(frame,P.sampleRate,1,newLeftHarmonics,newRightHarmonics);  %compute the angle using a GCC-PHAT approach
    
    exceedsThreshold=0;
    if(trigger>2.4e6)
        exceedsThreshold=1;
    end
    
    if(exceedsThreshold==1)
        newAngle_deg=newAngle/pi  * (180);
        
        if (P.sendAngleToYarp==1)
            if(newAngle_deg>=-90.0 && newAngle_deg<=90.0)
                SendAngleToYarp(newAngle_deg,port);
            else
                display('The value of newAngle_deg was out of the range +/- 90.0 degrees');
            end
        end
        
        
        %make some pretty pictures
        %display(['current angle to audio source = ' num2str(newAngle_deg)]);
        [x,y] = pol2cart(newAngle,1); %convert angle and unit radius to cartesian
        figure(1);
        hold off;
        compass(x,y);
        drawnow;
        
    end
    
   
   
    
end