%AudioCapture_MemMap pulls left and right channel data into the workspace
%from a memmory mapped file.  This is useful for getting audio from YARP in
%a raw format

%audio data should be dumped as a 4 x frameDuration_samples matrix in which the first two rows
    %are left and right audio data, respectively, the third row contains a
    %sample sequence stamp, and the fourth row contains timestamps synched
    %to the PC104.  
    
    %If you're using memory mapping to expose these frames to other
    %instances of MATLAB, it will use a buffer of pStruct.numMemMapFrames.
    %Every time a new frame is read, the columns of the buffer 
    %are shifted left and the new frame is appended.  This way the newest
    %sample is always the right-most sample in the buffer
    
    
    

%build a data structure of parameters to keep things organized
pStruct.streamAudioOutput = 1; %flag to toggle on streaming
pStruct.writeToMemMap = 1;  %flag to toggle on writing stereo signal to local shared memory (use this to expose the signal to other MATLAB instances)

totalTime=tic;
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
    pStruct.AudioMemMapFilename='/tmp/AudioMemMap.tmp';
    disp(['memory mapping file ' pStruct.AudioMemMapFilename ' for audio data.']);    
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

%%%%%%
%parameters for interacting with memory mapped input audio
%%%%%
memMapFileName_input='/tmp/preprocessedRawAudio.tmp';
inputDir=dir(memMapFileName_input);
P.bufferSize_bytes = inputDir.bytes; %the  buffer size is determined by your audio capture method.  Frames on that side are hard coded to be 4096 samples.  There are 4 rows by 4096 doubles x some number of frames in the  buffer.
P.bufferSize_samples = P.bufferSize_bytes / (8*4); %each sample is a 4 x 64-bit column (two audio data samples, sequence and time)

P.rawAudio  = memmapfile(memMapFileName_input, 'Writable', false, 'format',{'double' [4 P.bufferSize_samples] 'audioD'});
%initialization
frame=P.rawAudio.Data(1,1).audioD(:,:); %initialize the first frame
lastFrameStamp = frame(3,end);%ask what is the stamp of the most recent sample written (it's in the last column in the buffer)


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
    
    %grab audio out of the memory mapped file
    frame=P.rawAudio.Data(1,1).audioD(:,:);
    
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
    ylim([-0.15 0.15]);
    subplot(2,1,2);
    plot(audioOut.Data(1,1).audioD(2,:));
    ylim([-0.05 0.05]);
    drawnow;
    
    
%     problem=1; %check to make sure that the thread had to spin.  If it didn't, you probably aren't reading audio fast enough
%     while(P.rawAudio.Data(1,1).audioD(end-1,end)<nextFrameStamp) %check the sequence stamp of the last sample in the frame, wait until it increments before looping
%         %spin until the next frame has been written into the buffer
%         %display(['spinning at ' num2str(P.rawAudio.Data(1,1).audioD(end-1,end))]);
%         problem=0;
%     end
%     
%     
%     %     %some basic error catching
%     if(problem)
%         disp('did not spin, probably dropped audio');
%     end
    
    %check the timing
    frameCounter=frameCounter+1;
    display(frameCounter);
    toc(totalTime);
    elapsed=toc(t);
    if(elapsed>pStruct.frameDuration_seconds)
        disp(['frame number ' num2str(frameCounter) ' ran slow by ' num2str(elapsed - pStruct.frameDuration_seconds) ' seconds.  Samples may have been dropped.']);
    end
    
end