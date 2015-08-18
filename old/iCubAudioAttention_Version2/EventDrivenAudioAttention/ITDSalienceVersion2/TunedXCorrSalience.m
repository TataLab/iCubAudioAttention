%TunedXCorrSalience models Jeffress-like frequency-tuned coincidence
%detectors as banks of cross-correlations


display(['Running TunedXCorrSalience using: ' mfilename('fullpath')]);



ConfigureAudioParameters;  %call the script that sets up the P parameter structure


%initialize YARP if you want to send angles to an iCub
if(P.sendAngleToYarp==1)
    InitializeYARP;
end

[s,SamplingFre]=readwav('./audioTest_right_to_left.wav');
s=s';

%initialize a sheet of zeros to hold all the beams in all the bands
%note that in the xcorr case a "beam" is a single number per frame
currentBeams=zeros(P.nBands,P.nAngles);

pastBeams=zeros(P.nPast_frames,P.nBands,P.nAngles);


readIndex=1;
done=0;
while (~done)  %loop continuously handling audio in a spatialy sort of way
    t=tic;
    
    %grab audio data frame
    %replace this with actual audio capture when we figure that out
    [audioFrameL,audioFrameR,s]=GetCurrentAudioFrame_prerecorded(s,P.frameDuration_samples);
    
    %filter it into bands (using Mohamad's filterbank code)
   [GF_frameL,envL,delayL] = gammatoneFast(audioFrameL,P.cfs,P.sampleRate,P.align) ;
   [GF_frameR,envR,delayR] = gammatoneFast(audioFrameR,P.cfs,P.sampleRate,P.align) ;

%      GF_frameL=zeros(45,4096);
%      GF_frameR=GF_frameL;
%     
    
    %for each filter band in the bank by running xcorr over the two
    %channels
    for bandNum=1:P.nBands
                
        currentBeams(bandNum,:)=xcorr(GF_frameL(bandNum,:),GF_frameR(bandNum,:),P.nAnglesPerHemifield); %only running the middle nAngles because this falls between +/- max lag
        
    end
    
    %perform normalization and find the peaks
    M=squeeze(mean(pastBeams,1));
    D=squeeze(std(pastBeams,1));
    normalizedBeams=(currentBeams-M);
    normalizedBeams(normalizedBeams<0)=0;
    plot(normalizedBeams(13,:));
    ylim([-5 5]);
    drawnow;
    
    display(max(max(normalizedBeams)));
    %circshift pastBeams by one complete frame so we can overwrite data from this frame into the
    %right-end position
    pastBeams=circshift(pastBeams,[-1 0 0]);
    pastBeams(end,:,:)=currentBeams;

    %display(toc(t));

end