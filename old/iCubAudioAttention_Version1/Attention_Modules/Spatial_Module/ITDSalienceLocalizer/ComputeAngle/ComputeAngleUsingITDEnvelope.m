function[angle,tempLag,lagSpace,trigger,visFrame] = ComputeAngleUsingITDEnvelope(S,background)
%COMPUTEANGLEUSINGITDSALIENCEENVELOPE computes an angle to a source by
%cross-correlating the envelopes of the channels


%import some parameters from the workspace
global P;

%comput the envelope of the inputs using hilbert transform
S_env=abs(hilbert(S'))'; %transpose, hilbert works columnwise

% plot(S_env(1,:));
% drawnow;

%compute cross-correlation
cross=xcorr(S_env(1,:),S_env(2,:));



%this could be made *much faster* by only working on the subregion of the
%vectors that correspond to useful angles...in the future

%compute the difference between the current lag space and the
%previous scaled by  an estimate of the power at that lag
lag_dif=cross-background;

%display(max(lag_dif));

%frame's lag space
lag_dif(lag_dif<0)=0;  %ignore offsets for now
tempBlah=lag_dif;


%find the 0th lag
middle=floor(length(lag_dif)/2+2);

%find the index with the biggest peak
if(isnan(P.attentionBeamBounds(1))) %no bounds were specified so use the entire window
    
    
    %display(['computing angle for lag ' num2str(lag)]);
    
    %%%%%%%%%
    %these lines will just find the biggest peak
    wVec=[P.weightsV flipdim(P.weightsV,2)];
    window=lag_dif(middle-P.ITDWindow/2:middle+P.ITDWindow/2-1);
    window=window.*wVec;
    [peak,tempLag] = max(window); %only the window  lags are relevant
    lag=tempLag-P.ITDWindow/2;
    
else %bounds were specified, so only look within a range of lags
    [peak,~] = max(lag_dif(middle+P.attentionBeamBounds(1):middle+P.attentionBeamBounds(2)));
    lag=floor(mean(P.attentionBeamBounds)); %you've already specified an angle to watch for transients
    display(['peak value within the attention beam was ' num2str(peak)]);
end

%employ a simple trigger
if peak>P.xcorrPeakThreshold
    trigger=1;
    if(~isnan(P.attentionBeamBounds(1)))%report that we know the speaker is in the beam
        display('I know you are there!');
    end
else
    trigger=0;
end

%convert the lag in samples to lag in seconds
angle=ConvertLagToAngle(lag);

%display(['angle = ' num2str(angle)]);

%send this frame's gcc-phat vector back to be passed along to this function during the next frame
lagSpace=cross;

%return the data you want to visualize:
visFrame=tempBlah(middle-P.ITDWindow/2:middle+P.ITDWindow/2);
%visFrame=tempBlah;

return

end

