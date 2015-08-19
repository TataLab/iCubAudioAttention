

%Captures audio by opening a pre-recorded file and scaning through it
%frame-by-frame.  Exposes the frame to other matlab processes by memory
%mapping it as a file.  Note that the path to your audio data files will be
%different


audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository
[s,sampleRate]=readwav([audioAttentionRoot '/data/sounds/audioTest_right_to_left.wav']);
s=s';  %easier to think in row vectors

sLength=length(s);

frameDuration_samples = 4096;  %should be integer multiple of the page size to keep memory access fast
frameDuration_seconds= frameDuration_samples/sampleRate;

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

    
readIndex=1;
frameCounter=1;

%pre allocate vectors to hold frame count and sample time stamps
frameCountVector=ones(1,frameDuration_samples);
timeStampVector=zeros(1,frameDuration_samples);

%set up the filter
P_inL=zeros(8,2);
P_inR=zeros(8,2);
cfs=MakeErbCFs2(500,5000,15);

while(readIndex + frameDuration_samples < sLength)  %until you reach the end of the file
   
    t=tic;
    
    sampleIndex=readIndex:readIndex+frameDuration_samples-1; %where to read out of the input file
    
    %read audio out of file
    frameL=s(1,sampleIndex); %pull out the left and right channels
    frameR=s(2,sampleIndex);
    
    %try filtering
    [GF_frameL,P_outL,~,~,~]=gammatone_ciM2(frameL,P_inL,sampleRate, cfs);
    P_inL=P_outL; 
    [GF_frameR,P_outR,~,~,~]=gammatone_ciM2(frameR,P_inR,sampleRate, cfs);
    P_inR=P_outR;
    
    plot(isnan(GF_frameL));
    drawnow;
    
    
    
    %compute the two vectors of index data: time and sample number
    sampleStampVector=sampleIndex;
    timeStampVector=sampleIndex*(1/sampleRate);  %when caputuring real-time audio from a robot you'll use the time stamps provided by the grabber

    %write the data into the memory mapped region 
    memMapWriteIndex=(mod(frameCounter,numMemMapFrames)*frameDuration_samples+1); %keep track of which frame you're writting into.  Using modulus to "wrap" 

    audioOut.Data(1,1).audioD(1,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1) = frameL;
    audioOut.Data(1,1).audioD(2,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1) = frameR;
    audioOut.Data(1,1).audioD(3,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1) = sampleStampVector;
    audioOut.Data(1,1).audioD(4,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1) = timeStampVector;

    %incremement counters
    readIndex=readIndex+frameDuration_samples;
    frameCounter=frameCounter+1;

  
    while(toc(t)<frameDuration_seconds)
        %spin
    end
    
    
end