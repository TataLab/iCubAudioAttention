function [ leftP,rightP,leftD,rightD,freqs,thresholdFlag] = ComputePeriodogramTransients( frame,previousL,previousR,sampleRate,nfft )
%takes a stereo audio frame and computes the periodogram independently for
%the two channels

global P;

%compute the periodograms and save the complete spectrum temporarily
[tempLeftP,freqs]=periodogram(frame(1,:),[],'onesided',nfft,sampleRate);
[tempRightP]=periodogram(frame(2,:),[],'onesided',nfft,sampleRate);

leftP=tempLeftP;
rightP=tempRightP;
leftD=tempLeftP-previousL;
rightD=tempRightP-previousR;

leftD(leftD<0)=0; %only interested in onsets for now
rightD(rightD<0)=0;

%implement a very simple trigger:  look between 500 and 1000 hz for
%transients, only look for positive peaks

%apply some threshold detection and return a flag
%find the elements of the periodogram that correspond to the frequency
%bounds provided by P structure

[~,lowerBound]=min(abs(freqs-P.thresholdBounds(1)));
[~,upperBound]=min(abs(freqs-P.thresholdBounds(2)));


if( max(leftD(lowerBound:upperBound) > P.peakThreshold) || max(rightD(lowerBound:upperBound) > P.peakThreshold) )
       thresholdFlag=1;
else
        thresholdFlag=0;
end


return

end

