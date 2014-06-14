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
% currentFrameIndex = tempMostRecentSample - (P.frameDuration_samples - 1) - P.fixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
% currentFrameTime = tic;  %grab the current time

%preallocate an array to hold and plot frame data
visualizeArray=zeros(P.ITDWindow+1,10);



%initialize a lag space vector
currentLagSpace=zeros(1,P.frameDuration_samples-0*2);

timeOfLastObject=uint64(0); %keep track of the timestamp of the previous object registered.  Compare new objects against this.  Don't register new objects unless they are P.minTimeDelta seconds old.

%build a model of the background sound sources
%[backgroundLag,backgroundLagSD]=BuildBackground(30,currentFrameIndex,currentFrameTime);
backgroundLag=zeros(1,P.frameDuration_samples);

%reset the timing
tempMostRecentSample=sampleD.Data(1,1).f;
currentFrameIndex = tempMostRecentSample - (P.frameDuration_samples - 1) - P.fixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
currentFrameTime = tic;  %grab the current time



while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way

    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);

    [newAngle,backgroundLag,trigger,visFrame]=ComputeAngleUsingITDSalience(frame,backgroundLag);  %compute the angle using a GCC-PHAT approach
    

    t=tic;
    if((trigger==1) && (t-timeOfLastObject)>P.minTimeDelta_nanos) % update the angle if there's a positive transient (positive could be movement or onset...should do something with offsets eventually...but hmmm, not the same as onsets according to our EEG data)
       
        timeOfLastObject=t;
        new=GetNewEmptyObject;
        new.onsetAzimuth=newAngle;
        display(['New object has onset azimuth of ' num2str(newAngle) ' degrees']);
        new.timeStamp=t;
        new.name='ITDObjxx';
        new.isSelected=0;  %setting this to 1 forces this object onto the
        %top of the stack
        [result]=AddNewObject(new);
        
        
    else
    %nothing
       
    %plot
    
    end
    
   

end