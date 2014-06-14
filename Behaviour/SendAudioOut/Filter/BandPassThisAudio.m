function [ frame_filtered ] = BandPassThisAudio( frame )
%BANDPASSTHISAUDIOruns a bandpass filter over a frame of audio data and
%returns it

global P;  %get access to the parameters

%      for bandpass equiripple filter
frame_filtered=zeros(size(frame));
for i=1:size(frame,1)
    frame_filtered(i,:)=filtfilt(P.Hband.Numerator,1,frame(i,:));
end


return;

end

