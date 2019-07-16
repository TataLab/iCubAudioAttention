function [ angle_deg ] = ConvertLagToAngle( Lsamples )
%takes a vector of peaks in the cross-correlation (in lags) and computes
%their angles.  



%pull constants from the parameter file P
global P;

%convert the lag in samples to lag in seconds
lag_seconds=Lsamples./P.sampleRate;

%return the angles
tempAngle=real(asin(P.c.*lag_seconds.*(1/P.D)));
angle_deg=(tempAngle.*180.0)./pi;


end

