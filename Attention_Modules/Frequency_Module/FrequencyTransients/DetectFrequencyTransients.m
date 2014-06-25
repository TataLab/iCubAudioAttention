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

nFFT=2 ^ nextpow2(P.frameDuration_samples);
leftP=zeros(nFFT/2+1,1);  %-2 because we won't plot the points corresponding to the DC (first) or the nyquist (last)
rightP=zeros(nFFT/2+1,1);
leftDeltaP=zeros(nFFT/2+1,1); %hold the difference between spectra across time
rightDeltaP=zeros(nFFT/2+1,1);
freqs=zeros(nFFT/2-1+1,1);

frameNum=1; %initilize counter to capture video frames

doneLooping=0;
exceedsThreshold=0;

timeOfLastObject=uint64(0); %keep track of the timestamp of the previous object registered.  Compare new objects against this.  Don't register new objects unless they are P.minTimeDelta seconds old.
currentFrameTime = tic;  %grab the current time

while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
%     frame_filtered(1,:)=filtfilt(P.H.sosMatrix,P.H.scaleValues,frame(1,:));
%     frame_filtered(2,:)=filtfilt(P.H.sosMatrix,P.H.scaleValues,frame(2,:));

    [leftP,rightP,leftDeltaP,rightDeltaP,freqs, exceedsThreshold]=ComputePeriodogramTransients(frame,leftP,rightP,P.sampleRate,nFFT);
 
    t=tic;
    %interpret the threshold detection flag
    if(exceedsThreshold==1 && ((t-timeOfLastObject)>uint64(P.minTimeDelta_nanos))) %use _nanos on mac os and _micros on linux 
        %display('Exceeded threshold');
        
        new=GetNewEmptyObject;
        new.name='frqObjxx';
        new.onsetAzimuth=NaN;
        new.timeStamp=t;  
        new.isSelected=0;
        timeOfLastObject=new.timeStamp;
        didAddObject=AddNewObject(new);
      
        %display(didAddObject);
        %add a new object to the stack 
    else
    %nothing
   
    end
    
%     
    figure1=figure(1);
    subplot(2,1,1);
    bar(freqs(1:1000),leftDeltaP(1:1000)); %only plot frequencies up to some upper bound...this is hardcoded but could be initialized...but it depends on the frame size
    ylim([0 300]);
    title('Spectral Dynamics');

	ylabel('power');
    subplot(2,1,2);
    bar(freqs(1:1000),rightDeltaP(1:1000));
    ylim([0 300]);
    xlabel('frequency (hz)');
	ylabel('power');
% 
% %     annotation(figure1,'rectangle',...
% %     [0.386904761904762 0.582702702702703 0.0863095238095239 0.132972972972973],...
% %     'FaceColor','flat');
% %     annotation(figure1,'rectangle',...
% %     [0.387666666666668 0.10972972972973 0.0855476190476179 0.132972972972973],...
% %     'FaceColor','flat');
% 
%      drawnow;

%     videoFrames(frameNum)=getframe(figure1);
%     frameNum=frameNum+1;  
%     

   
end