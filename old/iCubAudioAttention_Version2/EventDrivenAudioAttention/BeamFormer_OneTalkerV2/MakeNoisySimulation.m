

[yy,SamplingFre]=readwav('./audioTest_right_to_left.wav');

% %prepare noise
% 
% noise=rand(length(yy),1)*2-1;
% noise=repmat(noise,[1 2]);


%read fn noise
[xx,SamplingFre]=readwav('./20-Fan-10min.wav');
xx(length(yy)+1:end,:)=[]; %trim the end

noiseAngle=



%add it
output=yy+xx;
output=output';
output=ScaleThis(output);
output=output';

audiowrite('audioTest_right_to_left_fan.wav',output,SamplingFre);