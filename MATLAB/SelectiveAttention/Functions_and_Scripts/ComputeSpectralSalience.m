function [ s ] = ComputeSpectralSalience( inFrame )
%COMPUTESPECTRALSALIENCE take a bands x samples frame of data and compute 
%spectral salience


amp=rms(inFrame,2);
peaks=findpeaks(amp);

if(isempty(peaks))
        peaks=max(amp); %just use the single largest value
end
s = sum(peaks) * length(peaks); %this is the magical secret sauce that tells us how likely there is a new "voice-like" object in the scene




end

