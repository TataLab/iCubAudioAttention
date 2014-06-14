function [ H] = MakeBandPassFilter( )

global P;
nyquist=P.kSampleRate/2;

Fp1=P.Fp1_hz/nyquist;
Fst1=P.Fst1_hz/nyquist;
Fp2=P.Fp2_hz/nyquist;
Fst2=P.Fst2_hz/nyquist;

d=fdesign.bandpass('Fst1,Fp1,Fp2,Fst2,Ast1,Ap,Ast2',Fst1,Fp1,Fst2,Fp2,P.Ast1,P.Abandp,P.Ast2);
H=design(d,'equiripple');
return;

end

