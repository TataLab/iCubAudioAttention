
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
pastAmpL=ones(P.nPastFrames,P.nBands).*.0001; %we have to seed this with some arbitrarily small numbers
pastAmpR=ones(P.nPastFrames,P.nBands).*.0001; %we have to seed this with some arbitrarily small numbers

selectedBeam=1;
selectedAngle=1;

%get some variables ready for the output of the beamformer stage
thisFrameImage=zeros(P.nBands,P.nBeams,P.frameDuration_samples); %it's only nBeamsPerHemi *2 long because we loose half the samples off either end (theoretically they're the last samples of the previous frame and the first samples of the frame that hasn't happend yet)

maxBeamsIndices=zeros(P.nBands,1);

done=0;
while(~done)
    

    %%%%%%%    Pre-Attentive Stage    **********

    %filter into bands
    [frameL,P_outL,~,~,~]=gammatone_ciM2(frame(1,:),P_inL,P.sampleRate, P.cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [frameR,P_outR,~,~,~]=gammatone_ciM2(frame(2,:),P_inR,P.sampleRate, P.cfs);
    P_inR=P_outR;
    
    %compute the amplitude of each band
    amp_frameL=rms(frameL,1);
    amp_frameR=rms(frameR,1);
    
    %compute the change in amplitude over some time
    deltaAmp_frameL=(amp_frameL-mean(pastAmpL(:,1)))./mean(pastAmpL(:,1)); %try a scaled delta amplitude
    deltaAmp_frameR=(amp_frameR-mean(pastAmpR(:,2)))./mean(pastAmpL(:,2)); %try a scaled delta amplitude
    
    %shift the past values and update the past
    pastAmpL=circshift(pastAmpL,[1 0]); %push the stack down and wrap
    pastAmpR=circshift(pastAmpR,[1 0]);
    pastAmpL(1,:)=amp_frameL; %overwrite the top of the stack
    pastAmpR(1,:)=amp_frameR;
    
    %only deal with amplitude increments
    deltaAmp_frameL(deltaAmp_frameL<0)=0;
    deltaAmp_frameR(deltaAmp_frameR<0)=0;
    
    %find peaks in the spectra from the two channels
    [spectralPeakValuesL,peaksL]=findpeaks(deltaAmp_frameL);
    [spectralPeakValuesR,peaksR]=findpeaks(deltaAmp_frameR);
    
 
    
    %handle the not-so-impossible case of zero peaks
    if(isempty(spectralPeakValuesL))
        [spectralPeakValuesL,peaksL]=max(deltaAmp_frameL); %just use the single biggest peak
    end
    
    if(isempty(spectralPeakValuesR))
        [spectralPeakValuesR,peaksR]=max(deltaAmp_frameR); %just use the single biggest peak
    end
    
    audioSalienceL=sum(spectralPeakValuesL)*length(spectralPeakValuesL);
    audioSalienceR=sum(spectralPeakValuesR)*length(spectralPeakValuesR);
    audioSalience=(audioSalienceL+audioSalienceR)/2;
    
    plot(lastFrameStamp,audioSalience,'o');
    hold on;
    drawnow;
    
    
    frameL=frameL';
    frameR=frameR';
    
    
    %beamformer
    for i=1:P.nBands
        thisBandL=frameL(i,:);
        thisBandR=frameR(i,:);

        thisFrameImage(i,:,:)=thisBandL(P.lIndex)+thisBandR(P.rIndex); %compute all the beams in one step = MATLAB is fast
    end

    %find the angle with the most maxima
    thisFrameRMS=rms(thisFrameImage,3); %find the peaksxbeams matrix of rms values
    [~,thisFrameMaxima]=max(thisFrameRMS,[],2);
    
    
    
    %%%%%%  Selective Attention Stage *********
    %select the modal beam
    if(audioSalience>P.attentionCaptureThreshold)
    selectedBeam=mode(thisFrameMaxima);
    selectedAngle=P.angles(selectedBeam);
    end
    
%     plot(lastFrameStamp,audioSalience,'o');
%     hold on;
%     drawnow;
    
%     [x,y] = pol2cart(selectedAngle,1); %convert angle and unit radius to cartesian
%     compass(x,y);
%     drawnow;
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
    display('next frame');
    
end