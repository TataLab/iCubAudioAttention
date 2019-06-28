%ITD Object Localizer

%monitor the ITD over time for transients that suggest new auditory object
%sources

%add new sources to the object stack

%simultaneously use the object on the top of the stack to improve the
%azimuth estimate


ITDObjectLocalizerConfig;  %call the script that sets up the P parameter structure

doneLooping=0;

previousAngle=0.0;  %scope this outside of the loop
newAngle = 0.0;
newAngle_deg=newAngle/pi  * (180);

%%%%%%%%%Audio Input Setup&&&&&&&&&&&&&
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
%%%%%%%%%%%%%%%%%%%%%%%%

%memory map the object files
[objectMap,~,~]=MapObjectFile;

currentITD=zeros(1,P.frameDuration_samples*2);

while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
    
    %check every frame for an ITD transient
    [newAngle,currentITD,trigger]=DetectNewITDObjects(frame,currentITD,P.sampleRate);  %compute the angle using a GCC-PHAT approach
    if (trigger==1) %found a new source, add a new object to the stack
        display(['new object source at ' num2str(newAngle) ' degrees']);
        new=GetNewEmptyObject;
        new.timeStamp=tic;
        new.onsetAzimuth=newAngle;
        new.azimuth=newAngle;
        AddNewObject(new);
    end
%      %if no new object detected so just use regular GCC-PHAT to localize the current frame
%     if trigger==0 && objectMap.Data(1,1).isDefault~=1  %don't update the default object, just wait until a new one appears
%         newAngle=ComputeAngleToObject(frame,objectMap,P.sampleRate);
%         display(['updating with angle: ' num2str(newAngle)]);
%         UpdateOldObject(1,'azimuth',newAngle,'average');
%     end
    
    trigger=0; %reset for next round
    

    
    %make some pretty pictures
    %display(['current angle to audio source = ' num2str(newAngle_deg)]);
%     [x,y] = pol2cart(newAngle,1); %convert angle and unit radius to cartesian
%     figure(1);
%     hold off;
%     compass(x,y);
%     drawnow;
    
end