kFrameSize_samples=1024;
kNumFrames= 2000;
kSampleRate=48000;
kFrameDuration_seconds=kFrameSize_samples/kSampleRate;


kFileName='junk.dat';
f=dir(kFileName);
fileSize_bytes=f.bytes;
kBytesPerSample=2; %16bit 
kNumChannels=2;

kDuration_samples=fileSize_bytes/kBytesPerSample/kNumChannels;
kDuration_seconds=kDuration_samples/kSampleRate;

%memory map audio back out of the file
mIn=memmapfile('junk.dat','format',{'int16' [2 kDuration_samples] 'd'});

%make a vector to preload into the buffer
pre=zeros(2,kFrameSize_samples*2);

%set up audio output
InitializePsychSound(1);
paoutput = PsychPortAudio('Open', [], 1, 2, 48000, 2,1024);


%get ready to stream
readIndex=1;
audiodata=mIn.Data(1,1).d(:,readIndex:readIndex+kFrameSize_samples-1); %the first frame is ready


%start playback
PsychPortAudio('FillBuffer', paoutput, pre);
t=tic;
playbackstart = PsychPortAudio('Start', paoutput, 0,0,1);

%loop playback
for i=1:kNumFrames-1
  
    t=tic;
    while (toc(t)<.02)
    end
    
    %scale the audio
    audiodata=double(audiodata);
    for j=1:2
        audiodata(j,:)=audiodata(j,:)./max(abs(audiodata(j,:)));
    end
    
    %now fill the buffer with the next frame
    [underflow,~,~]=PsychPortAudio('FillBuffer', paoutput, audiodata,1);

    %prepare next frame
    readIndex=readIndex+kFrameSize_samples;
    audiodata=mIn.Data(1,1).d(:,readIndex:readIndex+kFrameSize_samples-1); 

    
    %Keep looping as fast as possible.  The driver will take care of
    %buffering the data.
   
    
end

display('done writing audio');


%close down neatly
PsychPortAudio('Stop', paoutput, 1);
PsychPortAudio('Close');

display('done');
