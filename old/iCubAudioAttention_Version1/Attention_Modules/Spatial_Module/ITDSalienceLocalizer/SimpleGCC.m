function [ lag ] = SimpleGCC( S )
%SIMPLEGCC simply compute GCC_PHAT lag

ITDWindow=62;


%compute lags
s1=S(1,:);
s2=S(2,:);

f1=fft(s1);
f2=fft(s2);

%compute the weighted generalized correlation coefficient
gcc_phat=(f1.*conj(f2))./abs(f1.*conj(f2));

%compute the inverse transform
gcc_inv = ifft(gcc_phat);

%make it real
gcc_inv_real=real(gcc_inv);

%shift it to zero the 0th lag
gcc_inv_shifted=fftshift(gcc_inv_real);

%find the 0th lag
middle=floor(length(gcc_inv_shifted)/2)+2;

thisWindow=gcc_inv_shifted(middle-ITDWindow/2:middle+ITDWindow/2-1);

[~,tempLag] = max(thisWindow); %only the window  lags are relevant
lag=tempLag-ITDWindow/2;

end

