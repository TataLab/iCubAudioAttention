    
%learn a target talker and pre-select frequency bands

display(['Running SelectiveAttentionTunedBeamformer using code at: ' mfilename('fullpath')]);
addpath('/Users/Matthew/Documents/Robotics/iCubAudioAttention/MATLAB/SelectiveAttention/Functions_and_Scripts');

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

maxBeamsIndices=zeros(P.nBands,1);
frameCounter=1;

%set up an auditory object to keep track of sound sources
O.onsetTime=tic;
O.angle=0.0;
O.salience=0.0;

%tune the filterbank by listening to a target talker
%[fWeights]=TuneFilterBank(P);

%for testing
% fWeightsL=ones(4096,64);
% fWeightsR=ones(4096,64);

%for testing
numTalkers=2;
load('talker1Weights.mat');
load('talker2Weights.mat');

talker1Weights=talker1Weights';
talker2Weights=talker2Weights';

talker1Weights=repmat(talker1Weights,[P.frameDuration_samples 1]);
talker2Weights=repmat(talker2Weights,[P.frameDuration_samples 1]);

frameWeights(1,:,:)=talker1Weights;
frameWeights(2,:,:)=talker2Weights;

done=0;
while(~done)
    
    t=tic;
    
    
    %this reads output of the filterbank streamed by AudioCaptureFilterBank_YARP or
    %AudioCaptureFilterBank_PreRecorded
    frame=P.audioIn.Data(1,1).audioD(:,end-(P.frameDuration_samples+2*P.frameOverlap)+1:end); %note the overlap.  This is so that we can run beamformer and extract exactly frameDuration_samples from each frame by leaving off the tails.  Unnecessary unless you want to concatenate frames for output 


    frameL=frame(1,:);
    frameR=frame(2,:);
    
    %%%%%%%    Pre-Attentive Stage    **********
    
  
    
    
    %%%%%%Spectral Pre-processing*****
  
    %decompose each channle using a gammatone filterbank
    %and stream out the filtered frames into two seperate files
    [fFrameL,P_outL,~,~,~]=gammatonePhase(frame(1,:),P_inL,P.sampleRate, P.cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [fFrameR,P_outR,~,~,~]=gammatonePhase(frame(2,:),P_inR,P.sampleRate, P.cfs);
    P_inR=P_outR;
    
    %hold the salience for every talker
    audioSalience=zeros(1,numTalkers);
    
    for i=1:numTalkers
        
        %weight for this talker
        W=squeeze(frameWeights(i,:,:));
        fFrameL=W.*fFrameL;
        fFrameR=W.*fFrameR;
        
        
        %compute the amplitude of each band
        amp_frameL=rms(fFrameL,1);
        amp_frameR=rms(fFrameR,1);
        
        amp=(amp_frameL+amp_frameR)./2;  %collapse left and right channels - assume they have (nearly) identical spectra
        
        
        deltaAmp=(amp-mean(pastAmp,1))./mean(pastAmp,1);  %subtract the mean of the past spectral amplitude and divide by the mean of the past spectral amplitude
        
        pastAmp=circshift(pastAmp,[1 0]); %push the stack down and wrap
        pastAmp(1,:)=amp;  %overwrite the top of the stack
        
        %look at onsets and offsets independently
        deltaAmpOnsets=deltaAmp;
        deltaAmpOffsets=deltaAmp;
        
        deltaAmpOnsets(deltaAmpOnsets<0)=0; %only deal with increments
        deltaAmpOffsets(deltaAmpOffsets>0)=0;
        deltaAmpOffsets=deltaAmpOffsets.*-1; %make offsets positive
        
        
        %     subplot(2,1,1);
        %     plot(P.cfs,deltaAmpOnsets);
        %     ylim([0 1]);
        %     subplot(2,1,2);
        %     plot(P.cfs,deltaAmpOffsets);
        %     ylim([0 1]);
        %     drawnow;
        
        
        [spectralPeakValues,spectralPeakIndices]=findpeaks(deltaAmp); %find the peak values and their indices in the spectrum
        
        if(isempty(spectralPeakValues))
            [spectralPeakValues,spectralPeakIndices]=max(deltaAmp); %just use the single largest value
        end
        
        
        audioSalience(i)= sum(spectralPeakValues) * length(spectralPeakValues); %this is the magical secret sauce that tells us how likely there is a new "voice-like" object in the scene
        
        
    end
    
    subplot(2,1,1);
    plot(frameCounter,audioSalience(1),'o');
    hold on;
    subplot(2,1,2);
    plot(frameCounter,audioSalience(2),'o');
    hold on;
    drawnow;

%     %%%end spectral salience
%     
% %     %%%%%Spatial Pre-processing****
% %     %twist around to make audio signals into row vectors for beamforming
%     fFrameL=fFrameL';
%     fFrameR=fFrameR';
% 
%     
%     %get some variables ready for the output of the beamformer stage
%     thisFrameImage=zeros(P.nBands,P.nBeams,P.frameDuration_samples+2*P.frameOverlap); %it's only nBeamsPerHemi *2 long because we loose half the samples off either end (theoretically they're the last samples of the previous frame and the first samples of the frame that hasn't happend yet)
% 
%     
% %   %this is exactly a bank of delay-and-sum beamformers
%     for bandCounter=1:P.nBands
%         
%         thisBandL=fFrameL(bandCounter,:);
%         thisBandR=fFrameR(bandCounter,:);
%         
%         beamCounter=1;
%         for b=-P.nBeamsPerHemifield:P.nBeamsPerHemifield
%             tempR=circshift(thisBandR,[0 b]); %shift through each lag
%             thisFrameImage(bandCounter,beamCounter,:)=thisBandL+tempR; %add the shifted right channnel to the unshifted left channel
%             beamCounter=beamCounter+1;
%             
%         end
%     
%     end
%     
%      %localize by find the beam with the most energy 
%      %note that the filterbank weights multiply through this computation
%      
%     thisFrameRMS=rms(thisFrameImage,3); %find the peaksxbeams matrix of rms values
%     weightedRMS=sum(thisFrameRMS,1);
%     [~,maxBeam]=max(weightedRMS,[],2);
%     
% %     bar(P.angles,weightedRMS);
% %     drawnow;
%     
%     
% %     imagesc(thisFrameRMS);
% %     drawnow
%     
% %     P.attentionCaptureThreshold=0; %for testing
%     %%%%%%  Selective Attention Stage *********
%    
% %     
%     
%     %compute the time-decaying salience
%     %tdSalience =  P.salienceGain * 1./(1+exp(0.20*toc(O.onsetTime))) * O.salience ;
%     tdSalience = 2 * toc(O.onsetTime) * exp(-toc(O.onsetTime) * 0.8) + exp(-toc(O.onsetTime) * 0.8) * O.salience;
%     %     
%     plot(frameCounter,tdSalience,'o');
%     drawnow;
%     hold on;
% 
% 
% 
%     %apply object logic to select objects
%     %if the salience of the current frame exceeds the time-decaying
%     %salience of the selected object, then capture attention to the new
%     %object
%     if(audioSalience>tdSalience && audioSalience > P.attentionCaptureThreshold)
%         %a new object captured attention so update all the object features
%         O.salience=audioSalience;  %the current objects salience
%         O.onsetTime=tic;%take the last time stamp of the frame to be the onset time ... note that's arbitrarily inaccurate to within frameDuration
%         
%         %select the max beam
%         selectedBeam=maxBeam;
%         O.angle=P.angles(selectedBeam);
%         
%         if(P.sendAngleToYarp==1)
%             audioAttentionControl('/mosaic/angle:i',O.angle * 180/pi,tdSalience);
%         end
%         
%         display(['found salient talker at ' num2str(O.angle*180/pi) ' degrees']);
% 
%     end
%     
%     
%     
%     
%     
%     
%     
% 
% %         [x,y] = pol2cart(O.angle,1); %convert angle and unit radius to cartesian
% %         compass(x,y);
% %         drawnow;
%     
%     
% %     display(O.angle * 180 / pi);

    %increment for next frame
    nextFrameStamp=lastFrameStamp+P.frameDuration_samples; %increment
    lastFrameStamp=nextFrameStamp;
    frameCounter=frameCounter+1;
    
    while(P.audioIn.Data(1,1).audioD(end-1,end)<nextFrameStamp)
        %spin until the next frame has been written into the buffer
    end
    
    

    
end