function [ outFrame ] = EnhanceThis( inFrame, fs )
%ENHANCETHIS Call into the Voicebox toolbox to do some speech enhancement
%http://www.ee.ic.ac.uk/hp/staff/dmb/voicebox/doc/voicebox/specsub.html


outFrame=specsub(inFrame,fs);
outFrame=outFrame';

end

