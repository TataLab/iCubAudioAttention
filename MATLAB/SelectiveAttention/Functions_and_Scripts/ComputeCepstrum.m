function [ c ] = ComputeCepstrum( sig, minF, maxF, fs)
%computes cepstrum within a frequency band


%compute the cepstrum
cep=ifft(log(abs(fft(sig))));


%extract the part requested
minCep=ceil(fs/minF);
maxCep=ceil(fs/maxF);



c=cep(maxCep:minCep);




end

