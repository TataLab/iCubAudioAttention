%use Psychophysics Toolbox to play multichannel audio with a continuous
%noise background

%some parameters...neatly bundled into a struct to keep the workspace tidy
global P;

P.sampleRate = 44100; %check the rate of the wav files you're using!
P.numChannels=8; %there are 8 mappable channels but we'll only use a subset of them
P.channelRange = [ 4 5 6 ]; %use these channels (inclusive)
P.degreesRange = [-55.0  0.0  55.0 ]; 
P.noiseChannel = 7;
P.noiseGain=.1;
P.numTrials = 10;  %number of talker events
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

noiseSource=audioread('/Users/Matthew/Documents/Robotics/iCubAudioAttention/data/sounds/pink_20seconds.wav');
noiseSource=noiseSource';


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

timeCounter=1;
trialAudio=zeros(P.numChannels,5*P.sampleRate);  %prefill with 5 seconds of noise
trialAudio(P.noiseChannel,:)=noiseSource(5*P.sampleRate)*P.noiseGain;

%build the trials
for trial=1:P.numTrials

    
    %build an array of audio
    utterance1=audioread([trialStruct{trial,1} '1.wav']); %read the first two audio files
    utterance2=audioread([trialStruct{trial,1} '2.wav']);
    utterance3=audioread([trialStruct{trial,1} '3.wav']);
    talker=[utterance1' utterance2' utterance3'];
    
    trialStruct{trial,4}=timeCounter/P.sampleRate;  %grab the start time;  
    trialStruct{trial,5}=(timeCounter+length(talker))/P.sampleRate;
    timeCounter=timeCounter+length(talker)+1;
    
    audioData=zeros(P.numChannels,length(talker)); %make an array
    thisChan=trialStruct{trial,2};
    audioData(thisChan,:)=talker;
    
    audioData(P.noiseChannel,:)=noiseSource(1:length(talker))*P.noiseGain; %fill the noise channel
   
    trialAudio=[trialAudio audioData];
    
    
end

%now play the whole thing

PsychPortAudio('FillBuffer',paHandle,trialAudio); %load the sound vector into the audio buffer

PsychPortAudio('Start',paHandle,1,[],[],[],[]); %start playback

PsychPortAudio('Stop',paHandle,1,0,[],[]); %schedule to stop playback when last sample is reached



%write out the data
save('trialStruct.mat','trialStruct');
