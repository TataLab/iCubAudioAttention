%test writing data using memory mapping

%memory map this file
kFileName = './test.dat';
kSamplesPerFrame=4096;
kFrameSize=3*kSamplesPerFrame; %how much data to write each time:  left, right, time
kNumDataWrites = 14400;
kSampleRate=16000;

%....step 1:  set up a file with the correct number of bytes
kFileSize_samples = kFrameSize * kNumDataWrites;

emptyData = int32(zeros(1,kFileSize_samples)); %pre build an array of zeros to write into the file

t=linspace(0,2*pi,kSamplesPerFrame);
s=sin(t); %a 1 hz signal

fileID=fopen(kFileName,'w');
fwrite(fileID,emptyData,'single');
fclose(fileID);
%............



%step 2:  memory map the file we just created and write data into it
M=memmapfile(kFileName,'format','int32', 'writable',true);



index=1;
for i = 1 : kNumDataWrites
    
    display(['writing frame number ' int2str(i)]);
    
    %build a frame with 1s for audio data and sequential numbers for the time
    %stamp
    frame=zeros(kFrameSize,1);
    frameIndex=3;
    for j=1:kSamplesPerFrame
        frame(frameIndex,1)=(i-1)*kSamplesPerFrame+j; %the stamp
        frameIndex=frameIndex+3;
    end
    
    
    %write some samples
    M.Data(index:index+kFrameSize-1,1)=frame;
    
    pause(0.256);
    
    index = index+kFrameSize; %increment to the next frame
    
end


