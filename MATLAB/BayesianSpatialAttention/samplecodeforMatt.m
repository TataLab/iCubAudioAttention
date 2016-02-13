

if (move_flag==1) % Deal with the rotating the local frame and transfering Po between coordinates
    temp=zeros(1:length(Po));
    shift=(currentAngle-90);
    if shift>0
        temp(1:end-shift)=Po(shift:end); % shift
        temp(end-shift:end)=Po(1:shift); % flip over the rest of the Po
    elseif shift<0
        shift=-shift;% make a positive shift integer number
        temp(shift:end)=Po(1:end-shift); % shift
        temp(1:shift)=Po(end-shift:end); % flip over the rest of the Po
    end
    move_flag=0;% reset the flag
end

%%%%    RecursiveBayesian Function  %%%%
[Po]=RecursiveBayesian(Po,numBands);


[a,b]=find(Po==max(max(Po)));  % Get the peak value; it's most likely location of the sound.
sest=[Sa(a);Sb(b)];  %store the coordinates of this location

maxPo=max(max(Po));
summaxPo=summaxPo+maxPo;
sset_exp_num=(sset_exp_num+sest*maxPo);
sset_exp=(sset_exp_num)/(summaxPo);
% X=Sum(W(t)*P(Xi)*Xi)/Sum(W(t)P(Xi))
%TODO add a weight(W(t) as a function of time( Iteration) that put
%emphasis on the most recent ones