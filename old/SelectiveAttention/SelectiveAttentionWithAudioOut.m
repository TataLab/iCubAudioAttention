%perform initialization and setup;  get back a struct with all the settings
%we'll need

addpath('/Users/Matthew/Documents/Robotics/iCubAudioAttention/MATLAB/SelectiveAttention/Functions_and_Scripts');
P=ConfigureParameters;




%setup psychophysics toolbox for sound output
InitializePsychSound(1);
bufferSize=4096;
paoutput=PsychPortAudio('Open', [], 1, 0, 44100, 2,bufferSize);
pre=zeros(2,2*P.frameDuration_samples);
PsychPortAudio('FillBuffer', paoutput, pre);
playbackstart = PsychPortAudio('Start', paoutput, 0,0,1);
nextSampleStartIndex=1;

recordedAudio=zeros(1,100*P.frameDuration_samples);


frameCounter=1;
done=0;
while(~done)
    
    
    
   %this reads unfiltered audio
    frameL=P.audioIn.Data(1,1).audioD(1,end-P.frameDuration_samples+1:end);
    frameR=P.audioIn.Data(1,1).audioD(2,end-P.frameDuration_samples+1:end);

    frame=frameL+frameR;
    frame=repmat(frame,[2 1]);
    
    
    
%     %this reads output of the filterbank
%     frameBandsL=P.audioInL.Data(1,1).audioD(1:P.nBands,end-P.frameDuration_samples+1:end);
%     frameBandsR=P.audioInR.Data(1,1).audioD(1:P.nBands,end-P.frameDuration_samples+1:end);
% 
% 
%     
%     frameL=sum(frameBandsL);
%     frameR=sum(frameBandsR);
   
    [under,nextSampleStartIndex]=PsychPortAudio('FillBuffer',paoutput,[frame],1,nextSampleStartIndex);
    
%     in case you need to record what you're reading
%     recordedAudio=circshift(recordedAudio,[0 -P.frameDuration_samples]);
%     recordedAudio(1,end-P.frameDuration_samples+1:end)=frameL;
%     frameCounter=frameCounter+1;
%     if(frameCounter>100)
%         done=1;
%     end
%     
    
end