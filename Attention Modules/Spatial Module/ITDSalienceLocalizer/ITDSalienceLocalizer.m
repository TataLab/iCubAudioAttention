%ITD Salience Localizer

%fundamentally different from ITD Localizer!
%ITD Salience Localizer monitors the GCC-PHAT for spatial transients (i.e. new
%objects)

%it doesn't send data to YARP!  It creates a new object file when it sees a
%change in lag space

ITDSalienceConfigureAudioParameters;  %call the script that sets up the P parameter structure

doneLooping=0;
exceedsThreshold=0;

previousAngle=0.0;  %scope this outside of the loop
newAngle = 0.0;
tempNewAngle=0.0;
newAngle_deg=newAngle/pi  * (180);

freqs=1:40960;

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


%initialize a lag space vector
currentLagSpace=zeros(1,2*P.frameDuration_samples);

while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way

    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
    
    [newAngle,currentLagSpace,trigger]=ComputeAngleUsingITDSalience(frame,currentLagSpace,P.sampleRate);  %compute the angle using a GCC-PHAT approach
    
   
    if(trigger==1) % update the angle if there's a positive transient (positive could be movement or onset...should do something with offsets eventually...but hmmm, not the same as onsets according to our EEG data)
       
        new=GetNewEmptyObject;
        new.onsetAzimuth=newAngle;
        new.timeStamp=currentFrameTime;
        new.name='ITDObjxx';
        AddNewObject(new);
        
       
    end
   

end