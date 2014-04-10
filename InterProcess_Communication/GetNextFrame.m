function [frame, updatedFrameIndex,updatedTime] = GetNextFrame( currentFrameIndex,currentTime )
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


while(toc(currentTime)<(1/P.frameRate)) %check the time elapsed since the last frame was read
    %block the thread to wait
end

%grab the audio data
blah=audioD.Data(1,1).d;  %get the next frame
frame=double(blah(:,currentFrameIndex:currentFrameIndex+P.frameDuration_samples*P.numChannels-1));

%grab when we read the data
updatedTime=tic;

%update the frame index for the next frame
updatedFrameIndex=currentFrameIndex+ P.frameDuration_samples ; %figure out where the next frame will start

end

