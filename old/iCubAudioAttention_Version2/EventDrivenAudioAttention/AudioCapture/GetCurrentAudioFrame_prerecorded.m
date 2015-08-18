function [frameL,frameR,yy] = GetCurrentAudioFrame_prerecorded(yy,frameSize_samples)
%reaches into your audio buffer or port devices and grabs the most recent frameSize_samples
%it can find


%for pre-recorded wav file stored on workspace
frameL=yy(1:frameSize_samples,1);
frameR=yy(1:frameSize_samples,2);
yy=circshift(yy,[-frameSize_samples 0]);%shift and wrap the samples for the next frame read




end