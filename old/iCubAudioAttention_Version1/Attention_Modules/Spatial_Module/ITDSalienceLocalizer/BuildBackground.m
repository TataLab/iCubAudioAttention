function [lagBackground,lagStdDev] = BuildBackground(nFrames,startIndex,startTime)
%BUILDBACKGROUND takes a sequence of nFrames, computes their GCC-PHAT lag
%vectors, makes an average and returns the average and std. dev.

global P;
lagArray=zeros(nFrames,P.frameDuration_samples-0*2);

display('Collecting background audio');

%grab frames
for i=1:nFrames
    display(['computing GCC for frame' num2str(i)]);
    [temp,startIndex, startTime]=GetNextFrame(startIndex,startTime);
    
    
    f1=fft(temp(1,:));
    f2=fft(temp(2,:));
%     f1(1:1000)=[];
%     f2(1:1000)=[];
%     f1(end-999:end)=[];
%     f2(end-999:end)=[];
    
    %compute the weighted generalized correlation coefficient
    gcc_phat=(f1.*conj(f2))./abs(f1.*conj(f2));
    
    %compute the inverse transform
    gcc_inv = ifft(gcc_phat);
    
    %make it real
    gcc_inv_real=real(gcc_inv);
    
    %shift it to zero the 0th lag
    lagArray(i,:)=fftshift(gcc_inv_real);
    
end


lagBackground=mean(lagArray);
lagStdDev=std(lagArray);

display('Background is computed');
if(isnan(lagBackground(1)))
    display('warning, you have a NaN problem');
end

end

