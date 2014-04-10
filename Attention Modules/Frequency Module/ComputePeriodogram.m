function [ leftP,rightP,freqs] = ComputePeriodogram( frame,sampleRate,nfft )
%takes a stereo audio frame and computes the periodogram independently for
%the two channels

[leftP,freqs]=periodogram(frame(1,:),[],'onesided',nfft,sampleRate);
[rightP]=periodogram(frame(2,:),[],'onesided',nfft,sampleRate);

end

