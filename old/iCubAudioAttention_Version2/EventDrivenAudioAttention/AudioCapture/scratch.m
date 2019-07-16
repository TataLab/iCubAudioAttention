



%pull audio data out sample-by-sample

frameSize=s.getSamples;
numChans=s.getChannels;

audioD=zeros(frameSize,numChans);
audioRawP=s.getRawData();

for i=1:numChans
    for j=1:frameSize
        
        audioD(j,i)=s.get(j,i-1); %i-1 because the yarp java class expects indices starting at zero
        
    end
end
