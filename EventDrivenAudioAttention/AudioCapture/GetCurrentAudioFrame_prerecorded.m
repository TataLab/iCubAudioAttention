function [frameL,frameR,yy] = GetCurrentAudioFrame_prerecorded(yy,frameSize_samples)
%reaches into your audio buffer or port devices and grabs the most recent frameSize_samples
%it can find


%for pre-recorded wav file stored on workspace
frameL=yy(1,1:frameSize_samples);
frameR=yy(2,1:frameSize_samples);
yy=circshift(yy,[0 -frameSize_samples]);%shift and wrap the samples for the next frame read




end