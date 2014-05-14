%DetectFrequencyTransients buffers two frames of audio data and compares
%their spectra

FrequencyTransientsAudioParameters;

%memory map some files
global audioD;
global sampleD;
[audioD,sampleD]=OpenAudioInputData;
tempMostRecentSample=sampleD.Data(1,1).f;
currentFrameIndex = tempMostRecentSample - (P.frameDuration_samples - 1) - P.fixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
%currentFrameIndex=48000; %use this line to read file "off line" by starting at second second
currentFrameTime = tic;  %grab the current time

nFFT=2 ^ nextpow2(P.frameDuration_samples);
leftP=zeros(nFFT/2+1,1);  %-2 because we won't plot the points corresponding to the DC (first) or the nyquist (last)
rightP=zeros(nFFT/2+1,1);
leftDeltaP=zeros(nFFT/2+1,1); %hold the difference between spectra across time
rightDeltaP=zeros(nFFT/2+1,1);
freqs=zeros(nFFT/2-1+1,1);


doneLooping=0;
exceedsThreshold=0;

while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
    t=tic;

    
    [leftP,rightP,leftDeltaP,rightDeltaP,freqs, exceedsThreshold]=ComputePeriodogramTransients(frame,leftP,rightP,P.sampleRate,nFFT);
 
    
    %interpret the threshold detection flag
    if(exceedsThreshold==1)
        %display('Exceeded threshold');
        new=GetNewEmptyObject;
        new.name='frqObjxx';
        new.timeStamp=tic;
        didAddObject=AddNewObject(new);
        %display(didAddObject);
        %add a new object to the stack 
    end
    
    
%     
%     subplot(2,1,1);
%     plot(freqs(1:500),leftDeltaP(1:500)); %only plot frequencies up to some upper bound...this is hardcoded but could be initialized...but it depends on the frame size
%     ylim([-300 300]);
%     
%     subplot(2,1,2);
%     plot(freqs(1:500),rightDeltaP(1:500));
%     ylim([-300 300]);
%     
%     drawnow;

    

   
end