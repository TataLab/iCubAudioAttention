kFrameSize_samples=1024;
kFixedLag_samples=0;
kNumFrames= 2000;
kSampleRate=48000;
kFrameDuration_seconds=kFrameSize_samples/kSampleRate;


kFileName='/tmp/AudioDataDump.dat';
f=dir(kFileName);
fileSize_bytes=f.bytes;
kBytesPerSample=2; %16bit 
kNumChannels=2;

kDuration_samples=fileSize_bytes/kBytesPerSample/kNumChannels;
kDuration_seconds=kDuration_samples/kSampleRate;
% 
% %memory map audio back out of the file
% mIn=memmapfile(kFileName,'format',{'int16' [2 kDuration_samples] 'd'});

[mIn,sampleD]=OpenAudioInputData;
tempMostRecentSample=sampleD.Data(1,1).f;
readIndex = tempMostRecentSample - (kFrameSize_samples - 1) - kFixedLag_samples;  %set the read index to the first sample in the next frame to be read...here we need to be careful to lag the audio data dump by some small by non-trivial time
currentFrameTime = tic;  %grab the current time


%make a vector to preload into the buffer
pre=zeros(2,kFrameSize_samples*2);

%set up audio output
InitializePsychSound(1);
paoutput = PsychPortAudio('Open', [], 1, 2, 48000, 2,1024);


%get ready to stream
audiodata=mIn.Data(1,1).d(:,readIndex:readIndex+kFrameSize_samples-1); %the first frame is ready

scaleFactor=1/2^15;
%scale the audio
audiodata=double(audiodata).*scaleFactor;


%start playback
PsychPortAudio('FillBuffer', paoutput, pre);
playbackstart = PsychPortAudio('Start', paoutput, 0,0,1);

%loop playback
for i=1:kNumFrames-1

    %first immediately fill the buffer with the next frame
    [underflow,~,~]=PsychPortAudio('FillBuffer', paoutput, audiodata,1);
    
    %prepare next frame
    readIndex=readIndex+kFrameSize_samples;
    audiodata=mIn.Data(1,1).d(:,readIndex:readIndex+kFrameSize_samples-1); 

    %scale the audio
   audiodata=double(audiodata).*scaleFactor;

    
   %keep looping fast, the sound driver will take care of the rest
    
end

display('done writing audio');


%close down neatly
PsychPortAudio('Stop', paoutput, 1);
PsychPortAudio('Close');

display('done');
