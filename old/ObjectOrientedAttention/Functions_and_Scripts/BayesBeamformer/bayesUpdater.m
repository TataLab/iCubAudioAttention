function [ Po ] = bayesUpdater( A,Po )
%BAYESUPDATER Summary of this function goes here
%   Detailed explanation goes here

for ff=1:10 % number of band
        
    
    
    Pr=Po; %store the posterior to the prior.
    m=0*Pr;
    %likelihood
    
    
    
    x(2,k)=A;% by commenting this line program shift to nrandom created data set
    
    % Two dimentional polar space (r,theta)
    for i=1:nn
        for j=1:mm-1
            me=[Sa(i);Sb(j)];
            
            m(i,j)=   pdfv2(ff, round(x(2,k)),round((500-1)*(me(2,1) )/180+1));%Sb(j)+1
            
            m(i,j) = m(i,j) * Pr(i,j); % Combine this likelihood with the prior
            0;
        end
    end
    Po=m/sum(sum(m)); %normalize this distribution to make it a proper probability distribution.
    
    
    
    
    
end
end

