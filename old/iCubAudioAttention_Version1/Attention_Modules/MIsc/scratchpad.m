%scratchpad

f=44100;

%left side
leftSound=wavread('/Users/Matthew/Documents/MATLAB/Icub Interface/FindSpeakerAngle/AudioData/lessReverberation/left.wav');
leftSound=leftSound'; %for ease of mind

[leftAngle]=ComputeAngle(leftSound,f,1);


%right side
rightSound=wavread('/Users/Matthew/Documents/MATLAB/Icub Interface/FindSpeakerAngle/AudioData/lessReverberation/right.wav');
rightSound=rightSound'; %for ease of mind

[rightAngle]=ComputeAngle(rightSound,f,1);

%centre
centreSound=wavread('/Users/Matthew/Documents/MATLAB/Icub Interface/FindSpeakerAngle/AudioData/lessReverberation/centre.wav');
centreSound=centreSound'; %for ease of mind

[centreAngle]=ComputeAngle(centreSound,f,1);
