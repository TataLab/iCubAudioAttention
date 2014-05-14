function [theta,gcc_inv_shifted,trigger] = SmartComputeAngle(S,rate,method,leftHarm,rightHarm)
% takes S a stereo sound vector (chans x samples) 
%also take a pair of vectors containing frequencies to boost in the fft

c=340.29;%define speed of sound in m/s
D=0.1; %define distance between microphones in m



if(method==1);
    %compute GCC-PHAT
    
    %define the width of the window over which to inspect lags (we're only
    %interested in lags of a few 10's- the rest would be reverberation
    
    window=50; %should be divisible by 2...this is how many samples to look on each side of the midline for a peak...this should be related to the distance between the microphones
    
    
    %xplode the array to make this more clear
    s1=S(1,:);
    s2=S(2,:);
    
    %compute fft of inputs
    f1=fft(s1);
    f2=fft(s2);


   % %take the complex values apart

   leftAbsF=abs(f1);
   leftAngF=angle(f1);
   rightAbsF=abs(f2);
   rightAngF=angle(f2);
   
   %overlay the vectors of coefficients from the target harmonics and apply
   % the gain...note that we're given gains not normalized coeficients
   indices=leftHarm(2,:); %put all the indices in a row vector to make this code more clear
   leftAbsF(indices)=leftAbsF(indices).*leftHarm(1,:);
   rightAbsF(indices)=rightAbsF(indices).*rightHarm(1,:);
   
   %don't forget the right side of the fft
   leftAbsF(end-indices+1)=leftAbsF(end-indices+1).*leftHarm(1,:);
   rightAbsF(end-indices+1)=rightAbsF(end-indices+1).*rightHarm(1,:);
   
 
   
   %put the complex values back together

   f1=leftAbsF.*exp(1i*leftAngF);
   f2=rightAbsF.*exp(1i*rightAngF);

   %compute mean magnitude and send it back to use as a trigger
   twoChanMagnitude=(abs(f1)+abs(f2)) / 2;
   trigger=mean(twoChanMagnitude);
    
    %compute the weighted generalized correlation coefficient
    gcc_phat=(f1.*conj(f2))./abs(f1.*conj(f2));
    
    %compute the inverse transform
    gcc_inv = ifft(gcc_phat);
    
    %make it real
    gcc_inv_real=real(gcc_inv);
    
    %shift it to zero the 0th lag
    gcc_inv_shifted=fftshift(gcc_inv_real);
    
    
    
    
    %find the 0th lag
    middle=floor(length(gcc_inv_shifted)/2);
    %plot(gcc_inv_shifted(middle-200:middle+200));
    hold on;
    
    %find the index with the biggest peak
    [~,tempLag] = max(gcc_inv_shifted(middle-window/2:middle+window/2)); %only the first hundred or so lags are relevant
    lag=tempLag-window/2;
    
    %convert the lag in samples to lag in seconds
    lag_seconds=lag/rate;
   
    theta=real(asin(c*lag_seconds*(1/D)));
    
end

return


end

