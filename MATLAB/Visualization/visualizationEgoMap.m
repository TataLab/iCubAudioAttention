%this memory mapped file picks up the long Bayesian map
memMapFileName='/tmp/audioMapEgo.tmp';
f=dir(memMapFileName);
audioInv2  = memmapfile(memMapFileName, 'Writable', false, 'format',{'double' [360 128] 'longMap'});

while(1)
    surf((audioInv2.Data(1,1).longMap));
    drawnow;
end

