
priors=gausswin(P.nBeams);
priors=priors';
priors=repmat(priors,[P.nBands 1]);


evidenceBeam=12;

posteriors=zeros(P.nBands,P.nBeams);

for j=1:200
   
    %for each band x true arrival angle
    
    posteriors=priors.*squeeze(P.evidenceRatios(:,:,evidenceBeam));
    
    priors=posteriors;
    
    surf(posteriors);
    drawnow;

    
end