%Jeffress ITD Localizer finds transients in ITD and spectral domains and
%registers new objects onto an object file that is memory mapped for speed


%report what we're doing to keep this distinct from other versions of this
%code
display(['Running ITD Localizer using: ' mfilename('fullpath')]);


ConfigureAudioParameters;  %call the script that sets up the P parameter structure


%memory map the input-level file...this is the raw audio signal
%coming from the audio hardware...also initialize an index to keep track of
%which frame is the most recent available frame to read
global audioD;
global sampleD;
[audioD,sampleD]=OpenAudioInputData;

%preallocate an array to hold and plot frame data
visualizeArray=zeros(P.ITDWindow+1,10);
frameNum=1; %initilize counter to capture video frames

%initialize an array to store a collapsed audio frame
mono=zeros(1,P.frameDuration_samples);

%initialize a vector to hold the periodogram
tempPGram=zeros(1,length(P.frequencyWindow));
pGramArray=zeros(P.numPGrams, length(P.frequencyWindow)); %to accumulate periodograms of prior frames with which to scale the raw values

%initialize a lag space vector
lagSpaceArray=zeros(P.numLagSpaces,P.ITDWindow); %an array in which to store most recent lag space vectors  We'll use these to scale the raw values
previous=zeros(1,P.ITDWindow);%initialize a vector to hold the previous frame

timeOfLastObject=uint64(0); %keep track of the timestamp of the previous object registered.  Compare new objects against this.  Don't register new objects unless they are P.minTimeDelta seconds old.

%reset the timing
tempMostRecentSample=sampleD.Data(1,1).f;
currentFrameIndex = tempMostRecentSample - (P.frameDuration_samples - 1) - P.fixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
currentFrameTime = tic;  %grab the current time

%initialize YARP if you want to send angles to an iCub
if(P.sendAngleToYarp==1)
    InitializeYARP;
end

trigger=0;
doneLooping=0;
while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way

    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime,thisFrameSampleIndex]=GetNextFrame(currentFrameIndex,currentFrameTime);
    
    %%%%%%%The spatial ITD domain section%%%%%%%%
    %finds the peaks in a straight-ahead xcorr()
    [newXCorrAngle,newLag,previous,XCorrTrigger,visXcorrFrame]=ComputeAngleUsingITDSalienceXCor(frame,mean(lagSpaceArray,1),std(lagSpaceArray,0,1));  %compute the angle using a xcorr() approach
    
    lagSpaceArray=circshift(lagSpaceArray,[1 0]); %shift oldest to top
    lagSpaceArray(1,:)=previous; %overwrite the most recent on top
    %%%%%%%%%%%%%%%%%
  
    
    %%%%The frequency domain section%%%%%%%%%
    %merge stereo chans into mono in a smart way by using the lag reported
    %by the xcorrelation.
    tempStereo=[frame(1,:);circshift(frame(2,:),[0 newLag])]; %shift the signals so the new object aligns in phase
    mono=mean(tempStereo,1);  %average across the channels
    [tempPGram,diffPGram,~,frequencyTrigger]=ComputePeriodogramTransients(mono,mean(pGramArray,1),std(pGramArray,0,1),P.sampleRate);
    pGramArray=circshift(pGramArray,[1 0]); %shift oldest to top
    pGramArray(1,:)=tempPGram;
    
    %%%%%%%%%%%%%%%%
    
    
    %build and register new object IFF the xcor was superthreshold and the
    %previously registered object was long ago
    if((XCorrTrigger==1) && (frequencyTrigger==1) && (currentFrameTime-timeOfLastObject)>P.minTimeDelta_micros) % update the angle if there's a positive transient (positive could be movement or onset...should do something with offsets eventually...but hmmm, not the same as onsets according to our EEG data)
                                                                                                    %use _nanos on mac os and _micros on linux 
        timeOfLastObject=currentFrameTime;
        
        %send the angle of the current object to the iCub
        if(P.sendAngleToYarp==1)
            SendAngleToYarp(newXCorrAngle,port);
        end
        
        display(['new sound at ' num2str(newXCorrAngle)]);
        
        %since we just reoriented, all the lags are wrong.  We have to
        %re-initialize those arrays.  this isn't fully worked out yet
        %because really it should wait long enough to repopulate the entire
        %array rather than use zeros
        
        lagSpaceArray=zeros(P.numLagSpaces,P.ITDWindow); %an array in which to store most recent lag space vectors  We'll use these to scale the raw values

    end

    %else it just loops and waits for the next frame to come in
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
% % % %%%%% PLOT to check your work
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

subplot(2,1,1);
plot(P.freqs,diffPGram);
hold on;
ylim([0 P.peakThreshold*8]);
xlabel('frequency');
ylabel('delta power');

subplot(2,1,2);
plot(linspace(floor(-P.ITDWindow/2),ceil(P.ITDWindow/2),length(visXcorrFrame)),visXcorrFrame);
hold off;
xlim([-ceil(P.ITDWindow/2) ceil(P.ITDWindow/2)]);
ylim([0 P.xcorrPeakThreshold*20]);
xlabel('lag');
ylabel('delta xcorr');

drawnow;



end