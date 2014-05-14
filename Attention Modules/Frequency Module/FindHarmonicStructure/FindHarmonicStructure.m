%a utility to localize the peaks in the fft for an individual speaker:
%this is a crude way to do simple recognition and noise rejection

%over successive frames, gather 5 highest peaks in the fft
numPeaks=500;
numFrames=100; %number of frames to sample the fft over

FrequencyConfigureAudioParameters;

%memory map some files
global audioD;
global sampleD;
[audioD,sampleD]=OpenAudioInputData;
tempMostRecentSample=sampleD.Data(1,1).f;
currentFrameIndex = tempMostRecentSample - (P.frameDuration_samples - 1) - P.fixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
currentFrameTime = tic;  %grab the current time

nFFT=2 ^ nextpow2(P.frameDuration_samples);

%preset some graphing stuff
% subplot(2,1,1);
% plot(freqs(1:2000),leftP(1:2000)); %only plot frequencies up to some upper bound...this is hardcoded but could be initialized...but it depends on the frame size
% subplot(2,1,2);
% plot(freqs(1:2000),rightP(1:2000));
% drawnow;


doneLooping=0;
exceedsThreshold=0;

leftPs=zeros(numFrames, nFFT/2+1);
rightPs=zeros(numFrames,nFFT/2+1);
loopIndex=1;



while (loopIndex<numFrames)  %loop continuously handling audio in a pitchy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
    t=tic;
    %a simple trigger
    %check if this frame exceeds amplitude threshold, if it doesn't don't
    %do anything with it
    exceedsThreshold=0;
    if(max(frame(1,:) > P.peakThreshold) || max(frame(2,:))>P.peakThreshold)
        exceedsThreshold=1;  
    end
    
   
    if exceedsThreshold==1 
    
        [leftP,rightP,freqs]=ComputePeriodogram(frame,P.sampleRate,nFFT);
       
        t=tic;
        
        leftPs(loopIndex,:)=leftP;
        rightPs(loopIndex,:)=rightP;
        
        display('saved periodograms');
        
        subplot(2,1,1);
        plot(freqs(1:2000),leftP(1:2000)); %only plot frequencies up to some upper bound...this is hardcoded but could be initialized...but it depends on the frame size
        subplot(2,1,2);
        plot(freqs(1:2000),rightP(1:2000));
        drawnow;
        display(toc(t));
      
        loopIndex=loopIndex+1;
    end
  
    
end