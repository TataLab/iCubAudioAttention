
%use a Kalman-like approach to fuse probabalistic maps of auditory space
%across successive head rotations

addpath('./Functions');


%fill a convenient struct with parameters
P=ConfigureParameters;

pause(1);  %a bit of a cludge:  calling ConfigureParameters requires some data to be reinitilized in memory maps.  We need to wait until these get refilled.

%initialize an empty object
%set up an auditory object to keep track of sound sources
O.onsetTime=tic;
O.angle_allo=0;
O.angle_ego=0;
O.salience=0.0;
O.probMap_allo=ones(P.nBands,P.numSpaceAngles); %probabalistic map of where this object is in allocentric space
O.specSignature=ones(P.nBands,P.numSpaceAngles);  %the weights of each band toward the beamformer

%setup for controlling the iCub
rotateIndex=161:-10:81;
numRotations=length(rotateIndex)+1;

%initialize the head position "sensor" (it's not really a sensor but it
%will be eventually!?)
heading_allo=P.headAngle.Data(1,1).headAngle;

%initialize the probability map
probMap_allo=ones(P.nBands,P.numSpaceAngles);
probMapImages=zeros(numRotations,P.nBands,P.numSpaceAngles);

%setup to rotate the head periodically 
lastTurnedTime=tic;
rotationNumber=1;

done=0;
while(~done)
    
    %run looop control
    %we'll always grab the newest frame, but we will necessarily drop
    %frames while the head is rotating...that's not a bad thing...just be
    %aware of it
    
    currentFrameSequenceStamp=P.auditorySpace.Data(1,1).timeStamp(1,1);
    nextFrameSequenceStamp=currentFrameSequenceStamp+1;
    %display(currentFrameSequenceStamp);

    %grab the current image in the shared memory
    currentImage_ego=P.auditorySpace.Data(1,1).data;
    
    %collapse across beams (we'll use this both for spectral salience and
    %to normalize the image
    beamCollapsedImage=sum(currentImage_ego,2);
    
    %normalize the current image within each band to use beam energy as a
    %proxy for probability (note that repmat is faster than bsxfun()...in this case...on
    %matt's laptop...not sure why
    currentImage_egoNorm=currentImage_ego./repmat(beamCollapsedImage,[1 P.numSpaceAngles]);
    
    
    %align the image with allocentric space by rotating opposite the head
    %direction
    currentImage_allo=circshift(currentImage_egoNorm,[0 heading_allo]);

    
    
    %multiply the prior with the new evidence to compute a posterior
    %probability
    probMap_allo = probMap_allo .* currentImage_allo;
    
   
    
    % renormalize the posterior probability
    probMap_allo = probMap_allo ./ repmat(sum(probMap_allo,2),[1 P.numSpaceAngles]);
    
    
    %O.probMap_allo=probMap_allo;  %to do object oriented stuff eventually
    
    %just compute the image collapsed across frequency bands
    bandCollapsedImage_allo=sum(probMap_allo,1);
    currentBandCollapsedImage_allo=sum(currentImage_allo,1);
    deltaImage=currentBandCollapsedImage_allo-bandCollapsedImage_allo;
    
    
    surf(P.spaceAngles*180/pi,P.cfs,probMap_allo);
    view([-10,50]);
    drawnow;
    
    
    
    %every second or so, turn the head
    if(toc(lastTurnedTime)>2.25)
        
        
        %surf(P.spaceAngles*180/pi,P.cfs,probMap_allo);
        %drawnow;
        
        display(['writing image ' num2str(rotationNumber) ' for angle ' num2str(heading_allo)]);
        probMapImages(rotationNumber,:,:)=probMap_allo;
        
        nextHeadingIndex=rotateIndex(rotationNumber);
        rotationNumber=rotationNumber+1;
        if (P.useYARP == 1)
            
            nextHeading_allo=round(P.spaceAngles(nextHeadingIndex)*180/pi);     %figure out which allocentric angle we should look at 
            
            nextHeading_ego=nextHeading_allo-heading_allo; %what is that allocentric angle in egocentric coordinates
            
            %tell iCub to turn...
            
            
            pause(1); %give it a moment
            
            heading_allo=P.headAngle.Data(1,1).headAngle; %read the new head angle
        
            
            
        end
        
        
        lastTurnedTime=tic;
        
    end
    
    
    didSpin=0;
    while(P.auditorySpace.Data(1,1).timeStamp(1,1)<nextFrameSequenceStamp)
        %spin
        didSpin=1;
    end
    
    if(~didSpin)
        display('Timing problem. You probably dropped data.');
    end
    
end

display('quitting');