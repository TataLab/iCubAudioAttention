
%SendAudio monitors the object stack.  When a selected object
%appears at the top of the stack, SendAudio will optionaly process the signal and send the signal out the line-out port using
%Psychophysics Toolbox Portaudio interface




%load parameters
LoadParameters;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%memory map the incoming audio data dump file
%and get the audio stream queueueued up
%the fixed lag is important: since the run loop below runs through small
%frames as fast as PsychPortAudio fills them into the output buffer, it
%doesn't have the same time scale as other modules.  Since other modules
%are working over hundreds of ms, this module has to "look back in time" to
%recover relevant audio data.  How far back?  It should at least account
%for the difference in frame sizes between the attention orienting modules
%and the frames sent to PsychPortAudio's buffer (e.g. 10240 vs. 1024 so it
%should look back 10 frames).  But also account fixed lag in those modules!

[mIn,sampleD]=OpenAudioInputData;
tempMostRecentSample=sampleD.Data(1,1).f;
readIndex = tempMostRecentSample - (P.kFrameSize_samples - 1) - P.kFixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time

pre=zeros(2,P.kFrameSize_samples*2);%make a vector to preload into the buffer
silence=zeros(2,P.kFrameSize_samples);%send this when we don't want to send audio
InitializePsychSound(1);%set up audio output
paoutput = PsychPortAudio('Open', [], 1, 2, 48000, 2,1024);

%make a big buffer to store an entire phrase for later use (or debugging)
%longBuffer=zeros(2,P.kLongBufferSize_samples);

%get a frame ready
audiodata=mIn.Data(1,1).d(:,readIndex:readIndex+P.kFrameSize_samples-1); %the first frame is ready

scaleFactor=1/2^15;  %precompute for speed
audiodata=double(audiodata); %scale the audio from 16bit signed ints to .wav
audiodata=audiodata.*scaleFactor;

%initialize a vector to hold processed data
processedAudiodata=zeros(1,length(audiodata));

%initialize a stereo array to hold output audio data
outputAudiodata=zeros(2,length(audiodata));

%start playback
PsychPortAudio('FillBuffer', paoutput, pre);
playbackstart = PsychPortAudio('Start', paoutput, 0,0,1);

%%%%%%%%
%done setting up audio
%%%%%%%%


%%%%%%%%%%%%%%%%%%%%
%memory map the object stack
[objFileMap,~,~,isBusyMap]=MapObjectFile;



%%%%%%%%%%%%
%loop
done=0;
frameT=tic;
while(~done)
    %clc;
    %first immediately fill the buffer with the next frame
    %if the object is selected
    display(['that frame took ' num2str(toc(frameT)) ' seconds']);
    frameT=tic;
    while(isBusyMap.Data(1,1).isBusy==1)
        %the object stack is being written by another process, so block here
        display('waiting patiently while some else is modifying the object stack');
    end
    
    if(objFileMap.Data(1,1).isSelected==1  ) %there is an attended object on the stack
        
        display('sending audio');
       
        %%%%%%%%%%%
        %speech preprocessing section
        %%%%%%%%%

        %initialize
        processedAudioData=zeros(1,length(audiodata)); 
        
        %channel select
        processedAudiodata=SelectChannels(audiodata,objFileMap.Data(1,1).onsetAzimuth); %isolate the most relevant mono signal
        
        %Use runica() ICA
        %audioData=SeperateSources(audiodata,objFileMap.Data(1,1).onsetAzimuth);
        
        %use VoiceBox specsub enhancement
        processedAudiodata=EnhanceThis(processedAudiodata,P.kSampleRate);
        
        %use VoiceBox specsub filtering
        %processedAudiodata=FilterWithVoicebox(processedAudiodata,P.kSampleRate);
        
        %adjust gain
        processedAudiodata=processedAudiodata*P.kGain;
        
        %%%%%%%%%%%
        %output stage
        %%%%%%%%%%%%%
        
        %expand the mono signal onto both channels
        outputAudiodata(1,:)=processedAudiodata;
        outputAudiodata(2,:)=processedAudiodata;
        
        plot(processedAudiodata);
        ylim([-1 1]);
        drawnow;
        
        [underflow,~,~]=PsychPortAudio('FillBuffer', paoutput, outputAudiodata,1);
        
        
         %long buffer the data
%          longBuffer=circshift(longBuffer, [0 -1*P.kFrameSize_samples]); %it's a kind of ring buffer
%          longBuffer(:,end-P.kFrameSize_samples+1:end)=audiodata;
%          
%          plot(longBuffer(1,:));
%          hold on;
%          drawnow;
    else
        [underflow,~,~]=PsychPortAudio('FillBuffer', paoutput, silence,1); %else send zeros, how's that for early selection!
        %display('sssshhhhh');
    end
    
    %prepare next frame
    readIndex=readIndex+P.kFrameSize_samples;
    audiodata=mIn.Data(1,1).d(:,readIndex:readIndex+P.kFrameSize_samples-1); 

    %scale the audio from signed 16-bit ints to doubles between -1 and 1
    audiodata=double(audiodata).*scaleFactor;
   
    
   %keep looping fast, the sound driver will take care of the rest
   %ICLUDING TIME because PsychPortAudio will wait until the frame fills
   %into the buffer....note that this means the run loop here will not have
   %the same rate as other loops
    
    
    
end