% peaktime= peakfinder(cochbychannel)

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
% end