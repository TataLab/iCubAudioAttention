

[yy,SamplingFre]=readwav('./audioTest_right_to_left.wav');

%prepare noise

noise=rand(length(yy),1)*2-1;
noise=repmat(noise,[1 2]);

%add it
output=yy+noise*.5;

audiowrite('audioTest_right_to_left_noise.wav',output,44100);