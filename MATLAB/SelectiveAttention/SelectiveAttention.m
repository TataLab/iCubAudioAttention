%read audio stream out of shared memory and try to select a human voice
%from the mix

%perform basic stimulus driven orienting to a salient sound
addpath('/Users/Matthew/Documents/Robotics/iCubAudioAttention/MATLAB/SelectiveAttention/Functions_and_Scripts');
display(['Running SelectiveAttention using code at: ' mfilename('fullpath')]);

%perform initialization and setup;  get back a struct with all the settings
%we'll need
P=ConfigureParameters;


frameL=P.audioInL.Data(1,1).audioD(1:P.nBands,end-P.sizeFramePlusOverlap+1:end); %initialize the first frame 
frameR=P.audioInR.Data(1,1).audioD(1:P.nBands,end-P.sizeFramePlusOverlap+1:end);
nextFrameStamp = P.audioInL.Data(1,1).audioD(end-1,end);%initialize the sequence stamp of the most recent sample written (it's in the last column in the buffer, in the second from last row)

%matrices for holding previous frames of rms amplitudes
pastAmp=ones(P.nPastFrames,P.nBands).*.0001; %we have to seed this with some arbitrarily small numbers
pastDeltaAmp=zeros(P.nPastFrames,P.nBands);


frameCounter=1;

done=0;
while(~done)
    
    %grab audio for the next frame
    nextFrameStamp=nextFrameStamp+P.frameDuration_samples; %increment what is the sequence stamp we need to wait for
    
    while(P.audioInL.Data(1,1).audioD(end-1,end) < nextFrameStamp)
        %spin
        %if you don't have to spin here, then the code below is running too
        %slowly
    end

    frameL=P.audioInL.Data(1,1).audioD(1:P.nBands,end-P.sizeFramePlusOverlap+1:end); %retreive the last "frame size" chunk of data
    frameR=P.audioInL.Data(1,1).audioD(1:P.nBands,end-P.sizeFramePlusOverlap+1:end); %retreive the last "frame size" chunk of data


    %%%%%%%    Pre-Attentive Stage    **********
    
    
    
    %%%%%%Spectral Pre-processing*****
    
    %compute the amplitude of each band
    amp_frameL=rms(frameL,2);
    amp_frameR=rms(frameR,2);
    
    amp_frameL=amp_frameL';
    amp_frameR=amp_frameR';
    
    amp=(amp_frameL+amp_frameR)./2;  %collapse left and right channels - assume they have (nearly) identical spectra
    deltaAmp=(amp-mean(pastAmp,1))./mean(pastAmp,1);  %subtract the mean of the past spectral amplitude and divide by the mean of the past spectral amplitude
    pastAmp=circshift(pastAmp,[1 0]); %push the stack down and wrap
    pastAmp(1,:)=amp;  %overwrite the top of the stack
    pastDeltaAmp=circshift(pastDeltaAmp,[1 0]);
    pastDeltaAmp(1,:)=deltaAmp;
    
    deltaAmp(deltaAmp<0)=0; %only deal with increments
    [spectralPeakValues,spectralPeakIndices]=findpeaks(deltaAmp); %find the peak values and their indices in the spectrum 
    
    if(isempty(spectralPeakValues))
        [spectralPeakValues,spectralPeakIndices]=max(deltaAmp); %just use the single largest value
    end
    
    plot(deltaAmp);
    ylim([0 50]);
    drawnow;
    
    audioSalience= sum(spectralPeakValues) * length(spectralPeakValues); %this is the magical secret sauce that tells us how likely there is a new "voice-like" object in the scene
    
%     plot(tic,audioSalience,'o');
%     drawnow;
%     hold on;
    
    %%%%end spectral salience
    

    
    %%%%%Spectral Selection
    
    
    
    

    
    %
    %     plot(lastFrameStamp,audioSalience,'o');
    %     hold on;
    %     drawnow;
    
    %     [x,y] = pol2cart(selectedAngle,1); %convert angle and unit radius to cartesian
    %     compass(x,y);
    %     drawnow;
    %
    
    
frameCounter=frameCounter+1;
    
end