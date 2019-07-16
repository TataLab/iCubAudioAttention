

%memory map this file
kInFileName = '/Users/Matthew/Documents/MATLAB/Icub Interface/Memory/SensoryMemory.dat';
%kOutFileName = '/Users/Matthew/Documents/MATLAB/Icub Interface/Memory/SpatialAttentiveMemory.dat';

kSampleRate = 16000;
kSamplesPerFrame=4096;
kFrameSize=3*kSamplesPerFrame; %how much data to read each time
kFrameRate=kSampleRate/kSamplesPerFrame; %Hz
kNumDataReads=7000;
kFileSize_samples=kSamplesPerFrame*kNumDataReads;
kWindowSize_samples=16000;

%create a file to output the processed audio signal
%CreateSpatialAudioFile(kOutFileName,kFileSize_samples);

%the input audio
mIn=memmapfile(kInFileName,'format',{'int32' [3 kFileSize_samples] 'd'}, 'writable',false);
%the output audio
%mOut=memmapfile(kOutFileName,'format',{'single' [2 kFileSize_samples] 'd'}, 'writable',false);

%audioData=zeros(3,kSamplesPerFrame*kNumDataReads);
%looooooop to acquire audio
done=0;

%find most recent sample written
tempD=mIn.Data(1,1).d(3,:); %get the audio out of the struct
%sampleIndex=find(tempD==0,1,'first')-kSamplesPerFrame-1;
sampleIndex=1;
secondCounter=tic;
while(~done)
    t=tic;
    currentFrame=mIn.Data(1,1).d(:,sampleIndex:sampleIndex+kSamplesPerFrame-1); %get the audio out of the struct 
    
    %here we could do ... attention!
    %display(['writing data at index ' int2str(sampleIndex)]);
    %MOut.Data(1,1).d(:,sampleIndex:sampleIndex+kSamplesPerFrame-1)=currentFrame;
   
    %just to check:  every 1 second plot 1 second worth of audio
    while(toc(t)<(1/kFrameRate))
        %block for the rest of the frame duration
        
        if(toc(secondCounter)>1.0)
            dataWindow=MOut.Data(1,1).d(:,sampleIndex-kWindowSize);
            plot(dataWindow(1,:)); %plot the current audio frame every second
            drawnow;
            secondCounter=tic;
        end
        
        sampleIndex=sampleIndex+kSamplesPerFrame;
    end  
 
end