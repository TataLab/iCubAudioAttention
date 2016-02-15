function [ noiseFloor ] = BuildNoiseFloor(P, fileName )
%BUILDNOISEFLOOR 
%
%load a pre-recorded audio file and steer a beam through the frequency x
%beam space to estimate probability that any given steering angle will be
%identified as having a sound source

display('estimating background noise beam pattern');

[frame,fs]=audioread(fileName);

frame=frame'; %transpose of confusion to get audio running in row vectors as god intended

frame(2,:)=circshift(frame(2,:),[0 10]);

%throw an error if the samplerate of the noise file doesn't match the
%expected sample rate
if(~isequal(fs,P.sampleRate))
    display('noise floor file does not match expected sample rate. Check the config parameters.');
end


%%%%%%Spectral Pre-processing*****

%prime the filterbank
P_inL=zeros(8,P.nBands);
P_inR=zeros(8,P.nBands);

%decompose each channle using a gammatone filterbank
%and stream out the filtered frames into two seperate files
[fFrameL,~,~,~,~]=gammatonePhase(frame(1,:),P_inL,P.sampleRate, P.cfs);
[fFrameR,~,~,~,~]=gammatonePhase(frame(2,:),P_inR,P.sampleRate, P.cfs);


%twist around to make audio signals into row vectors for beamforming
fFrameL=fFrameL';
fFrameR=fFrameR';



%get some variables ready for the output of the beamformer stage
thisFrameImage=zeros(P.nBands,P.nBeams); %it's only nBeamsPerHemi *2 long because we loose half the samples off either end (theoretically they're the last samples of the previous frame and the first samples of the frame that hasn't happend yet)


%for each frequency in the filterbank
for bandCounter=1:P.nBands
    
    thisBandL=fFrameL(bandCounter,:);
    thisBandR=fFrameR(bandCounter,:);
    
    %for each steering angle
    beamCounter=1;
    for b=-P.nBeamsPerHemifield:P.nBeamsPerHemifield
        tempR=circshift(thisBandR,[0 b]); %shift through each lag
        thisBeam=thisBandL+tempR; %add the shifted right channnel to the unshifted left channel and get the rms energy of the beam
        thisBeamRMS=rms(thisBeam);
        thisFrameImage(bandCounter,beamCounter,:)=thisBeamRMS; %accumulate
        
        beamCounter=beamCounter+1;
        
    end
    
    %normalize so that the values are between 0 and 1
    thisFrameImage(bandCounter,:)=thisFrameImage(bandCounter,:)./sum(thisFrameImage(bandCounter,:));
    
end

%upsample and interpolate the measured noise floor to match the resolution of the space angles 
thisFrameImage=thisFrameImage'; %interp will want it to be anglesxbands
thisFrameImage=interp1(P.angles,thisFrameImage,-pi/2:P.radialResolution_radians:pi/2); %interpolate the smaller number of beams onto the (probably) larger number of angles in real space.  Divide by two because we're still only working with the front half.
thisFrameImage=thisFrameImage'; %flip it back to bandsxangles
reflectedFrameImage=fliplr(thisFrameImage);
noiseFloor=[thisFrameImage reflectedFrameImage(:,2:end-1)];

display('done');

end

