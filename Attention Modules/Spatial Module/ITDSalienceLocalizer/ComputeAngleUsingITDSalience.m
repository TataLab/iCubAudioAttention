function [angle,lagSpace,trigger] = ComputeAngleUsingITDSalience(S,lagVectorPrevious,rate)
% takes S a stereo sound vector (chans x samples) 
%returns an angle to a peak in GCC_PHAT, the vector of lags, and the
%difference of lags between current frame and previous frame


global P;

%compute GCC-PHAT

%define the width of the window over which to inspect lags (we're only
%interested in lags of a few 10's- the rest would be reverberation

%g=gausswin(length(S));

%xplode the array to make this more clear
s1=S(1,:);
s2=S(2,:);

f1=fft(s1);
f2=fft(s2);

%compute the weighted generalized correlation coefficient
gcc_phat=(f1.*conj(f2))./abs(f1.*conj(f2));

%compute the inverse transform
gcc_inv = ifft(gcc_phat);

%make it real
gcc_inv_real=real(gcc_inv);

%shift it to zero the 0th lag
gcc_inv_shifted=fftshift(gcc_inv_real);

%compute the difference between the current lag space and the
%previous

%frame's lag space
lag_dif=gcc_inv_shifted-lagVectorPrevious;



%find the 0th lag
middle=floor(length(lag_dif)/2);

%find the index with the biggest peak
if(isnan(P.attentionBeamBounds(1))) %no bounds were specified so use the entire window
    [peak,tempLag] = max(lag_dif(middle-P.ITDWindow/2:middle+P.ITDWindow/2)); %only the window  lags are relevant
    lag=tempLag-P.ITDWindow/2;
else
    [peak,~] = max(lag_dif(middle+P.attentionBeamBounds(1):middle+P.attentionBeamBounds(2)));
    lag=floor(mean(P.attentionBeamBounds)); %you've already specified an angle to watch for transients
    display(['peak value within the attention beam was ' num2str(peak)]);
end

%employ a simple trigger
if peak>P.peakThreshold
    trigger=1;
    if(~isnan(P.attentionBeamBounds(1)))%report that we know the speaker is in the beam
        display('I know you are there!');
    end
else
    trigger=0;
end

%convert the lag in samples to lag in seconds
angle=ConvertLagToAngle(lag);


%send this frame's gcc-phat vector back to be passed along to this function during the next frame
lagSpace=gcc_inv_shifted;

% % %%%%%check your work
plot(lag_dif(middle-P.ITDWindow/2:middle+P.ITDWindow/2));
ylim([-.2 .2]);
drawnow;

return

end

