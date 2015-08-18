function [ leftP,rightP,freqs] = ComputePeriodogram( frame,sampleRate,nfft )
%takes a stereo audio frame and computes the periodogram independently for
%the two channels

[ltemp,freqs]=periodogram(frame(1,:),[],'onesided',nfft,sampleRate);
[rtemp]=periodogram(frame(2,:),[],'onesided',nfft,sampleRate);

%arrange them sensibly
leftP=ltemp';
rightP=rtemp';
end

