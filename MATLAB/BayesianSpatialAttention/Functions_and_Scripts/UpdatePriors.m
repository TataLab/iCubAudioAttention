function [ posteriors_micAligned,posteriors_spaceAligned ] = UpdatePriors( O, evidenceBeam, currentMicHeading_index, P , frameSalience)
%pass the object with its vector of priors in external space and the
%current angle we think the sound is comming from, the heading of the robot and the parameters of the array

%update priors will rotate the priors to align with mic space and
%read out the priors that correspond to the beams
%then it will use these to update the priors using Bayes and return
%the object with updated priors


%P(A|B)=P(A)*P(B|A)/P(B)


%evidence beam is in mic beams with P.nBeams resolution; we want it to be
%in space with P.numSpaceAngles resolution

%find the index of the angle in the high-resolution radial space that corresponds to the evidence beam
tempDif=P.spaceAngles-P.angles(evidenceBeam);
[~,evidenceBeamAngleIndex]=min(abs(tempDif));


%now we can use Bayes to update these priors
PBgivenA=squeeze(P.evidenceRatios(:,evidenceBeamAngleIndex));

% plot(ratios);
% drawnow;

tempPosteriors=O.radialPriors_micAligned.*PBgivenA'; %traspose


%normalize posteriors so they add to 1
tempPosteriors=tempPosteriors./sum(tempPosteriors);

posteriors_micAligned=O.radialPriors_micAligned +  P.learnRate * (tempPosteriors * frameSalience);

%now rotate them back into real-world space
posteriors_spaceAligned=circshift(posteriors_micAligned,[0,currentMicHeading_index]); %mind the sign, be sure you're rotating the right direction!



end

