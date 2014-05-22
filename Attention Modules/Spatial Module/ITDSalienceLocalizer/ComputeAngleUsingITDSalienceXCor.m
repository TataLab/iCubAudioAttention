function [angle,lagSpace,trigger] = ComputeAngleUsingITDSalienceXCor(S,lagVectorPrevious,rate)
% takes S a stereo sound vector (chans x samples) 
%returns an angle to a peak in GCC_PHAT, the vector of lags, and the
%difference of lags between current frame and previous frame


global P;

%compute cross-correlation

%define the width of the window over which to inspect lags (we're only
%interested in lags of a few 10's- the rest would be reverberation

g=gausswin(length(S));

%xplode the array to make this more clear
s1=S(1,:).*g';
s2=S(2,:).*g';

%compute the difference between the current lag space and the
%previous

lagVec=xcorr(s1,s2,P.ITDWindow,'coeff');
lagSpace=lagVec; %send it back

%frame's lag space
lagDif=lagVec-lagVectorPrevious;


%find the index with the biggest peak
[peak,lag] = max(lagDif); %only the window  lags are relevant


%employ a simple trigger
if peak>P.peakThreshold
    trigger=1;
else
    trigger=0;
end

%convert the lag in samples to lag in seconds
angle=ConvertLagToAngle(lag);


% % %%%%%check your work
plot(lagDif);
ylim([-1 1]);
drawnow;

return

end

