function [outFrame] = SelectChannels(inFrame,angle)
%SELECTCHANNELS

        
        %really want to just send mono but we can select which mono signal
        %intelligently
        %we'll use repmat to quickly copy the signal for both chanels prior
        %to sending it out
        display(['playing sound at angle ' num2str(angle)]);
        if(angle<-10.0) %sound is to left of midline
            outFrame=inFrame(1,:);
            %outFrame(2,:)=inFrame(1,:);
            %outFrame(2,:)=zeros(1,size(inFrame,2));

        elseif(angle>=10.0) %sound is right of midline
            outFrame=inFrame(2,:);
            %outFrame(2,:)=inFrame(2,:);
            %outFrame(2,:)=zeros(1,size(inFrame,2));
        else
            %there is no else, but if you know the signal is in front you could do a delay and
            %sum sort of approach
            %outFrame=inFrame;
            outFrame=mean(inFrame,1);
        end
        

end

