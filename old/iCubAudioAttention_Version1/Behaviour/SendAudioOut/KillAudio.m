
display('done writing audio');


%close down neatly
PsychPortAudio('Stop', paoutput, 1);
PsychPortAudio('Close');
