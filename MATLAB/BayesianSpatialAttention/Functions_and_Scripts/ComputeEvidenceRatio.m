function [ evidenceRatios ] = ComputeEvidenceRatio( P )
%COMPUTEEVIDENCERATIO take the sensitivity beam patterns and the noise
%floor and compute the relative evidence that a talker is at a steering
%angle given a true arrival angle

%this is P(B|A)/P(B)
%we can pre-compute this

display('pre-computing evidence ratios');

evidenceRatios=zeros(P.nBands,P.numSpaceAngles,P.numSpaceAngles);

%for each frequency band
for bandIndex=1:P.nBands
    
    %for each steering angle B
    for steeringAngleIndex=1:P.numSpaceAngles
        
        B=P.noiseFloor(bandIndex,steeringAngleIndex); %pull out the P(B) due to background noise at this steering angle
        
        %for each true arrival angle
        thisArrivalAngleVector=P.beamPattern(bandIndex,:,steeringAngleIndex);
        thisEvidenceRatioVector=(thisArrivalAngleVector)./B;  %compute all the ratios at once
        evidenceRatios(bandIndex,:,steeringAngleIndex)=thisEvidenceRatioVector; %stash the ratio into the return matrix
           
    end
end





display('done');

end

