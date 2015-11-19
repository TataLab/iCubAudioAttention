

%Captures audio by opening a pre-recorded file and scaning through it
%frame-by-frame.  Exposes the frame to other matlab processes by memory
%mapping it as a file.  Note that the path to your audio data files will be
%different


audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository
audioFileName=[audioAttentionRoot '/data/sounds/audioTest_right_to_left.wav'];
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



%setup to stream filtered left and right channels out
%some parameters for the filterbank
nBands=10;
low_cf=50; % center frequencies based on Erb scale
high_cf=10000;
cfs = MakeErbCFs2(low_cf,high_cf,nBands);
%prime the filterbank
P_inL=zeros(8,nBands);
P_inR=zeros(8,nBands);
audioMemMapSize = numMemMapFrames * frameDuration_samples;  %this is a tricky part of the code.  All other processes that want to memory map this audio will need to know how big it is.  They can get that info using dir().

%prepare memory mapping
AudioMemMapFilenameFilterL=[audioAttentionRoot '/data/AudioMemMapFilterL.tmp'];
AudioMemMapFilenameFilterR=[audioAttentionRoot '/data/AudioMemMapFilterR.tmp'];


display(['memory mapping file ' AudioMemMapFilenameFilterL ' for audio data.']);
display(['memory mapping file ' AudioMemMapFilenameFilterR ' for audio data.']);

try
tempData = zeros(nBands+2,audioMemMapSize); %nbands plus  a counter channel, and a time stamp channel

%write a file for left filtered channel
fileID=fopen([AudioMemMapFilenameFilterL],'w');
fwrite(fileID,tempData,'double');
fclose(fileID);

%write a file for right filtered channel
fileID=fopen([AudioMemMapFilenameFilterR],'w');
fwrite(fileID,tempData,'double');
fclose(fileID);

ok=1;

catch % to not catch the error in a fancy way
    ok = 0;
end

%get ready to stream audio into the shared memory
filteredAudioOutL  = memmapfile([AudioMemMapFilenameFilterL], 'Writable', true, 'format',{'double' [nBands+2 audioMemMapSize] 'audioD'});
filteredAudioOutR = memmapfile([AudioMemMapFilenameFilterR], 'Writable', true, 'format',{'double' [nBands+2 audioMemMapSize] 'audioD'});


%initialize buffers to hold output
oldBuffer=zeros(4,numMemMapFrames*frameDuration_samples); %to store the old buffer
oldBufferFilterL=zeros(nBands+2,numMemMapFrames*frameDuration_samples); %to store the old buffer
oldBufferFilterR=zeros(nBands+2,numMemMapFrames*frameDuration_samples); %to store the old buffer


% recordedAudio=zeros(1,75*frameDuration_samples);
    
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
    
    
    %now compute and write out filterbank
    
    
    %decompose each channle using a gammatone filterbank
    %and stream out the filtered frames into two seperate files
    [fFrameL,P_outL,~,~,~]=gammatonePhase(frame(1,:),P_inL,sampleRate, cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [fFrameR,P_outR,~,~,~]=gammatonePhase(frame(2,:),P_inR,sampleRate, cfs);
    P_inR=P_outR;
    
    
    fFrameL=fFrameL'; %the obligatory transpose for sanity
    fFrameR=fFrameR';
    
    newBufferL=circshift(oldBufferFilterL,[0 -frameDuration_samples]); %shift and wrap
    newBufferR=circshift(oldBufferFilterR,[0 -frameDuration_samples]); %shift and wrap

    newBufferL(:,end-frameDuration_samples+1:end)=[fFrameL;frame(3,:);frame(4,:)];  %append the most recent frame onto the buffer by overwritting the frame that got wrapped
    newBufferR(:,end-frameDuration_samples+1:end)=[fFrameR;frame(3,:);frame(4,:)];  %append the most recent frame onto the buffer by overwritting the frame that got wrapped

    filteredAudioOutL.Data(1,1).audioD=newBufferL;    %dump the frame into the memmapped region
    filteredAudioOutR.Data(1,1).audioD=newBufferR;    %dump the frame into the memmapped region

    oldBufferFilterL=newBufferL;
    oldBufferFilterR=newBufferR; 
    
    %in case you need to record what you're writing out
%     frameR=sum(fFrameR);
%       
%     recordedAudio=circshift(recordedAudio,[0 -frameDuration_samples]);
%     recordedAudio(1,end-frameDuration_samples+1:end)=frameR;
%     if(frameCounter>75)
%         done=1;
%     end
%     
    
    
    while(toc(t)<frameDuration_seconds)
        %spin
    end
    
    toc(t);
    
end