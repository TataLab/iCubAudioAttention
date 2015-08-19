
%capture audio from a YARP stream using mex code to open and close YARP
%ports and grab sound class



audioAttentionRoot='/Users/Matthew/Documents/Robotics/iCubAudioAttention'; %point to the root of the repository

%set up some parameters of the audio grabber.  Careful: some of this is
%hard coded into the mex function.
sampleRate=48000;
sampleDuration_seconds= 1/sampleRate;

frameDuration_samples = 4096;  %must match the settings in the mex function. should be integer multiple of the page size to keep memory access fast
frameDuration_seconds= frameDuration_samples * sampleDuration_seconds;

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

%pre allocate variables to hold frame count and sample time stamps
frameCount=1;
frameCountVector=ones(1,frameDuration_samples);
timeStamp=1;
timeStampVector=zeros(1,frameDuration_samples);


done=0;
while(~done) %loop continuously
   
    t=tic;
    j=batch('audioCapture');
    
    while(toc(t)<frameDuration_seconds);
    end
    
    display('job done');
    %[frameL,frameR,stamps]=audioCapture;  %call into YARP through the mex code
    
%     frameCount=stamps(1); %parse out the frame count and the time stamp
%     timeStamp=stamps(2);
% 
%     
%     %figure out where to write the current frame into the ring buffer
%     memMapWriteIndex=(mod(frameCount,numMemMapFrames)*frameDuration_samples+1); %keep track of which frame you're writting into.  Using modulus to "wrap" 
%     
%     %write the audio data of the current frame
%     audioOut.Data(1,1).audioD(1,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1) = frameL;
%     audioOut.Data(1,1).audioD(2,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1) = frameR;
%     
% %     plot(audioOut.Data(1,1).audioD(1,:));
% %     ylim([-1.0 1.0]);
% %     drawnow;
%     
%     %compute the sample count stamps and the time stamps
%     frameCountVector=frameCount:frameCount+frameDuration_samples-1;
%     timeStampVector=timeStamp:timeStamp+(frameDuration_samples-1)*sampleDuration_seconds;
% 
%     audioOut.Data(1,1).audioD(3,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1)=frameCountVector;
%     audioOut.Data(1,1).audioD(4,memMapWriteIndex:memMapWriteIndex+frameDuration_samples-1)=timeStampVector;

end
