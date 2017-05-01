% Copyright (C) 2015 Matthew Tata
% email: matthew.tata@uleth.ca
%
% Permission is granted to copy, distribute, and/or modify this program
% under the terms of the GNU General Public License, version 2 or any
% later version published by the Free Software Foundation.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
% Public License for more details


% a simple utility to test if your audio stream is coming through the
% shared memory


frameSize=4096;  
sampleRate=48000;

%%%%%%
%parameters for interacting with memory mapped audio
%%%%%
%memMapFileName='/Users/Matthew/Documents/Robotics/iCubAudioAttention/data/AudioMemMap.tmp';
memMapFileName='/tmp/AudioMemMap.tmp';
f=dir(memMapFileName);
bufferSize_bytes = f.bytes; %the  buffer size is determined by AudioCapture_YARP or AudioCapture_PreRecorded.  Frames on that side are hard coded to be 4096 samples.  There are 4 rows by 4096 doubles x some number of frames in the  buffer.
bufferSize_samples = bufferSize_bytes / (8*4); %each sample is a 4 x 64-bit column (two audio data samples, sequence and time)
audioIn  = memmapfile(memMapFileName, 'Writable', false, 'format',{'double' [4 bufferSize_samples] 'audioD'});

%how much audio do you want to record?
howLong_seconds=10;  
howLong_samples=howLong_seconds*sampleRate;
howLong_frames=floor(howLong_samples/frameSize);

recordedAudio=zeros(2,howLong_samples);

previousFrameStamp=0;
writeIndex=1;
t=tic;
for i=1:howLong_frames
    measuredFrameTime=tic;
    %map the audio buffer into shared memory
    frame=audioIn.Data(1,1).audioD(:,end-frameSize+1:end); %get the most recent frame
    thisFrameStamp = frame(3,end);%ask what is the stamp of the most recent sample written (it's in the last column in the buffer)
        
    previousFrameStamp=thisFrameStamp;
    
    recordedAudio(:,writeIndex:writeIndex+frameSize-1)=frame(1:2,:);
    writeIndex=writeIndex+frameSize;
    
    wellSpun=0;
    %wait until AudioCapture_YARP writes new data into the shared memory
    while(audioIn.Data(1,1).audioD(3,end)<thisFrameStamp+frameSize)
%         %spin
%         display('waiting for the next frame');
          wellSpun=1;
    end

    
    if(~wellSpun)
        display('timing problems, you may have dropped audio data');
    end
    
    
    
    plot(frame(1,:));
    drawnow;
    %display(['that frame took ' num2str(toc(measuredFrameTime)) ' seconds.  It should have taken ' num2str(frameSize/sampleRate) ' seconds.']);
end
display(['recording that audio took ' num2str(toc(t)) ' seconds.  It should have taken ' num2str(howLong_samples/sampleRate) ' seconds.']);

%scale the audio so it runs between -1 and 1
recordedAudio=recordedAudio./max(max(abs(recordedAudio)));

audiowrite('./test.wav',recordedAudio',sampleRate);
soundsc (recordedAudio(1,:),sampleRate);