
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

frameCounter=1;
done=0;
while(~done) %loop continuously
   
   
    
    [frame]=audioCapture;  %call into YARP through the mex code
    
    t=tic;
    %figure out where to write the current frame into the ring buffer
    memMapWriteIndex=(mod(frameCounter,numMemMapFrames)*frameDuration_samples+1); %keep track of which frame you're writting into.  Using modulus to "wrap" 

    %dump the frame into the memmapped region
    audioOut.Data(1,1).audioD(:,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1);
     
    frameCounter=frameCounter+1;
    
    display(toc(t));
end
