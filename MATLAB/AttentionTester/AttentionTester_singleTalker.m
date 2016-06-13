%use Psychophysics Toolbox to play multichannel audio and output events so
%that we can evaluate performance of attention AIs

%talkers will appear randomly in the scene at random channels (between 3
%and 7).  We'll map those channels to discrete speakers in the VR lab.

%some parameters...neatly bundled into a struct to keep the workspace tidy
global P;

P.sampleRate = 44100; %check the rate of the wav files you're using!
P.numChannels=8; %there are 8 mappable channels but we'll only use a subset of them
P.channelRange = [4 5 6]; %use these channels (inclusive)
P.degreesRange = [-22.5 0 22.5]; 
P.numTrials = 52;  %number of talker events
P.audioFileRoot = '/Users/Matthew/Documents/MATLAB/sounds/PNNC Corpus';

%initialize psych sound
InitializePsychSound;
paHandle = PsychPortAudio('open',[],[],0,P.sampleRate,P.numChannels); %configure the audio hardware
PsychPortAudio('FillBuffer',paHandle,zeros(P.numChannels,100)); %load zeros to prime the audio hardware without making sound
pause(1); %give the hardware a moment to compose itself

%call start and stop on the port to "prime it"
PsychPortAudio('Start',paHandle,1,[],[],[],[]); %start playback
PsychPortAudio('Stop',paHandle,1,0,[],[]); %schedule to stop playback when last sample is reached

previousTalker=0;  %this flag will let us enforce that each talker is a new talker
previousChannel=0; %this flag will let us enforce that each talker is at a new location

%pre-build the structure of trial parameters
trialStruct=cell([P.numTrials,5]); % talker file; channel number; angle in degrees; start time; stop time;
for i=1:P.numTrials
   
    
    %choose a wav file for each trial
    talker=randi(5); %there are 5 different talkers in our set of the PNNC corpus
    while(talker==previousTalker)
        talker=randi(5);
    end
    previousTalker=talker;
    
    
    trialStruct{i,1}=[P.audioFileRoot '/speaker' num2str(talker) '/' ];

    %choose a channel
%     chanIndex=randi(length(P.channelRange)); %pick an index of a chan in the list
%     while(chanIndex==previousChannel) %don't pick the same as last trial
%         chanIndex=randi(length(P.channelRange));
%     end
%     previousChannel=chanIndex;
    
    %or if you want it to step through the channels
    chanIndex=mod(i,length(P.channelRange))+1;
    
    trialStruct{i,2}=P.channelRange(chanIndex);
    trialStruct{i,3}=P.degreesRange(chanIndex); %fill in the speaker angle in degrees
    %start and stop times will be grabbed on the fly
    
end


%run the trials
for trial=1:P.numTrials

    display(['running trial ' num2str(trial) ' of ' num2str(P.numTrials)]);

    display(['talker is at ' num2str(trialStruct{trial,3})]);
    
    plot(trial,trialStruct{trial,3},'ro');
    ylim([-180 180]);
    hold on;
    drawnow;
    
    %build an array of audio
    utterance1=audioread([trialStruct{trial,1} '1.wav']); %read the first two audio files
    utterance2=audioread([trialStruct{trial,1} '2.wav']);
    utterance3=audioread([trialStruct{trial,1} '3.wav']);
    talker=[utterance1' utterance2' utterance3'];
    audioData=zeros(P.numChannels,length(talker)); %make an array
    thisChan=trialStruct{trial,2};
    audioData(thisChan,:)=talker;
   
    
    %now fill the audio buffer with the sound vector we just built above
    PsychPortAudio('FillBuffer',paHandle,audioData); %load the sound vector into the audio buffer
    pause(1); %give the hardware a moment to compose itself
    
    PsychPortAudio('Start',paHandle,1,[],[],[],[]); %start playback
    trialStruct{trial,4}=tic;  %grab the start time;  

    PsychPortAudio('Stop',paHandle,1,0,[],[]); %schedule to stop playback when last sample is reached
    trialStruct{trial,5}=tic;  %grab the stop time;
    pause(1); %pause at least 1 second between talkers
    
end

%write out the data
save('trialStruct.mat','trialStruct');
