%read audio stream out of shared memory and use a registered object to
%select a spatial/spectral object

%perform basic stimulus driven orienting to a salient sound
addpath('/Users/Matthew/Documents/Robotics/iCubAudioAttention/MATLAB/SelectiveAttention/Functions_and_Scripts');
display(['Running SelectiveAttention using code at: ' mfilename('fullpath')]);

%perform initialization and setup;  get back a struct with all the settings
%we'll need
P=ConfigureParameters;

%prime the filterbank
P_inL=zeros(8,P.nBands);
P_inR=zeros(8,P.nBands);




frameCounter=1;
nextFrameStamp=P.audioIn.Data(1,1).audioD(end-1,end); %reach into the shared memory and find the last sample written, time stamps are the last row vector, sample stamps are the next row up from the bottom

done=0;
while(~done)
    
    %grab audio for the next frame
    nextFrameStamp=nextFrameStamp+P.frameDuration_samples; %increment what is the sequence stamp we need to wait for
    frame=P.audioIn.Data(1,1).audioD(1:2,end-P.sizeFramePlusOverlap+1:end); %retreive the last "frame size" chunk of audio data
 
%     plot(frame(1,:));
%     ylim([-1.0 1.0]); %scale to fit audio range
%     drawnow;
    
    
    


    
    
    frameCounter=frameCounter+1;
    while(P.audioIn.Data(1,1).audioD(end-1,end) < nextFrameStamp)
        %spin
        %if you don't have to spin here, then the code below is running too
        %slowly
    end
end