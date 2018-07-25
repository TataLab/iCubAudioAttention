talkerOnsetsOffsets=[75175 204047 397355 494008 676577 762491 934320 1020235 1202803 1331675]./44100;
talkerOnsetsOffsets=round(talkerOnsetsOffsets/.33);



targetVector=zeros(1,96);
targetVector(5:14)=60;
targetVector(27:34)=30;
targetVector(46:52)=0;
targetVector(64:70)=-30;
targetVector(83:92)=-60;




%define thresholds
thresholds=[0 .1 .2 .4 .6];
faVector=zeros(length(thresholds),length(errorVector));
hitVector=zeros(length(thresholds),length(errorVector));
%define false alarm
for f=1:length(thresholds)
    
    tempSal=salienceVector>=thresholds(f);

    %a hit is when threshold is exceeded and talker is present
    %false alarms are when threshold is exceeded and no talker is present
    for j=1:length(targetVector)
        if(targetVector(j)==0 && tempSal(j)==1)
            faVector(f,j)=1;
        end
        
        if(targetVector(j)==1 && tempSal(j)==1)
            hitVector(f,j)=1;
        end
        
    end
    
   
end

numTargPresent=sum(targetVector);
numTargAbsent=length(targetVector)-numTargPresent;

FA=sum(faVector,2)./numTargAbsent;
H=sum(hitVector,2)./numTargPresent;

