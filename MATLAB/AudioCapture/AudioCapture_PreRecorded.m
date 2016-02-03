%Captures audio by opening a pre-recorded file and scaning through it
%frame-by-frame.  Exposes the frame to other matlab processes by memory
%mapping it as a file.  Note that the path to your audio data files will be
%different


audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository
audioFileName=[audioAttentionRoot '/MATLAB/AudioCapture/test.wav'];
disp(audioFileName);
[s,sampleRate]=audioread(audioFileName);
s=s';  %easier to think in row vectors

sLength=length(s);

frameDuration_samples = 4096;  %should be integer multiple of the page size to keep memory access fast
frameDuration_seconds= frameDuration_samples/sampleRate;

numMemMapFrames=200;
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

%initialize buffer to hold output
oldBuffer=zeros(4,numMemMapFrames*frameDuration_samples); %to store the old buffer

readIndex=1;
frameCounter=1;
done=0;

while(done==0 && readIndex + frameDuration_samples < sLength)  %until you reach the end of the file
   
    t=tic;
    
    sampleIndex=readIndex:readIndex+frameDuration_samples-1; %where to read out of the input file
    
    %read audio out of file
    frame(1,:)=s(1,sampleIndex); %pull out the left and right channels
    frame(2,:)=s(2,sampleIndex);

    
    %compute the two vectors of index data: time and sample number
    frame(3,:)=sampleIndex;
    frame(4,:)=sampleIndex*(1/sampleRate);  %when caputuring real-time audio from a robot you'll use the time stamps provided by the grabber

   
    newBuffer=circshift(oldBuffer,[0 -frameDuration_samples]); %shift and wrap
    newBuffer(:,end-frameDuration_samples+1:end)=frame;  %append the most recent frame onto the buffer by overwritting the frame that got wrapped

    
    %dump the frame into the memmapped region
    audioOut.Data(1,1).audioD=newBuffer;
    oldBuffer=newBuffer; %store for the next frame
    

    %incremement counters
    readIndex=readIndex+frameDuration_samples;
    frameCounter=frameCounter+1;
    
    
    while(toc(t)<frameDuration_seconds)
        %spin
    end
    
    toc(t);
    
end