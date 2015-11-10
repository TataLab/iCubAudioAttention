%read audio stream out of shared memory and try to select a human voice
%from the mix

%perform basic stimulus driven orienting to a salient sound

display(['Running StimulusDrivenOrienting using code at: ' mfilename('fullpath')]);

%perform initialization and setup;  get back a struct with all the settings
%we'll need
P=ConfigureParameters;


frame=P.audioIn.Data(1,1).audioD(:,end-P.sizeFramePlusOverlap+1:end); %initialize the first frame 
lastFrameStamp = frame(3,end);%ask what is the stamp of the most recent sample written (it's in the last column in the buffer)

%prime the filterbank
P_inL=zeros(8,P.nBands);
P_inR=zeros(8,P.nBands);


%matrices for holding previous frames of rms amplitudes
pastAmp=ones(P.nPastFrames,P.nBands).*.0001; %we have to seed this with some arbitrarily small numbers
pastDeltaAmp=zeros(P.nPastFrames,P.nBands);

selectedBeam=1;
selectedAngle=1;

%get some variables ready for the output of the beamformer stage
thisFrameImage=zeros(P.nBands,P.nBeams,P.frameDuration_samples); %it's only nBeamsPerHemi *2 long because we loose half the samples off either end (theoretically they're the last samples of the previous frame and the first samples of the frame that hasn't happend yet)

maxBeamsIndices=zeros(P.nBands,1);
frameCounter=1;

done=0;
while(~done)
    

    %%%%%%%    Pre-Attentive Stage    **********
    
    %filter into bands
    [frameL,P_outL,~,~,~]=gammatonePhase(frame(1,:),P_inL,P.sampleRate, P.cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [frameR,P_outR,~,~,~]=gammatonePhase(frame(2,:),P_inR,P.sampleRate, P.cfs);
    P_inR=P_outR;
    
    
    %%%%%%Spectral Pre-processing*****
    
    %compute the amplitude of each band
    amp_frameL=rms(frameL,1);
    amp_frameR=rms(frameR,1);
    
    amp=(amp_frameL+amp_frameR)./2;  %collapse left and right channels - assume they have (nearly) identical spectra
    deltaAmp=(amp-mean(pastAmp(:,1)))./mean(pastAmp(:,1));  %subtract the mean of the past spectral amplitude and divide by the mean of the past spectral amplitude
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
    
    
    %%%%end spectral salience
    
%     %%%%%Spatial Pre-processing****
%     %twist around to make audio signals into row vectors for beamforming
%     frameL=frameL';
%     frameR=frameR';
% 
%     %beamformer
%     for i=1:P.nBands
%         thisBandL=frameL(i,:);
%         thisBandR=frameR(i,:);
% 
%         thisFrameImage(i,:,:)=thisBandL(P.lIndex)+thisBandR(P.rIndex); %compute all the beams in one step = MATLAB is fast
%     end
% 
%     %localize by find the angle with the most maxima
%     thisFrameRMS=rms(thisFrameImage,3); %find the peaksxbeams matrix of rms values
%     [~,thisFrameMaxima]=max(thisFrameRMS,[],2);
%     
%     
%     
%     %%%%%%  Selective Attention Stage *********
%     %select the modal beam
%     if(audioSalience>P.attentionCaptureThreshold)
%         selectedBeam=mode(thisFrameMaxima);
%         selectedAngle=P.angles(selectedBeam);
%         if (P.sendAngleToYarp==1)
%             %send the angle
%             audioAttentionControl('/mosaic/angle:i',selectedAngle*180/pi,1.0);
%             display(['sending ' num2str(selectedAngle*180/pi) ' to YARP']);
%         end
%     
%     end
%     
% 
%     
%     %
%     %     plot(lastFrameStamp,audioSalience,'o');
%     %     hold on;
%     %     drawnow;
%     
%     %     [x,y] = pol2cart(selectedAngle,1); %convert angle and unit radius to cartesian
%     %     compass(x,y);
%     %     drawnow;
%     %
%     
    
    %grab audio for the next frame
    %don't worry about going too fast because GetNextFrame waits (but do
    %worry about going to slow)
    nextFrameStamp=lastFrameStamp+P.frameDuration_samples; %increment

    [frame]=GetNextFrame(P,nextFrameStamp); %blocks until the last sample in the input buffer is greater or equal to the sample you want.  Requires that we handle data here faster than it's written in.

    if(~isequal(nextFrameStamp,frame(3,end)))
        display([' expected frame to end at sample ' num2str(nextFrameStamp) ' but I was ' num2str(frame(3,end) - nextFrameStamp) ' samples behind']);
    end
    
    lastFrameStamp=nextFrameStamp;
    frameCounter=frameCounter+1;
    
end