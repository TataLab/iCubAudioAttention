function [ tempPGram,freqs,thresholdFlag] = ComputePeriodogramTransients( monoFrame,meanPGram,stdevPGram,sampleRate )
%takes a mono audio frame and computes the periodogram, returns a trigger flag set if it
%finds a salient peak and also returns this frame's current periodogram to
%pass to next call to this function (to compute difference over time)

global P;

%compute the periodograms and save the complete spectrum temporarily
[tempPGram,freqs]=periodogram(monoFrame(1,:),[],P.nfft,sampleRate,'one-sided');

tempPGram=tempPGram(P.frequencyWindow); %restrict the periodogram to the window of interest

tempPGram=tempPGram'; %reorient for sanity
diffPGram=(tempPGram-meanPGram)./stdevPGram;


diffPGram(diffPGram<0)=0; %only interested in onsets for now

%apply some threshold detection and return a flag


if(max(diffPGram) >= P.peakThreshold)  
       thresholdFlag=1;
else
        thresholdFlag=0;
end


return

end

