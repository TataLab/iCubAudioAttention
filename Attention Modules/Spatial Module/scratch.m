%generate some audio with known lags

noise=rand(1,48000)-.5;
silence=zeros(1,5*48000);
sig=[silence noise silence];

%zero lag
lag_0=repmat(sig,2,1);
wavwrite(lag_0',48000,'lag_0.wav');
lag_0_lag=SimpleGCC(lag_0);

%%%%%%%
%negative lags
%%%%%%%

%lag minus 5
lag_minus5=[sig;circshift(sig,[0 -5])];
wavwrite(lag_minus5',48000,'lag_minus5.wav');
lag_minus5_lag=SimpleGCC(lag_minus5);


%lag minus 15
lag_minus15=[sig;circshift(sig,[0 -15])];
wavwrite(lag_minus15',48000,'lag_minus15.wav');
lag_minus15_lag=SimpleGCC(lag_minus15);

%lag minus 15
lag_minus15=[sig;circshift(sig,[0 -15])];
wavwrite(lag_minus15',48000,'lag_minus15.wav');
lag_minus15_lag=SimpleGCC(lag_minus15);

%lag minus 18
lag_minus18=[sig;circshift(sig,[0 -18])];
wavwrite(lag_minus18',48000,'lag_minus18.wav');
lag_minus18_lag=SimpleGCC(lag_minus18);


%%%%%
%positive lags
%%%%%

%lag plus 5
lag_plus5=[sig;circshift(sig,[0 5])];
wavwrite(lag_plus5',48000,'lag_plus5.wav');
lag_plus5_lag=SimpleGCC(lag_plus5);


%lag plus 15
lag_plus15=[sig;circshift(sig,[0 15])];
wavwrite(lag_plus15',48000,'lag_plus15.wav');
lag_plus15_lag=SimpleGCC(lag_plus15);


%%%%%%
%tone lags with central noise
%%%%%%

t=linspace(0,2*pi,48000);
tone=sin(200*t) + sin(400*t) + sin(600*t) +sin(700*t) + sin(800*t);
tone=ScaleThis(tone);
tone_minus10=[noise noise noise noise+tone noise noise; noise noise noise noise+circshift(tone,[0 -10]) noise noise];
tone_minus10=ScaleThis(tone_minus10);
wavwrite(tone_minus10',48000,'tone_minus10.wav');

tone_plus5=[noise noise noise noise+tone noise noise; noise noise noise noise+circshift(tone,[0 5]) noise noise];
tone_plus5=ScaleThis(tone_plus5);
wavwrite(tone_plus5',48000,'tone_plus5.wav');

%%%%%
%tone lags with central tone
%%%%%
monotone=sin(300*t);
tone_minus10_monotone=[monotone monotone monotone monotone+tone monotone monotone; monotone monotone monotone monotone+circshift(tone,[0 -10]) monotone monotone];
tone_minus10_monotone=ScaleThis(tone_minus10_monotone);
wavwrite(tone_minus10_monotone',48000,'tone_minus10_monotone.wav');


%%%%
%tone with uncorrelated background
%%%%%
noise1=rand(1,48000)*.1;
noise2=rand(1,48000)*.1;
tone_minus10_uncorBack=[noise1 noise1 noise1 noise1+tone noise1 noise1; noise2 noise2 noise2 noise2+circshift(tone,[0 -10]) noise2 noise2];
tone_minus10_uncorBack = ScaleThis(tone_minus10_uncorBack);
wavwrite(tone_minus10_uncorBack',48000,'tone_minus10_uncorBack.wav');


%%%%
%tone with weak correlated background
%%%%%
noise1=rand(1,48000)*.1;
noise2=noise1;
tone_minus10_weakCorBack=[noise1 noise1 noise1 noise1+tone noise1 noise1; noise2 noise2 noise2 noise2+circshift(tone,[0 -10]) noise2 noise2];
tone_minus10_weakCorBack = ScaleThis(tone_minus10_weakCorBack);
wavwrite(tone_minus10_weakCorBack',48000,'tone_minus10_weakCorBack.wav');
