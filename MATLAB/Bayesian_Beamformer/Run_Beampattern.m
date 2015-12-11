low_cf=500;high_cf=4000;numchans=15;

cfs = MakeErbCFs_c(low_cf,high_cf,numchans);

angle =  180.0 * (0:500-1) / (500-1);

(500-1)*angle/180
angleRad = pi *  angle / 180;
N=2;D=.14/(N-1);
for frn=1:numchans;%100:200:20000
    fr=cfs(frn)
for SteeringAngle=0:1:90%.15/N:.15/N:.6/N


am=beampattern2(N,D,fr,SteeringAngle);
auc=trapz(angle,am-min(am));%area under curve
pdf=(am-min(am))/auc;
pdfv2(frn,SteeringAngle+1,:)=pdf;

figure(12)
plot(angle,pdf)
pause(.1)
end
end

break
figure (1)
plot(angle,am)

title([num2str(fr),',', num2str(D)])

figure (2)
plot(angle,am-min(am))
figure (3)
aaa2=trapz(angle,(am-min(am))/aaa);
plot(angle,(am-min(am))/aaa)