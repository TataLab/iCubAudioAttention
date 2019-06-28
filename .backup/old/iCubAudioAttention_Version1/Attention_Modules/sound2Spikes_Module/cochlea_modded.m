function [tap, cochwave,fs,cf ] = cochlea_modded(tap, fs, numChannel, lowFreq, highFreq )
% function output = cochlea_modded(tap, fs, numChannel, lowFreq, highFreq)
% Frontend function to call ERB filters for processing an input waveform with a gammatone filter bank. 
% This function takes the raw data from audio device and returns back the
% filtered audio for each gamma band.

numEars=size(tap,2); %mono or stereo
cochwave=cell(1,numEars);

% coeficients of 4 2nd order gammatone filters in paralled numChannel
% channels distributed in ERB space from 200Hz to 1.5kHz
% lowFreq=100;
[fcoefs,cf]=MakeERBFilters(fs,numChannel,lowFreq,highFreq);

for i=1:numEars
    % signal pass ERB filter
    cochwave{1,i}=ERBFilterBank(tap(:,i),fcoefs);
end

end

