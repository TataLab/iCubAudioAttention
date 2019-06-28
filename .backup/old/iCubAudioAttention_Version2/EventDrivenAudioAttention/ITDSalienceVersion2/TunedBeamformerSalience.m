%TunedBeamformerSalience models Jeffress-like frequency-tuned coincidence
%detectors as banks of narrow-band delay-and-sum beamformers


display(['Running TunedBeamformerSalience using: ' mfilename('fullpath')]);


ConfigureAudioParameters;  %call the script that sets up the P parameter structure


%initialize YARP if you want to send angles to an iCub
if(P.sendAngleToYarp==1)
    InitializeYARP;
end

% %for development, build a test frame
% t=linspace(0,2*pi,P.sampleRate);
% s=sin(1000*t);
% s=[s;s]; %make it stereo
% clear t;

% %for testing with a realistic signal
% [s,SamplingFre]=readwav('./audioTest_right_to_left.wav');
% [s,SamplingFre]=readwav('./audioTest_right_to_left_with_distractor.wav');
% s=s';

%configure an audio recorder for grabbing frames
A=audiorecorder(P.sampleRate,P.bitDepth,P.numChannels);


%initialize an array to store past rms amplitude values for each bin
pastAmp=ones(P.nBands,P.nPast_frames).*.0001; %we have to seed this with some arbitrarily small numbers
pastAngles=ones(1,P.nAngles);  %representation of when the last time each angle in the bank of beams was hit

attentionFocused=0; %provide a flag that let's you lock out attention capture

attendThisAngle=0; %this is the angle we'll hand off to iCub; only update this angle if we decide to capture attention

frameCounter=0;
done=0;
while (~done)  %loop continuously handling audio in a spatialy sort of way
    t=tic;
    frameCounter=frameCounter+1;
    
    %grab audio data frame
    %replace this with actual audio capture when we figure that out
    %[audioFrameL,audioFrameR,s]=GetCurrentAudioFrame_prerecorded(s,P.frameDuration_samples);
    [audioFrameL,audioFrameR]=GetCurrentAudioFrame_builtin(P.frameDuration_seconds,A);
   
    
    %filter it into bands (using Mohamad's filterbank code)
   
   [GF_frameL,envL,delayL] = gammatoneFast(audioFrameL,P.cfs,P.sampleRate,P.align) ;
   [GF_frameR,envR,delayR] = gammatoneFast(audioFrameR,P.cfs,P.sampleRate,P.align) ;

    %compute the energy in each band
    amp_frameL=rms(GF_frameL,2);
    deltaAmp_frameL=(amp_frameL-mean(pastAmp,2))./mean(pastAmp,2); %try a scaled delta amplitude
    deltaAmp_frameL(deltaAmp_frameL<0)=0; %only deal with amplitude increments (which should be onsets) for now
    plot(deltaAmp_frameL(:));
    ylim([0 100]);
    drawnow;
    
    %update the stored background with the current frame by overwritting
    %the oldest
    pastAmp=circshift(pastAmp,[0 -1]); %shift and wrap
    pastAmp(:,end)=amp_frameL; %overwrite
    
    %find the most representative bands
    [GF_peakValues,GF_peakIndices] = findpeaks(deltaAmp_frameL);
    currentNumPeaks=size(GF_peakIndices,1); %each frame can have a different number of peaks so grab it here
    %display(currentNumPeaks);
    
    %compute a strange index of how important this frame might be
    attentionCaptureScore=sum(GF_peakValues)*currentNumPeaks;
    
    
    %only proceed to process this frame if you found peaks and attention
    %isn't locked
    if (attentionCaptureScore>P.attentionCaptureThreshold && attentionFocused==0)
        
        %pull those bands out of the larger bank of filtered signals (to reduce
        %beamformer computation time later on
        GF_peakFrameL=GF_frameL(GF_peakIndices,:);
        GF_peakFrameR=GF_frameR(GF_peakIndices,:);
        
        %initialize a sheet of zeros to hold all the beams in all the bands we
        %want
        currentBeams=zeros(currentNumPeaks,P.nAngles,P.frameDuration_samples-P.nAngles);
        
        
        %for each filter band in the bank, sweep a beam
        for bandNum=1:currentNumPeaks; %for each beam selected for this frame
            
            for i=P.nAnglesPerHemifield+1:size(currentBeams,3)
                
                %arbitrarily lag right channel +/- relative to left
                currentBeams(bandNum,:,i)=GF_peakFrameL(bandNum,i)+GF_peakFrameR(bandNum,i-P.nAnglesPerHemifield:i+P.nAnglesPerHemifield);  %this line actually computes the beam
            end
            
            
        end
        
        %find the single beam in each band with the highest rms power
        beamRMS=rms(currentBeams,3);
        [maxBeams,maxBeamIndices]=max(beamRMS,[],2);
        
        %rescore angles according to when they were last excited, then average
        %the best ones
        currentAngleScore=pastAngles(maxBeamIndices);
        averageTheseAngles=P.angles(maxBeamIndices);
        %averageAngle= mean(averageTheseAngles.*currentAngleScore)./sum(currentAngleScore);
        averageAngle= mean(averageTheseAngles);
        attendThisAngle=averageAngle;
        display(['Orient to ' num2str(180/pi*averageAngle) ' degrees']);
        
        %update previous angles
        pastAngles=pastAngles+1; %incremement all the angle counters
        pastAngles(maxBeamIndices)=1; %but reset the current angles to 1
        %done
        %%%%%%
        
    end
    
    
    
    %%%%%%%
%     %plot
%     [xpeak,ypeak]=pol2cart(P.angles(maxBeamIndices),1);
%     compass(xpeak,ypeak,'r');
%     
%     drawnow;
    %done plotting
    %%%%%%%%%
    
    
end