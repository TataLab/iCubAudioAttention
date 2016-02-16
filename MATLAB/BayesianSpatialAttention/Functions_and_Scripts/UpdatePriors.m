function [ posteriors ] = UpdatePriors( O, evidenceBeam, currentMicHeading_degrees, P )
%pass the object with its vector of priors in external space and the
%current angle we think the sound is comming from, the heading of the robot and the parameters of the array

%update priors will rotate the priors to align with mic space and
%read out the priors that correspond to the beams
%then it will use these to update the priors using Bayes and return
%the object with updated priors


%P(A|B)=P(A)*P(B|A)/P(B)

%start by circshifting the vector of priors so that we can look up a prior
%in mic array coordinates
currentMicHeading_index=find(currentMicHeading_degrees>=P.spaceAngles,1,'first'); %find the index in space angles that corresponds to the mic heading
%evidence beam is in mic beams with P.nBeams resolution; we want it to be
%in space with P.numSpaceAngles resolution

[~,evidenceBeamAngle]=find(P.spaceAngles>=P.angles(evidenceBeam),1,'first'); %find the index of the angle in the high-resolution radial space that corresponds to the evidence beam



micPriors=circshift(O.radialPriors,[0 -currentMicHeading_index]); %think about the sign of the head very very carefully
%display(['current evidence beam is ' num2str(evidenceBeam)]);

%expand the priors to span all frequencies
micPriors=repmat(micPriors,[P.nBands 1]);

%now we can use Bayes to update these priors
posteriors=micPriors.*squeeze(P.evidenceRatios(:,:,evidenceBeamAngle));


%collapse across the bands to arrive at a single best-guess of the new
%probabilities
posteriors=sum(posteriors,1);

%normalize posteriors so they add to 1
posteriors=posteriors./sum(posteriors);

%now rotate them back into real-world space
posteriors=circshift(posteriors,[0 currentMicHeading_index]); %mind the sign, be sure you're rotating the right direction!



end

