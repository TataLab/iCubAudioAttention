function [frame, updatedFrameIndex,updatedTime,thisFrameSampleIndex] = GetNextFrame( currentFrameIndex,currentTime)
%wait until the duration of a frame has elapsed and then read the next
%unread frame
%
%then update the coordinates of that frame and return everything
%
%this will give a frame back to the main code, which will handle the audio
%and then loop to come back here and wait

%look into the global parameter struct...this is slow...consider replacing
%if you need to speed up
global P;
global audioD;
global sampleD;

timingProblem=1;
timeSinceLastFrame=toc(currentTime);
while(toc(currentTime)<(1/P.frameRate)) %check the time elapsed since the last frame was read
    %wait until it is time to read the next frame
    %display(['time until we grab the next frame = ' num2str(1/P.frameRate-toc(currentTime))]);
    timingProblem=0;  %a simple check to make sure you're having to wait for each frame
end

%display('Ok grab the next frame...');
if(timingProblem==1) %then we didn't have to wait above
    display(['Timing problem! Your audio may be lagging by about ' num2str(timeSinceLastFrame-P.frameDuration_seconds) ' seconds.  Will skip ahead.  This might be clicks or pops.']);
    %try to catch up, this might make a click or pop
    currentFrameIndex=sampleD.data(1,1).f-P.frameDuration_samples;
end

%grab the audio data
frame=double(audioD.Data(1,1).d(:,currentFrameIndex:currentFrameIndex+P.frameDuration_samples-1));
updatedTime=tic;

%a quick sanity check to make sure you're not reading off the end of the
%data that's been written
if(sum(frame(1,:))==0) %theres a problem.  Try to recover.
    display('You are reading zeros in the audio data.  Maybe the portaudio yarpdev is lagging.  Scanning for most recent sample to resync.');
    updatedFrameIndex=sampleD.Data(1,1).f+P.frameDuration_samples;
else
    % all is well so update the frame index for the next frame using time
    updatedFrameIndex=currentFrameIndex+ P.frameDuration_samples ; %figure out where the next frame will start
end


thisFrameSampleIndex=sampleD.Data(1,1).f;  %return the value

end

