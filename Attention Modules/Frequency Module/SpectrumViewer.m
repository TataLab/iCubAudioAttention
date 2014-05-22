%Simple code to grab audio and display its current fft

%set our parameters (notice this calls a differen "configure" script that
%than the spatial module...because it's running in a different instance of
%MATLAB

FrequencyConfigureAudioParameters;

%memory map some files
global audioD;
global sampleD;
[audioD,sampleD]=OpenAudioInputData;
tempMostRecentSample=sampleD.Data(1,1).f;
currentFrameIndex = tempMostRecentSample - (P.frameDuration_samples - 1) - P.fixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
currentFrameTime = tic;  %grab the current time

nFFT=2 ^ nextpow2(P.frameDuration_samples);
leftP=zeros(1,nFFT/2-1);  %-2 because we won't plot the points corresponding to the DC (first) or the nyquist (last)
rightP=zeros(1,nFFT/2-1);
freqs=zeros(1,nFFT/2-1);

%preset some graphing stuff
subplot(2,1,1);
plot(freqs(1:2000),leftP(1:2000)); %only plot frequencies up to some upper bound...this is hardcoded but could be initialized...but it depends on the frame size
subplot(2,1,2);
plot(freqs(1:2000),rightP(1:2000));
drawnow;


doneLooping=0;
exceedsThreshold=0;



while (~doneLooping)  %loop continuously handling audio in a pitchy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
    t=tic;
    %a simple trigger
    %check if this frame exceeds amplitude threshold, if it doesn't don't
    %do anything with it
    exceedsThreshold=1;
    if(max(frame(1,:) > P.peakThreshold) || max(frame(2,:))>P.peakThreshold)
        exceedsThreshold=1;  
    end
    
    %check and update the angle to the source if necesasary
    if exceedsThreshold==1 
    
        [leftP,rightP,freqs]=ComputePeriodogram(frame,P.sampleRate,nFFT);
        
        %display(['computed ffts in ' num2str(toc(t)) ' seconds']);  
        
        subplot(2,1,1);
        plot(freqs(1:2000),leftP(1:2000)); %only plot frequencies up to some upper bound...this is hardcoded but could be initialized...but it depends on the frame size
        subplot(2,1,2);
        plot(freqs(1:2000),rightP(1:2000));
        drawnow;
        
         
    end
  
    
end