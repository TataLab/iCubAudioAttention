function [ outL,outR ] = GetCurrentAudioFrame_builtin( frameDuration, A )
%GETCURRENTAUDIOFRAME_BUILTIN 
%grab the most recent audio frame using matlab's builtin audio recorder


recordblocking(A,frameDuration);
data=getaudiodata(A);
outL=data(:,1);
outR=data(:,2);



end

