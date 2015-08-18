ITDConfigureAudioParameters;

audio=wavread('/Users/Matthew/Desktop/junk.wav');
audio=audio';

InitializePsychSound(1);

oPort=OpenAudioOutput(P);
pre=zeros(P.outputNumChans,P.outputBufferSize);
PsychPortAudio('FillBuffer',oPort,audio);
PsychPortAudio('Start',oPort,0,0,1);

while (length(audio)>=P.outputFrameSize)
    
    [underflow,~,~]=PsychPortAudio('FillBuffer',oPort,audio(:,1:P.outputFrameSize),1);
    if(underflow~=0) 
        display('problems'); 
    end;
    
    audio(:,1:P.outputFrameSize)=[];
    display([num2str(length(audio)) ' samples left to play']);
    
end


PsychPortAudio('Stop',oPort);
PsychPortAudio('Close');
