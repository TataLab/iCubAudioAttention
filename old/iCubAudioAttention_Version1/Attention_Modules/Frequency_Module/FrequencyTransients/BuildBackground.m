function [ output_args ] =  [freqBackground,freqStdDev] = BuildBackground(nFrames,startIndex,startTime)
%BUILDBACKGROUND takes frames of background and averages them


global P;
lagArray=zeros(nFrames,P.frameDuration_samples);

display('Collecting background audio');

%grab frames
for i=1:nFrames
    display(['computing GCC for frame' num2str(i)]);
    [temp,startIndex, startTime]=GetNextFrame(startIndex,startTime);
    


end

