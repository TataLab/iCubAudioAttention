%ITD Localizer

%periodically grab audio data out of AudioD and then:
%1) compute the angle to the
%most prominent source using GCC-PHAT
%
%2) compute simple beamforming to supress noise (a little bit)
%and write the resulting (one channel) signal to the SpatialAudio.data file

ITDConfigureAudioParameters;  %call the script that sets up the P parameter structure

if (P.sendAngleToYarp==1)   %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');
    %connect with Yarp network
    port = OpenYarpWritePort;
    display('press any key when ready');
    waitforbuttonpress;
end

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

display('waiting for above-threshold sound');

while (~doneLooping)  %loop continuously handling audio in a spatialy sort of way
    
    %update our local copy of the audio data frame and its coordinates
    [frame,currentFrameIndex, currentFrameTime]=GetNextFrame(currentFrameIndex,currentFrameTime);
    
    %a simple trigger
    %check if this frame exceeds amplitude threshold, if it doesn't don't
    %do anything with it
    exceedsThreshold=0;
    if(max(frame(1,:) > P.peakThreshold) || max(frame(2,:))>P.peakThreshold)
        exceedsThreshold=1;
        %display('That frame exceeded threshold');
    end
    
    
    
    %check and update the angle to the source if necesasary
    if exceedsThreshold==1 
        [newAngle,gcc_inv_shifted]=ComputeAngle(frame,P.sampleRate,1);  %compute the angle using a GCC-PHAT approach
        
        newAngle_deg=newAngle/pi  * (180);
        display(['Angle to source is aproximately: ' num2str(newAngle_deg) ' degrees']);
                
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