%SoundHandler
%capture and process sound from stereo input (ideally the iCub
%microphones), but we can use PsychToolbox's PsychPortAudio as a proxy for
%developement


%grab sound segment and compute the angle to the speaker


%start with PsychToolbox portaudio stuff, move to iCub later


kFreq = 44100;
kThreshold=1;  %default to very very quiet
frameDuration_seconds=5.0; 
frame=[];

% Running on PTB-3? Abort otherwise.
AssertOpenGL;
recordedAudio =[];

while KbCheck; end;

% Perform basic initialization of the sound driver and get a handle to an
% audio device
InitializePsychSound;
pahandle = PsychPortAudio('Open', [], 2, 0, kFreq, 2);

pause(0.5);
% Preallocate an internal audio recording  buffer with a capacity of 10 seconds:
PsychPortAudio('GetAudioData', pahandle, 10);


%start the audio stream
PsychPortAudio('Start', pahandle, 0, 0, 1);

%wait until signal exceeds threshold
currentLevel = 0; %initialize
pause(0.5);

%grab audio data
currentTime=tic;
done=0;
numIterationsWithoutAudio=0;
while(toc(currentTime)<frameDuration_seconds)
   
            [audiodata offset overflow tCaptureStart] = PsychPortAudio('GetAudioData', pahandle);
            recordedAudio=[recordedAudio audiodata];
            pause(.5);
            
end
 


% Close the audio device:
PsychPortAudio('Close', pahandle);