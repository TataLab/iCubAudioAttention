function [ evidenceRatios ] = ComputeEvidenceRatio( P )
%COMPUTEEVIDENCERATIO take the sensitivity beam patterns and the noise
%floor and compute the relative evidence that a talker is at a steering
%angle given a true arrival angle

%this is P(B|A)/P(B)
%we can pre-compute this

display('pre-computing evidence ratios');

evidenceRatios=zeros(P.numSpaceAngles,P.numSpaceAngles);


    %for each steering angle B
    for steeringAngleIndex=1:P.numSpaceAngles
        
        
        
        %for each true arrival angle
        thisArrivalAngleVector=squeeze(P.beamPattern(:,steeringAngleIndex));
        thisEvidenceRatioVector=(thisArrivalAngleVector')./P.noiseFloor;  %compute all the ratios at once
        thisEvidenceRatioVector=thisEvidenceRatioVector./sum(thisEvidenceRatioVector);
        evidenceRatios(:,steeringAngleIndex)=thisEvidenceRatioVector; %stash the ratio into the return matrix
        
    end



display('done');

end

