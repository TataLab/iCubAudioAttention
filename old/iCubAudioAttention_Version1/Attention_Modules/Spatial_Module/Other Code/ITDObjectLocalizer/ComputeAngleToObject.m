function [theta] = ComputeAngleToObject(S,objectMap,rate)
% takes S a stereo sound vector (chans x samples) 
%also take a pair of vectors containing frequencies to boost in the fft

c=340.29;%define speed of sound in m/s
D=0.1; %define distance between microphones in m
spotlightRadius=10.0; %degrees

%compute GCC-PHAT

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

%find the 0th lag
middle=floor(length(gcc_inv_shifted)/2);

%%%%%%%
%this is the "smart" part:  we know the estimated azimuth of the object on
%the top of the stack, so we adjust the window to look within a "spotlight"
%around this lag.

targetAngle=objectMap.Data(1,1).onsetAzimuth; %in degs
posBound=targetAngle+spotlightRadius;
negBound=targetAngle-spotlightRadius;

%check that the bounds don't go past +/- 90
if(posBound>90.0)
    posBound=90.0;
end
if(negBound<-90.0)
    negBound=-90.0;
end

%convert the bounds to lags
posBound_lag=ceil(middle+rate*D*sin(posBound/360*2*pi)/c);
negBound_lag=floor(middle-rate*D*sin(negBound/360*2*pi)/c);

hold off;
[x,y] = pol2cart(posBound/360.0*2*pi,1); %convert angle and unit radius to cartesian
compass(x,y);
hold on;
[x0,y0] = pol2cart(negBound/360.0*2*pi,1); %convert angle and unit radius to cartesian
compass(x0,y0);
drawnow;

%find the index with the biggest peak
[~,tempLag] = max(gcc_inv_shifted(negBound_lag:posBound_lag)); %only the first hundred or so lags are relevant
tempLag=tempLag+negBound; %the lag isn't accurate in the restricted space within the spotlight
lag=tempLag-window/2;

%convert the lag in samples to lag in seconds
lag_seconds=lag/rate;

theta_rad=real(asin(c*lag_seconds*(1/D)));
theta=theta_rad/pi  * (180);

return


end

