function [outFrame] = SelectChannels(inFrame,angle)
%SELECTCHANNELS

        
        %really want to just send mono but we can select which mono signal
        %intelligently
        
        if(angle<20.0) %sound is to left of midline
            outFrame(1,:)=inFrame(1,:);
            outFrame(2,:)=inFrame(1,:);
            %outFrame(2,:)=zeros(1,size(inFrame,2));

        elseif(angle>20.0) %sound is right of midline
            outFrame(1,:)=inFrame(2,:);
            %outFrame(1,:)=zeros(1,size(inFrame,2));
            outFrame(2,:)=inFrame(2,:);
        else
            %there is no else, but if you know the signal is in front you could do a delay and
            %sum sort of approach
            outFrame=mean(inFrame,2);
            
        end
        

end

