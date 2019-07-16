kFrameSize_samples=4096;
kNumFrames= 200;
kSampleRate=48000;
kFrameDuration_seconds=kFrameSize_samples/kSampleRate;
kDuration_seconds=kFrameDuration_seconds*kNumFrames;
kDuration_samples=kDuration_seconds*kSampleRate;

%make a big empty audio file and memory map it
empty=int16(zeros(2,kDuration_samples));

fid=fopen('./junk.dat','w');
fwrite(fid,empty,'int16');
fclose(fid);

mIn=memmapfile('junk.dat','format',{'int16' [2 kDuration_samples] 'd'},'writable',true);

%build audio
t=linspace(0,2*pi,kSampleRate);
s=sin(100*t);
signal=repmat(s,[2,ceil(kDuration_seconds)]); 

%write it to the file every one second
writeIndex=1;
for i=1:kNumFrames
    
    mIn.Data(1,1).d(:,writeIndex:writeIndex+kFrameSize_samples-1)=signal(:,writeIndex:writeIndex+kFrameSize_samples-1);
    writeIndex=writeIndex+kFrameSize_samples;
    
    t=tic;
    while(toc(t)<kFrameDuration_seconds)
        %wait
    end
    
    
    
end
