%set up a global struct of parameters to keep the workspace clean
global P;

P.kFrameSize_samples=10*1024;
P.kFixedLag_samples=6*10240; %lots of fixed lag so we know we're not missing data
P.kSampleRate=48000;
P.kFrameDuration_seconds=P.kFrameSize_samples/P.kSampleRate;
P.bitDepth_bytes=2;
P.kGain=15;  %you can add some gain (carefully) here.  Watch for clipping.

P.audioDataDumpFilename='/tmp/AudioDataDump.dat';
P.mostRecentSampleFilename='/tmp/lastSampleIndex.dat';

f=dir(P.audioDataDumpFilename);
P.fileSize_bytes=f.bytes;
P.kBytesPerSample=2; %16bit 
P.numChannels=2; %stereo

P.kDuration_samples=P.fileSize_bytes/P.kBytesPerSample/P.numChannels;
P.kDuration_seconds=P.kDuration_samples/P.kSampleRate;

%%%%%%%%%%%
%for filtering
%lowpass:

% P.Fp_hz=3000;
% P.Fst_hz=3500;
% P.Ap=10;
% P.Ast=40;
% P.Hlow=MakeLowPassFilter;
% % 


%bandpass
P.Fst1_hz=100;
P.Fp1_hz=400;
P.Fst2_hz=1000;
P.Fp2_hz=2000;
P.Ast1=20;
P.Abandp=1;
P.Ast2=20;
P.Hband=MakeBandPassFilter;