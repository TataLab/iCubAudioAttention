%scratch

sessionDuration_samples = 200*44100;

audioD_memFile=memmapfile('audioD.dat','format',{'single' [2 sessionDuration_samples] 'd'}, 'writable',false);



done=0;

while(done<20)
    
    plot(audioD_memFile.data(1,1).d(1,:));
    pause(1); %every second-ish
    
    done=done+1;
    
    
end

wavwrite(audioD_memFile.data(1,1).d,44100,16,'recorded_from_memmap.wav');