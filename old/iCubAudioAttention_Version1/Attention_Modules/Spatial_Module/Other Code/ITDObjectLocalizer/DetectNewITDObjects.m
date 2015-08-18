function [theta,newITD,trigger] = DetectNewITDObjects(S,previousITD,rate)
% takes S a stereo sound vector (chans x samples)
% computes difference between previous ITD frame and current ITD frame to
%find transients in the ITD, these suggest new objects

c=340.29;%define speed of sound in m/s
D=0.1; %define distance between microphones in m

%define the width of the window over which to inspect lags (we're only
%interested in lags of a few 10's- the rest would be reverberation

window=28; %should be divisible by 2...this is how many samples to look on each side of the midline for a peak...this should be related to the distance between the microphones

%xplode the array to make this more clear
s1=S(1,:);
s2=S(2,:);

%compute fft of inputs
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

%compute the difference between this ITD and that of the previous frame
deltaITD=previousITD-gcc_inv_shifted;  %ponder this:  because this is GCC-PHAT, new sources "steal" power from old sources.  So positive values of deltaITD indicate old sources and negative values indicate new sources
newITD=gcc_inv_shifted; %return to be passed along with subsequent frame

%simple threshold of the ITD difference. We can be much more sophisticated


%negative values
if(min(deltaITD(end/2-window:end/2+window))<-0.2)
    trigger=1;
    plot(deltaITD(end/2-100:end/2+100));
%ylim([0 .1]);
drawnow;

    
else
    trigger=0;
end


%find the 0th lag
middle=floor(length(deltaITD)/2);

%find the index with the biggest peak
[~,tempLag] = min(deltaITD(middle-window/2:middle+window/2)); %only the first hundred or so lags are relevant
lag=tempLag-window/2;

%convert the lag in samples to lag in seconds
lag_seconds=lag/rate;

%return the angle of the peak in the difference ITD...note that's not quite
%the same as just using ITD to find an angle
theta_rad=real(asin(c*lag_seconds*(1/D)));
theta=theta_rad/pi  * (180);

return


end

