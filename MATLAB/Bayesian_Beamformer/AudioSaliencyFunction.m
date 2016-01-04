%   function [audioSalience, audioSalienceFactor]=AudioSaliencyFunction(frameL,frameR)  

   %compute the amplitude of each band
    amp_frameL=rms(GF_frameL,1);
    amp_frameR=rms(GF_frameR,1);
    
    amp=(amp_frameL+amp_frameR)./2;  %collapse left and right channels - assume they have (nearly) identical spectra
 
% amp=rms(frame_GM,1);
    
    
    
    deltaAmp=(amp-mean(pastAmp,1))./mean(pastAmp,1);  %subtract the mean of the past spectral amplitude and divide by the mean of the past spectral amplitude
    pastAmp=circshift(pastAmp,[1 0]); %push the stack down and wrap
    pastAmp(1,:)=amp;  %overwrite the top of the stack
    pastDeltaAmp=circshift(pastDeltaAmp,[1 0]);
    pastDeltaAmp(1,:)=deltaAmp;
    
    deltaAmp(deltaAmp<0)=0; %only deal with increments
    [spectralPeakValues,spectralPeakIndices]=findpeaks(deltaAmp); %find the peak values and their indices in the spectrum 
    
%     cfs(spectralPeakIndices)
%     deltaAmp(spectralPeakIndices)
    if(isempty(spectralPeakValues))
      [spectralPeakValues,spectralPeakIndices]=max(deltaAmp); %just use the single largest value
    end
    
%     surf(pastDeltaAmp);
%     zlim([0 10]);
%     drawnow;
    
    audioSalience= sum(spectralPeakValues) * length(spectralPeakValues); %this is the magical secret sauce that tells us how likely there is a new "voice-like" object in the scene
    
%     if audioSalience>50
%         audioSalience=50;
%     end
    audioSalienceFactor=audioSalience/100;
    %%%%end spectral salience
    
%   end % function