
%capture audio from a YARP stream using mex code to open and close YARP
%ports and grab sound class



audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository

%set up some parameters of the audio grabber.  Careful: some of this is
%hard coded into the mex function.
sampleRate=48000;
frameDuration_samples = 4096;  %must match the settings in the mex function. should be integer multiple of the page size to keep memory access fast

numMemMapFrames=10;
audioMemMapSize = numMemMapFrames * frameDuration_samples;  %this is a tricky part of the code.  All other processes that want to memory map this audio will need to know how big it is.  They can get that info using dir().


%prepare memory mapping
AudioMemMapFilename=[audioAttentionRoot '/data/AudioMemMap.tmp'];
display(['memory mapping file ' AudioMemMapFilename ' for audio data.']);

try
tempData = zeros(4,audioMemMapSize); %2 audio channels, a counter channel, and a time stamp channel
fileID=fopen(AudioMemMapFilename,'w');
fwrite(fileID,tempData,'double');
fclose(fileID);
ok=1;

catch % to not catch the error in a fancy way
    ok = 0;
end

audioOut  = memmapfile(AudioMemMapFilename, 'Writable', true, 'format',{'double' [4 audioMemMapSize] 'audioD'});

oldBuffer=zeros(4,numMemMapFrames*frameDuration_samples); %to store the old buffer


frameCounter=1;
done=0;
while(~done) %loop continuously
   
   
    %how this works:  audioCapture is a mex function that calls a "blocking
    %read" on a YARP port.  This means everything stops until the next
    %frame is fully captured from the audio buffer.  audioCapture then
    %returns a 4 x frameDuration_samples matrix in which the first two rows
    %are left and right audio data, respectively, the third row contains a
    %sample sequence stamp, and the fourth row contains timestamps synched
    %to the PC104.  Every time a new frame is read, the columns of the buffer 
    %are shifted left and the new frame is appended.  This way the newest
    %sample is always the right-most sample in the buffer
    
    %All of this is dumped into a memmory mapped file for
    %speed.  
    
    %If all goes well this should execute with plenty of time to
    %call audioCapture again on the next loop before the next frame is
    %fully captured.
    
    [frame]=audioCapture;  %call into YARP through the mex code
    
    
    newBuffer=circshift(oldBuffer,[0 -frameDuration_samples]); %shift and wrap
    newBuffer(:,end-frameDuration_samples+1:end)=frame;  %append the most recent frame onto the buffer by overwritting the frame that got wrapped

    
    %dump the frame into the memmapped region
    audioOut.Data(1,1).audioD=newBuffer;
    oldBuffer=newBuffer;
    
    if(mod(frameCounter,20)==0)
        display(['Capturing audio from YARP.  Last sample written was ' num2str(frame(3,end))]);
    end
    
    plot(audioOut.Data(1,1).audioD(1,:));
    ylim([-1.0 1.0]);
    drawnow;
    
    frameCounter=frameCounter+1;
    
%     %check
%     frame=audioOut.Data(1,1).audioD;
%     plot(frame(1,:));
%     drawnow;
%     
end
