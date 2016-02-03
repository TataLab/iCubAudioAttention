
% Copyright (C) 2015 Matthew Tata
% email: matthew.tata@uleth.ca
%
% Permi ssion is granted to copy, distribute, and/or modify this program
% under the terms of the GNU General Public License, version 2 or any
% later version published by the Free Software Foundation.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
% Public License for more details




%capture audio from a YARP stream using mex code to open and close YARP
%ports and grab sound class

%optionally use the excellent Psychophysics Toolbox to stream audio out to
%the local audio device


   
    %how this works:  audioCapture is a mex function that calls a "blocking
    %read" on a YARP port.  This means everything stops until the next
    %frame is fully captured from the audio buffer.  audioCapture then
    %returns a 4 x frameDuration_samples matrix in which the first two rows
    %are left and right audio data, respectively, the third row contains a
    %sample sequence stamp, and the fourth row contains timestamps synched
    %to the PC104.  
    
    %If you're using memory mapping to expose these frames to other
    %instances of MATLAB, it will use a buffer of pStruct.numMemMapFrames.
    %Every time a new frame is read, the columns of the buffer 
    %are shifted left and the new frame is appended.  This way the newest
    %sample is always the right-most sample in the buffer
    
    
    %If all goes well this should execute with plenty of time to
    %call audioCapture() again on the next loop before the next frame is
    %fully captured. Use tic() and toc() to test (warning: tic() and toc()
    %don't work well on Windows, but do work well on any UNIX including Mac
    %OS)



%build a data structure of parameters to keep things organized
pStruct.streamAudioOutput = 0; %flag to toggle on streaming
pStruct.writeToMemMap = 1;  %flag to toggle on writing stereo signal to local shared memory (use this to expose the signal to other MATLAB instances)

%set up some parameters of the audio grabber.  Careful: some of this is
%hard coded into the mex function.
pStruct.sampleRate=48000;
pStruct.frameDuration_samples = 4096;  %must match the settings in the mex function. should be integer multiple of the page size to keep memory access fast
pStruct.frameDuration_seconds=pStruct.frameDuration_samples/pStruct.sampleRate;

if(pStruct.streamAudioOutput)%setup psychophysics toolbox for sound output
    InitializePsychSound(1);
    bufferSize=4096;
    paoutput=PsychPortAudio('Open', [], 1, 0, pStruct.sampleRate, 2,bufferSize);
    pre=zeros(2,2*pStruct.frameDuration_samples); %dummy data to fill the first buffer
    PsychPortAudio('FillBuffer', paoutput, pre);
    pause(1); %give PsychPortAudio a moment to compose itself
    playbackstart = PsychPortAudio('Start', paoutput, 0,0,1); %start the stream playing back
    nextSampleStartIndex=1;
    PsychPortAudio('Verbosity',0); %toggle this if you are sure your audio is OK and you don't need to be told that PsychPortAudio is always 1 frame behind
end


if(pStruct.writeToMemMap) %set up to use memory mapping to expose the audio to other MATLAB instances
    
    pStruct.audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository
    pStruct.numMemMapFrames=10; %think of this as echoic memory:  a raw audio data buffer
    pStruct.audioMemMapSize = pStruct.numMemMapFrames * pStruct.frameDuration_samples;  %this is a tricky part of the code.  All other processes that want to memory map this audio will need to know how big it is.  They can get that info using dir().
    %prepare memory mapping
    pStruct.AudioMemMapFilename=[pStruct.audioAttentionRoot '/data/AudioMemMap.tmp'];
    display(['memory mapping file ' pStruct.AudioMemMapFilename ' for audio data.']);    
    try
        tempData = zeros(4,pStruct.audioMemMapSize); %2 audio channels, a counter channel, and a time stamp channel
        fileID=fopen(pStruct.AudioMemMapFilename,'w');
        fwrite(fileID,tempData,'double');
        fclose(fileID);
        ok=1;        
    catch % to not catch the error in a fancy way
        ok = 0;
    end  
    %get ready to stream audio into the shared memory
    audioOut  = memmapfile(pStruct.AudioMemMapFilename, 'Writable', true, 'format',{'double' [4 pStruct.audioMemMapSize] 'audioD'});
    oldBuffer=zeros(4,pStruct.numMemMapFrames*pStruct.frameDuration_samples); %to store the un-updated  memory map from 
    previousLastSample=0;
    previousLastSampleTime=0;
end %setting up mem mapping for unfiltered stereo signal



%%%%%%%%%%
%setup is complete
%start grabbing audio
%%%%%%%%%%%
%%%%%%
%grab an initial frame to start
% [frame]=audioCapture;  
% [under,nextSampleStartIndex]=PsychPortAudio('FillBuffer',paoutput,frame(1:2,:),1,nextSampleStartIndex);


frameCounter=1;
done=0;
while(~done) %loop continuously
   
    
    
    [frame]=audioCapture; %grab audio from YARP.  This is a blocking read.  It waits until the buffer fills.  It doesn't play nicely with streaming refills of the PsychPortAudio buffer, so if you're monitoring the audio out then you'll always be a frame behind and possibly dropping samples  
    %now you have audio data
    %do something with it
    
    
    t=tic;  %keep track of time.  you need to recall audioCapture() before 1 frame has elapsed or you'll drop samples
    
    if(pStruct.streamAudioOutput) %if you want to monitor the audio on the local audio hardware
        [under,nextSampleStartIndex]=PsychPortAudio('FillBuffer',paoutput,frame(1:2,:),1,nextSampleStartIndex);     
    end
    
    if(pStruct.writeToMemMap) %if you want to write the frame into a buffer of shared memory
        newBuffer=circshift(oldBuffer,[0 -pStruct.frameDuration_samples]); %shift and wrap
        newBuffer(:,end-pStruct.frameDuration_samples+1:end)=frame;  %append the most recent frame onto the buffer by overwritting the frame that got wrapped
        audioOut.Data(1,1).audioD=newBuffer;%dump the frame into the memmapped region
        oldBuffer=newBuffer;   
    end
    
%     %if you want to visualize the audio (this may slow too much)
    subplot(2,1,1);
    plot(audioOut.Data(1,1).audioD(1,:));
    ylim([-1.0 1.0]);
    subplot(2,1,2);
    plot(audioOut.Data(1,1).audioD(2,:));
    ylim([-1.0 1.0]);
    drawnow;

%check the timing
frameCounter=frameCounter+1;
elapsed=toc(t);
if(elapsed>pStruct.frameDuration_seconds)
    display(['frame number ' num2str(frameCounter) ' ran slow by ' num2str(elapsed - pStruct.frameDuration_seconds) ' seconds.  Samples may have been dropped.']);
end

end
