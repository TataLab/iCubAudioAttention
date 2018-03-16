%call this to stop PsychPortAudio and stop playing the audio buffer


display('done writing audio');


%close down neatly
PsychPortAudio('Stop', paoutput, 1);
PsychPortAudio('Close');
