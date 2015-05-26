
timeVector=1:96;
timeVector=timeVector.*.33;

talker60=[75175 204047] ./48000 ;
talker30=[397355 494008] ./48000 ;
talker0=[676577 762491] ./48000 ;
talkerNeg30=[934320 1020235] ./48000 ;
talkerNeg60=[1202803 1331675] ./48000 ;


%plot the angles sent to iCub and indicate talker onsets
subplot(2,1,1);
plot(timeVector,azimuthVector(1:96),'r');
ylim([-80 80]);
title('Azimuth of Selected Beam (with fan at 0 deg)');

xlabel('time (seconds)');
ylabel('selected Angle (deg)');
line(talker60,[60 60]);
line(talker30,[30 30]);
line(talker0,[0 0]);
line(talkerNeg30,[-30 -30]);
line(talkerNeg60,[-60 -60]);

%plot the saliences sent to iCub and indicate talker onsets
subplot(2,1,2);
plot(timeVector,salienceVector(1:96),'r');
ylim([0 1.5]);
title('Salience of Selected Beam');
xlabel('time (seconds)');
ylabel('scaled salience');
line(talker60,[1.1 1.1]);
line(talker30,[1.1 1.1]);
line(talker0,[1.1 1.1]);
line(talkerNeg30,[1.1 1.1]);
line(talkerNeg60,[1.1 1.1]);

