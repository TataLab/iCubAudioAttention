function [ frame] = GetNextFrame( P , requestedStamp)
%GetNextFrame is given a memorymapped address and a stamp to wait for.  It
%spins until the last sample written into the buffer by audioCapture is
%equal or greater than the requested stamp of the last pixel in the frame
%you want to retrieve.  If it doesn't have to spin it returns 0 to indicate
%that you're probably underrunning.

    
    while(P.audioIn.Data(1,1).audioD(3,end)<requestedStamp)
        %spin
    end

    frame=P.audioIn.Data(1,1).audioD(:,end-P.sizeFramePlusOverlap+1:end); %return the last "frame size" chunk of data
 
end

