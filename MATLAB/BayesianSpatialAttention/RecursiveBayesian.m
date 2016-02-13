
function [Po]=RecursiveBayesian(Po,numBands)

for ff=1:numBands % number of band
   
    
    Pr=Po; %store the posterior to the prior. Pr=P(A)
    P_AB=0*Pr;%zeros(zis of Pr);
    
    % look at each location, assume that the given location is
    % where sound is, and get the likelihood of the data x(:,n)
    
    
    
    
    P_BA3= pdfv2(ff, round(DOA(ff)),round((500-1)*(Sb )/180+1));%Sb(j)+1
    P_BA=reshape(P_BA3,[1,181]); %liklihood
    P_AB=P_BA .* Pr(1,:); % Combine this likelihood with the prior
    % TODO add P(B)
    Po=P_AB/sum(sum(P_AB)); %normalize this distribution to make it a proper probability distribution.
    
    
    
end



end% function