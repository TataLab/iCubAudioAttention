%A utility to make sure that data is being written fast enough


%memory map the most-recent sample
[~,sampleD]=OpenAudioInputData;

done=0;

oldFrame=0;
newFrame=0;

display(newFrame);
while(~done)
    t=tic;
    while(oldFrame==newFrame)
        newFrame=sampleD.Data(1,1).f;
    end
    display(['that frame took ' num2str(toc(t)) ' seconds to arrive']);
    oldFrame=newFrame;
end