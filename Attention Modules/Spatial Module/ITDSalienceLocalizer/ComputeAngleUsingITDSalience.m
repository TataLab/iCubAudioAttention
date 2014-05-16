function [angle,lagSpace,trigger] = ComputeAngleUsingITDSalience(S,lagVectorPrevious,rate)
% takes S a stereo sound vector (chans x samples) 
%returns an angle to a peak in GCC_PHAT, the vector of lags, and the
%difference of lags between current frame and previous frame


global P;

%compute GCC-PHAT

%define the width of the window over which to inspect lags (we're only
%interested in lags of a few 10's- the rest would be reverberation

window=40; %should be divisible by 2...this is how many samples to look on each side of the midline for a peak...this should be related to the distance between the microphones
                    %for a 12 cm distance it is only linear for about 15
                    %samples on eithe side of the midline and 17 samples
                    %pins it at 90 degress.

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
[peak,tempLag] = max(lag_dif(middle-window/2:middle+window/2)); %only the window  lags are relevant
lag=tempLag-window/2;

%employ a simple trigger
if peak>P.peakThreshold  %looking for new objects as negative values because GCC_PHAT works that way
    trigger=1;
    
else
    trigger=0;
end

%convert the lag in samples to lag in seconds
angle=ConvertLagToAngle(lag);

%send this frame's gcc-phat vector back to be passed along to this function during the next frame
lagSpace=gcc_inv_shifted;

% %%%%%check your work
plot(lag_dif(middle-window/2:middle+window/2));
ylim([-.25 .25]);
drawnow;

return

end

