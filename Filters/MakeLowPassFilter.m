function [ H] = MakeLowPassFilter( )
%MAKELOWPASSFILTER

global P;
nyquist=P.sampleRate/2;

Fp=P.Fp_hz/nyquist;
Fst=P.Fst_hz/nyquist;

d=fdesign.lowpass('Fp,Fst,Ap,Ast',Fp,Fst,P.Ap,P.Ast);
H=design(d,'butter');
return;

end

