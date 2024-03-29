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

%call periodogram on dummy data to pre-get nFFT and the frequency vector
[tempP,freqs]=periodogram(zeros(1,P.frameDuration_samples),[],[],P.sampleRate,'one-sided');  %get the size of the periodogram that will be returned for a frame of this length
nPeriodogram=length(tempP);
leftP=zeros(1,nPeriodogram);  %holds the periodograms for each frame
rightP=zeros(1,nPeriodogram);
leftDeltaP=zeros(1,nPeriodogram); %hold the difference between periodograms across time
rightDeltaP=zeros(1,nPeriodogram);


%frameNum=1; %initilize counter to capture video frames

doneLooping=0;
exceedsThreshold=0;

timeOfLastObject=uint64(0); %keep track of the timestamp of the previous object registered.  Compare new objects against this.  Don't register new objects unless they are P.minTimeDelta seconds old.
currentFrameTime = tic;  %grab the current time



while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    ftic=tic;
    [frame,currentFrameIndex, currentFrameTime, thisFrameSampleIndex]=GetNextFrame(currentFrameIndex,currentFrameTime);
   % display(['time to grab a frame = ' num2str(toc(ftic))]);
    %     frame_filtered(1,:)=filtfilt(P.H.sosMatrix,P.H.scaleValues,frame(1,:));
%     frame_filtered(2,:)=filtfilt(P.H.sosMatrix,P.H.scaleValues,frame(2,:));

    %display(['most recent sample written is: ' num2str(sampleD.data(1,1).f)]);
    [leftP,rightP,leftDeltaP,rightDeltaP,~, exceedsThreshold]=ComputePeriodogramTransients(frame,leftP,rightP,P.sampleRate);
 
    t=currentFrameTime;

    %interpret the threshold detection flag
    %exceedsThreshold=0; %in case you want to block object file
    if(exceedsThreshold==1 && ((t-timeOfLastObject)>uint64(P.minTimeDelta_nanos))) %use _nanos on mac os and _micros on linux 
        %display('Exceeded threshold');
        
        new=GetNewEmptyObject;
        new.name='frqObjxx';
        new.onsetAzimuth=0.0;
        new.timeStamp=t; 
        new.timeStampSamples=uint64(thisFrameSampleIndex);
        new.isSelected=0;
        new.isOriented=0;
        timeOfLastObject=new.timeStamp;
        didAddObject=AddNewObjectWithCapture(new,1);
      
        %display(didAddObject);
        %add a new object to the stack 
    else
    %nothing
   
    end
    
    %prepare to draw
    figure1=figure(1);
    subplot(2,1,1);
    bar(freqs(1:1000),leftDeltaP(1:1000)); %only plot frequencies up to some upper bound...this is hardcoded but could be initialized...but it depends on the frame size
    ylim([-80 80]);
    title('Spectral Dynamics');

	ylabel('power');
    subplot(2,1,2);
    bar(freqs(1:1000),rightDeltaP(1:1000));
    ylim([-80 80]);
    xlabel('frequency (hz)');
	ylabel('power');



     drawnow;
%      
%     To capture video of the periodogram:
%     videoFrames(frameNum)=getframe(figure1);
%     frameNum=frameNum+1;  
%     

   
end