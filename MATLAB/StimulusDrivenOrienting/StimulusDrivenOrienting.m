  
%perform basic stimulus driven orienting to a salient sound

display(['Running StimulusDrivenOrienting using code at: ' mfilename('fullpath')]);
addpath('/Users/Matthew/Documents/Robotics/iCubAudioAttention/MATLAB/StimulusDrivenOrienting/Functions_and_Scripts');

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
thisFrameImage=zeros(P.nBands,P.nBeams,P.frameDuration_samples+2*P.frameOverlap); %it's only nBeamsPerHemi *2 long because we loose half the samples off either end (theoretically they're the last samples of the previous frame and the first samples of the frame that hasn't happend yet)

maxBeamsIndices=zeros(P.nBands,1);
frameCounter=1;


done=0;
while(~done)
    
    t=tic;
    
    
    %this reads output of the filterbank streamed by AudioCaptureFilterBank_YARP or
    %AudioCaptureFilterBank_PreRecorded
    frame=P.audioIn.Data(1,1).audioD(:,end-(P.frameDuration_samples+2*P.frameOverlap)+1:end); %note the overlap.  This is so that we can run beamformer and extract exactly frameDuration_samples from each frame by leaving off the tails.  Unnecessary unless you want to concatenate frames for output 


    frameL=frame(1,:); %the transpose of confusion
    frameR=frame(2,:);
    
    %%%%%%%    Pre-Attentive Stage    **********
    
  
    
    
    %%%%%%Spectral Pre-processing*****
  
    %decompose each channle using a gammatone filterbank
    %and stream out the filtered frames into two seperate files
    [fFrameL,P_outL,~,~,~]=gammatone_ciM2(frame(1,:),P_inL,P.sampleRate, P.cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [fFrameR,P_outR,~,~,~]=gammatone_ciM2(frame(2,:),P_inR,P.sampleRate, P.cfs);
    P_inR=P_outR;
    
    
    %compute the amplitude of each band
    amp_frameL=rms(fFrameL,1);
    amp_frameR=rms(fFrameR,1);
    
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
    
    
    audioSalience= sum(spectralPeakValues) * length(spectralPeakValues); %this is the magical secret sauce that tells us how likely there is a new "voice-like" object in the scene
    
% 
%     
%     plot(frameCounter,audioSalience,'o');
%     hold on;
%     drawnow;
%     
    
    %%%%end spectral salience
    
%     %%%%%Spatial Pre-processing****
%     %twist around to make audio signals into row vectors for beamforming
    fFrameL=fFrameL';
    fFrameR=fFrameR';

%   %this is exactly a bank of delay-and-sum beamformers
    for bandCounter=1:P.nBands
        
        thisBandL=fFrameL(bandCounter,:);
        thisBandR=fFrameR(bandCounter,:);
        
        beamCounter=1;
        for b=-P.nBeamsPerHemifield:P.nBeamsPerHemifield
            tempR=circshift(thisBandR,[0 b]); %shift through each lag
            thisFrameImage(bandCounter,beamCounter,:)=thisBandL+tempR; %add the shifted right channnel to the unshifted left channel
            beamCounter=beamCounter+1;
            
        end
    
    end
    
     %localize by find the angle with the most maxima
    thisFrameRMS=rms(thisFrameImage,3); %find the peaksxbeams matrix of rms values
    [~,thisFrameMaxima]=max(thisFrameRMS,[],2);
  
    
%     P.attentionCaptureThreshold=0; %for testing
    %%%%%%  Selective Attention Stage *********
    
    %compare the salience of the current frame to the 
    %time-decaying salience of the previously selected object
    
    tdSalience =  1./(1+exp(toc(P.objFileMap.Data.onsetTime))) * P.objFileMap.Data.salience ;
    
%     plot(frameCounter,tdSalience,'o');
%     drawnow;
%     hold on;
%    
    if(audioSalience>tdSalience)
        %a new object captured attention so update all the object features
        P.objFileMap.Data.salience=audioSalience;  %the current objects salience
        P.objFileMap.Data.onsetTime=tic;%take the last time stamp of the frame to be the onset time ... note that's arbitrarily inaccurate to within frameDuration
        
        %select the modal beam
        selectedBeam=mode(thisFrameMaxima(spectralPeakIndices));
        P.objFileMap.Data.angle=P.angles(selectedBeam);
        
%         display(['frame number ' num2str(frameCounter) ' had ' num2str(length(spectralPeakValues)) ' spectral peaks']);
%         plot(P.cfs,deltaAmp);
%         drawnow;
    end

    %     [x,y] = pol2cart(selectedAngle,1); %convert angle and unit radius to cartesian
    %     compass(x,y);
    %     drawnow;
    %
    
    
    %increment for next frame
    nextFrameStamp=lastFrameStamp+P.frameDuration_samples; %increment
    lastFrameStamp=nextFrameStamp;
    frameCounter=frameCounter+1;
    
    while(P.audioIn.Data(1,1).audioD(end-1,end)<nextFrameStamp)
        %spin until the next frame has been written into the buffer
    end
    
    

    
end