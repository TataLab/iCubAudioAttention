%this memory mapped raw Audio
memMapFileName='/tmp/rawAudio.tmp';
f=dir(memMapFileName);
audioInv2  = memmapfile(memMapFileName, 'Writable', false, 'format',{'double' [2 4800] 'sound'});

while(1)
    plot(linspace(0,1,4800),(audioInv2.Data(1,1).sound(1,:)));
    drawnow;
end
