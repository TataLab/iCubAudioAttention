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

timingProblem=1;

while(toc(currentTime)<(1/P.frameRate)) %check the time elapsed since the last frame was read
    timingProblem=0;  %a simple check to make sure you're having to wait for each frame
end
%grab when we read the data
updatedTime=tic;

if(timingProblem==1)
    display('Timing problem! Your audio may be lagging.')
end

%grab the audio data
blah=audioD.Data(1,1).d;  %get the next frame
frame=double(blah(:,currentFrameIndex:currentFrameIndex+P.frameDuration_samples-1));

%a quick sanity check to make sure you're not reading off the end of the
%data that's been written
if(strcmp(frame(1,1:10),zeros(1,10)));
    display('you are reading zeros in the audio data.  Something might be wrong!');
end

% plot(frame(1,:));
% ylim([-2e14 2e14]);
% drawnow;

%update the frame index for the next frame
updatedFrameIndex=currentFrameIndex+ P.frameDuration_samples ; %figure out where the next frame will start

end

