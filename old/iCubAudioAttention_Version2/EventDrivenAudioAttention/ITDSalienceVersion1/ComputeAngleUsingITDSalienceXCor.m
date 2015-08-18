function [angle,lag,lagSpace,trigger,visFrame] = ComputeAngleUsingITDSalienceXCor(S,means,stdevs)
% takes S a stereo sound vector (chans x samples) 
%returns an angle to a peak in GCC_PHAT, the vector of lags, and the
%difference of lags between current frame and previous frame

%display(['Jefress is looking for ITD Salience using: ' mfilename('fullpath')]);


global P;

%compute cross-correlation
cross=xcorr(S(1,:),S(2,:));

%this could be made *much faster* by only working on the subregion of the
%vectors that correspond to useful angles
middle=floor(length(cross)/2+2);
cross=cross(middle-P.ITDWindow/2:middle+P.ITDWindow/2-1);


%compute the difference between the current lag space and the
%previous in normalized values
lag_dif=(cross-means)./stdevs;

%display(max(lag_dif));

%frame's lag space
lag_dif(lag_dif<0)=0;  %ignore offsets for now


%find the index with the biggest peak

wVec=[P.weightsV flipdim(P.weightsV,2)]; %apply weighting to periphery to tune the AI if necessary
window=lag_dif;
window=window.*wVec;
[peak,tempLag] = max(window); %only the window  lags are relevant
lag=tempLag-P.ITDWindow/2;


%employ a simple trigger
if peak>P.xcorrPeakThreshold
    trigger=1;
    
else
    trigger=0;
end

%convert the lag in samples to lag in seconds
angle=ConvertLagToAngle(lag);

%display(['angle = ' num2str(angle)]);

%send this frame's gcc-phat vector back to be passed along to this function during the next frame
lagSpace=cross;

%return the data you want to visualize:
visFrame=lag_dif;
%visFrame=tempBlah;

return

end

