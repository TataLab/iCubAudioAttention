%capture audio from a YARP stream using mex code to open and close YARP
%ports and grab sound class

%applies a gammatone filterbank to aproximate the frequency decomposition
%of the low-level auditory system



pStruct.audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository

%set up some parameters of the audio grabber.  Careful: some of this is
%hard coded into the mex function.
pStruct.sampleRate=48000;
pStruct.frameDuration_samples = 4096;  %must match the settings in the mex function. should be integer multiple of the page size to keep memory access fast

%setup the shared memory for the unfiltered frames
pStruct.numMemMapFrames=10;
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

%setup to stream filtered left and right channels out
%some parameters for the filterbank
pStruct.nBands=50;
pStruct.low_cf=50; % center frequencies based on Erb scale
pStruct.high_cf=10000;
pStruct.cfs = MakeErbCFs2(pStruct.low_cf,pStruct.high_cf,pStruct.nBands);
%prime the filterbank
P_inL=zeros(8,pStruct.nBands);
P_inR=zeros(8,pStruct.nBands);
pStruct.numMemMapFrames=10;
pStruct.audioMemMapSize = pStruct.numMemMapFrames * pStruct.frameDuration_samples;  %this is a tricky part of the code.  All other processes that want to memory map this audio will need to know how big it is.  They can get that info using dir().

%prepare memory mapping
pStruct.AudioMemMapFilenameFilterL=[pStruct.audioAttentionRoot '/data/AudioMemMapFilterL.tmp'];
pStruct.AudioMemMapFilenameFilterR=[pStruct.audioAttentionRoot '/data/AudioMemMapFilterR.tmp'];


display(['memory mapping file ' pStruct.AudioMemMapFilenameFilterL ' for audio data.']);
display(['memory mapping file ' pStruct.AudioMemMapFilenameFilterR ' for audio data.']);

try
tempData = zeros(pStruct.nBands+2,pStruct.audioMemMapSize); %nbands plus  a counter channel, and a time stamp channel

%write a file for left filtered channel
fileID=fopen([pStruct.AudioMemMapFilenameFilterL],'w');
fwrite(fileID,tempData,'double');
fclose(fileID);

%write a file for right filtered channel
fileID=fopen([pStruct.AudioMemMapFilenameFilterR],'w');
fwrite(fileID,tempData,'double');
fclose(fileID);

ok=1;

catch % to not catch the error in a fancy way
    ok = 0;
end

%get ready to stream audio into the shared memory
filteredAudioOutL  = memmapfile([pStruct.AudioMemMapFilenameFilterL], 'Writable', true, 'format',{'double' [pStruct.nBands+2 pStruct.audioMemMapSize] 'audioD'});
filteredAudioOutR = memmapfile([pStruct.AudioMemMapFilenameFilterR], 'Writable', true, 'format',{'double' [pStruct.nBands+2 pStruct.audioMemMapSize] 'audioD'});


%initialize buffers to hold output
oldBuffer=zeros(4,pStruct.numMemMapFrames*pStruct.frameDuration_samples); %to store the old buffer
oldBufferFilterL=zeros(pStruct.nBands+2,pStruct.numMemMapFrames*pStruct.frameDuration_samples); %to store the old buffer
oldBufferFilterR=zeros(pStruct.nBands+2,pStruct.numMemMapFrames*pStruct.frameDuration_samples); %to store the old buffer

previousLastSample=0;
previousLastSampleTime=0;

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
    
    %keep track of how long each frame takes to warn of problems
    t=tic;
    
    [frame]=audioCapture;  %call into YARP through the mex code
    
    %do a quick check that we haven't gone off track
    if(frame(3,end)-previousLastSample~=pStruct.frameDuration_samples && previousLastSample~=0)
        display('problems....frames might be getting out of sequence');
    end
    
    
    previousLastSample=frame(3,end);
    
    
    %stream out the unfiltered frame
    newBuffer=circshift(oldBuffer,[0 -pStruct.frameDuration_samples]); %shift and wrap
    newBuffer(:,end-pStruct.frameDuration_samples+1:end)=frame;  %append the most recent frame onto the buffer by overwritting the frame that got wrapped
    audioOut.Data(1,1).audioD=newBuffer;    %dump the frame into the memmapped region
    oldBuffer=newBuffer;
    
    
    
    
    %decompose each channle using a gammatone filterbank
    %and stream out the filtered frames into two seperate files
    [fFrameL,P_outL,~,~,~]=gammatonePhase(frame(1,:),P_inL,pStruct.sampleRate, pStruct.cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [fFrameR,P_outR,~,~,~]=gammatonePhase(frame(2,:),P_inR,pStruct.sampleRate, pStruct.cfs);
    P_inR=P_outR;
    
    fFrameL=fFrameL'; %the obligatory transpose for sanity
    fFrameR=fFrameR';
    
    newBufferL=circshift(oldBufferFilterL,[0 -pStruct.frameDuration_samples]); %shift and wrap
    newBufferR=circshift(oldBufferFilterR,[0 -pStruct.frameDuration_samples]); %shift and wrap

    newBufferL(:,end-pStruct.frameDuration_samples+1:end)=[fFrameL;frame(3,:);frame(4,:)];  %append the most recent frame onto the buffer by overwritting the frame that got wrapped
    newBufferR(:,end-pStruct.frameDuration_samples+1:end)=[fFrameR;frame(3,:);frame(4,:)];  %append the most recent frame onto the buffer by overwritting the frame that got wrapped

    filteredAudioOutL.Data(1,1).audioD=newBufferL;    %dump the frame into the memmapped region
    filteredAudioOutR.Data(1,1).audioD=newBufferR;    %dump the frame into the memmapped region

    oldBufferFilterL=newBufferL;
    oldBufferFilterR=newBufferR; 
    
    
    

    
    
    
    toc(t);
    
    
%  
%     plot(audioOut.Data(1,1).audioD(1,:));
%     ylim([-1.0 1.0]);
%     drawnow;

end
