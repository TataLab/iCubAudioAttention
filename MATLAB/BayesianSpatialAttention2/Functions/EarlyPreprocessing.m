 
addpath('/Users/Matthew/Documents/Robotics/AuditoryAttention/ObjectOriented_Kalman/Functions');

%grab raw audio from L/R memory mapped stream and build a bands x beams
%"image" for further processing.  Dump this image into a second memory
%mapped region



%run configuration; this includes opening a connection to memory mapped
%audio data
P=ConfigureParameters;  %use the same configuration as any subsequent audio attention code

%initialization
frame=P.rawAudio.Data(1,1).audioD(:,end-P.sizeFramePlusOverlap+1:end); %initialize the first frame
lastFrameStamp = frame(3,end);%ask what is the stamp of the most recent sample written (it's in the last column in the buffer)

%prime the filterbank
P_inL=zeros(8,P.nBands);
P_inR=zeros(8,P.nBands);

%pre-allocate some variables we need
bandCounter = 0;
frameCounter = 1;
tempBeams=zeros(P.nBands,P.nBeams,P.frameDuration_samples);

egoSpaceImage_front=zeros(P.nBands,P.nBeams); %this holds the initial output of beamforming
egoSpaceImage_back=zeros(P.nBands,P.nBeams-2); %+/-90 are redundant so we won't include them in the back image
egoSpaceImage_lowResolution = zeros(P.nBands,P.nBeams*2-2); %this holds the interpolated and reflected current acoustic scene
egoSpaceImage_highResolution = zeros(P.nBands,P.numSpaceAngles); %this is the final image of the entire scene


done=0;
while(~done)
    
    t=tic;
    
    
    
    %this reads output of the filterbank streamed by AudioCaptureFilterBank_YARP or
    %AudioCaptureFilterBank_PreRecorded
    frame=P.rawAudio.Data(1,1).audioD(:,end-(P.frameDuration_samples+2*P.frameOverlap)+1:end); %note the overlap.  This is so that we can run beamformer and extract exactly frameDuration_samples from each frame by leaving off the tails.  Unnecessary unless you want to concatenate frames for output
    
    frameL=frame(1,:);
    frameR=frame(2,:);
    
    
    %for a controlled test signal
%     lag=10;
%     frameL=rand(1,length(frameL));
%     frameR=circshift(frameL,[0 lag]);
    
    
    %%%%%%Spectral Pre-processing*****
    
    %decompose each channle using a gammatone filterbank
    %and stream out the filtered frames into two seperate files
    [fFrameL,P_outL,~,~,~]=gammatonePhase(frame(1,:),P_inL,P.sampleRate, P.cfs);
    P_inL=P_outL; %save last value to initialize next call of the filter
    [fFrameR,P_outR,~,~,~]=gammatonePhase(frame(2,:),P_inR,P.sampleRate, P.cfs);
    P_inR=P_outR;
    
    
    
    %beamforming
    fFrameL=fFrameL';
    fFrameR=fFrameR';
    
    %sweep a beam to find a first guess at this new object's location
    %this is exactly a bank of delay-and-sum beamformers
    %notice we're doing all frequency bands at once to speed up MATLAB
    
    
    for bandCounter=1:P.nBands
        
        thisBandL=fFrameL(bandCounter,:);
        thisBandR=fFrameR(bandCounter,:);
        
        previousBeamCounter=1;
        beamCounter=1;
        beamStep=1; %you can skip beams if you want!  Interpolation smooths it out pretty well!
        for b=-P.nBeamsPerHemifield:beamStep:P.nBeamsPerHemifield
            tempR=circshift(thisBandR,[0 b]); %shift through each lag
            tempBeam=thisBandL+tempR; %add the shifted right channnel to the unshifted left channel
           
            egoSpaceImage_front(bandCounter,previousBeamCounter:beamCounter)=sqrt(sum(tempBeam.^2)/P.frameDuration_samples);%use RMS to find the sqrt of the average power;  it seems to be fastest if you don't use RMS() and do it inside this loop (!?)
            previousBeamCounter=beamCounter;
            beamCounter=beamCounter+beamStep;
            
        end
        
    end
    
  
    
   
    
    %build a "surround" image that wraps around the entire space
    egoSpaceMap_back=fliplr(egoSpaceImage_front(:,2:end-1)); %this is the "back" image
    
    %concatenate with the "front" image
    egoSpaceImage_lowResolution=[egoSpaceImage_front egoSpaceMap_back];
    
    %rotate so that 0deg is the middle of the array
    egoSpaceImage_lowResolution = circshift(egoSpaceImage_lowResolution,[0 P.nBeamsPerHemifield]);
    
    
    %interpolate and
    %"map" of the egocentric auditory scene
    %interp1 can work on more than one vector at once if they're in columns
    %(this could be optimized a bit more)
    egoSpaceImage_lowResolution=egoSpaceImage_lowResolution';
    
    egoSpaceImage_highResolution=interp1(P.micAngles,egoSpaceImage_lowResolution,P.spaceAngles,'spline'); %the beam distribution isn't linearly arranged around the circle and doesn't sample the space with the same resolution as the angles that point into external space.  Interpolate.
    egoSpaceImage_highResolution=egoSpaceImage_highResolution';
   
%     imagesc(egoSpaceImage_highResolution);
%     drawnow;
    
    %dump it all into the memory mapped image
    P.auditorySpace.Data(1,1).timeStamp(1,1)=frameCounter;  %stamp the first element of the output frame with the frame sequence number
    P.auditorySpace.Data(1,1).timeStamp(1,2)=P.rawAudio.Data(1,1).audioD(4,end); %stamp the second element of the output frame with the original host time of the last sample in the current frame
    P.auditorySpace.Data(1,1).data = egoSpaceImage_highResolution; %dump out the image of the auditory scene
    
    %display(P.rawAudio.Data(1,1).audioD(3,end));
    
    %handle audio packet stuff
    %increment for next frame
    %display(P.rawAudio.Data(1,1).audioD(end-1,end)-lastFrameStamp);
    nextFrameStamp=lastFrameStamp+P.frameDuration_samples; %increment
    lastFrameStamp=nextFrameStamp;
    frameCounter=frameCounter+1;
   
    
    
    toc(t);
    
   %display(['currently ' num2str(P.rawAudio.Data(1,1).audioD(end-1,end)) ' will wait for sample ' num2str(nextFrameStamp)]);
    
    problem=1; %check to make sure that the thread had to spin.  If it didn't, you probably aren't reading audio fast enough
    while(P.rawAudio.Data(1,1).audioD(end-1,end)<nextFrameStamp)
        %spin until the next frame has been written into the buffer
        %display(['spinning at ' num2str(P.rawAudio.Data(1,1).audioD(end-1,end))]);

        problem=0;
    end
    
    
    %     %some basic error catching
    if(problem)
        display('did not spin, probably dropped audio');
    end
    
    
    
    
    
    
end