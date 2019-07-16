function [ outFrame ] = FilterWithVoicebox(inFrame, fs )
%FILTERWITHVOICEBOX use Voicebox toolbox to filter signal

outFrame=inFrame; %just to initialize it

for i=1:size(inFrame,1)
    [outFrame(i,:)]=specsub(inFrame(i,:),fs);
end


end

