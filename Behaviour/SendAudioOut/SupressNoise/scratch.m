[S,fs]=wavread('/Users/Matthew/Desktop/audio.wav');
S=S';

for i=1:2
    improvedS(i,:)=specsub(S(i,:),fs);
end
wavwrite(improvedS',fs,'junk1.wav');