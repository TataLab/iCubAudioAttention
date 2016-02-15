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
currentMicHeading_index=ceil(currentMicHeading_degrees);  %this is a cludge, it hardcodes a requirement that the resolution of radialPriors be 1 degree
micPriors=circshift(O.radialPriors,[0 -currentMicHeading_index]); %think about the sign of the head very very carefully


%now we can use Bayes to update these priors
posteriors=micPriors.*squeeze(P.evidenceRatios(:,:,evidenceBeam));

%now rotate them back into real-world space
posteriors=circshift(posteriors,[0 currentMicHeading_index]); %mind the sign, be sure you're rotating the right direction!



end

