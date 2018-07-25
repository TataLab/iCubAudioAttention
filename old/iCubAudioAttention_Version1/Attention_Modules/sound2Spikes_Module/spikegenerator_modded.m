function [ spikes,aerdata] = spikegenerator_modded( cochwave,fs,numChannel,numCell )
%SPIKEGENERATOR 
% function spikes = spikegenerator_modded(cochwave, fs, numChannel, numCell)
% Take the filtered signal for each band and convert to spikes.
% Returns a 2-D matrix containing 1 or 0 for each time-frequency bin.

numEars=size(cochwave,2);
spikes=cell(1,numEars);
spl=20:10:20+numCell*10; % sound presure level from 20db
p_ref = 20*1e-6; % reference pressure in air is typically 20 uPa
meansignal=p_ref*10.^(spl/20);
for i=1:numEars%  for each ear
    for j=1:numChannel  % for each channel
        c=max(cochwave{1,i}(j,:),0);  % Halfwave rectifier
        %c=filter([1],[1 prea],c);
        spikebychannel=phaselock(c,numCell,meansignal);
        if size(spikes{1,i},1)==0
            spikes{1,i}=spikebychannel;
        else
            spikes{1,i}=[spikes{1,i};spikebychannel];
        end
        
    end
end

% plot spikes of all the frequency channels and neurons
[neuronID_l,sampleID_l]=find(spikes{1,1}==1);
sampleID_l=fix(sampleID_l*1000000/fs);
% plot (sampleID_l,neuronID_l, '.', 'MarkerSize', 10, 'Color', [0 0 1]);
% hold on;
[neuronID_r,sampleID_r]=find(spikes{1,1}==1);
sampleID_r=fix(sampleID_r*1000000/fs);
% plot (sampleID_r,neuronID_r, '.', 'MarkerSize', 10, 'Color', [0 1 0]);
% hold off;

% AER format of Spikes
aerdata=sortrows([[sampleID_l;sampleID_r],[neuronID_l;neuronID_r+numCell*numChannel]]);
% aer_time=aerData(:,1);
% aer_add=aerData(:,2);
end

function spikebychannel=phaselock(cochbychannel,numCell,threshold)

peaktime= peakfinder(cochbychannel);
spikebychannel=zeros(numCell,size(cochbychannel,2));

for i=1:size(peaktime,2)
    for j=1:numCell
        %         if coch(peaktime(i))>0.0001
        if cochbychannel(peaktime(i))>threshold(j)
            spikebychannel(j,peaktime(i))=1;
        end
    end
end

end

function peaktime= peakfinder(cochbychannel)
up=[];
down=[];

for i=2:(size(cochbychannel,2)-1)
    if cochbychannel(i)==0 && cochbychannel(i+1)>0
        up=[up i];
    elseif cochbychannel(i)==0 && cochbychannel(i-1)>0
        down=[down i];
    end
end

while up(1)>down(1)
    down(1)=[];
end

while size(up,2)>size(down,2)
    up(end)=[];
end

while size(up,2)<size(down,2)
    down(end)=[];
end

peaktime=fix((up+down)/2);
end