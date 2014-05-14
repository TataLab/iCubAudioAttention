%%%%%%%%%%%%%%%%
%this module scans the object stack and does "housekeeping".  Mainly it removes
%old objects.  

objectLifetime = 5.0;  %seconds...how many seconds should an object stay on the stack

[objFileMap,numObjectsMap,n]=MapObjectFile;
features=fieldnames(objFileMap.Data(1,1));  % a list of all features in objects

done=0;
while(~done)
    %loop continuously
    
    while(objFileMap.Data(1,1).isDefault~=1) %if the object at the top of the stack is the default object there's nothing to maintain
        
        %scan the objects for old objects, the oldest object should be the
        %second from the bottom, but this makes it explicit
        
        for i=1:n-1 %never time out the top object, never time out the bottom object because that will be the default object
            if(toc(objFileMap.Data(i,1).timeStamp)>objectLifetime)
                RemoveOldObject(i);
                display(['Object ' num2str(i) ' timed out.  Removing it now.']);
            end
        end
        
%         
%         
%         %integrate top two objects if they are coincident in time and one is frequency and the other is ITD, notice this
%         %could miss some coincidences if this isn't cycling fast enough
%         
%         obj1Time=objFileMap.Data(1,1).timeStamp;
%         obj2Time=objFileMap.Data(2,1).timeStamp;
%         deltaTime=double(obj2Time-obj1Time)/1000000000.0; %compute time difference in seconds
%         display(['Delta Time: ' num2str(deltaTime)]); 
%         if(deltaTime < integrationTime)
%            
%             
%             if(strcmp(objFileMap.Data(1,1).name,cast('ITDObjxx','uint16')))  %the top object was an ITD transient
%                 if(strcmp(objFileMap.Data(2,1).name,cast('frqObjxx','uint16'))) %the second object was a frequency transient
%                     new=GetNewEmptyObject;
%                     new.name='mergedxx';
%                     new.timeStamp=objFileMap.Data(1,1).timeStamp; %use the time of the more recent object?
%                     new.onsetAzimuth=objFileMap.Data(1,1).onsetAzimuth;  %object 1 has the azimuth
%                     %RemoveOldObject(1);
%                     %RemoveOldObject(1);
%                     AddNewObject(new);
%                 end
%                 
%             elseif(strcmp(objFileMap.Data(1,1).name,cast('frqObjxx','uint16')))  %the top object was an frequency transient
%                 if(strcmp(objFileMap.Data(2,1).name,cast('ITDObjxx','uint16'))) %the second object was an ITD transient
%                     new=GetNewEmptyObject;
%                     new.name='mergedxx';
%                     new.timeStamp=objFileMap.Data(1,1).timeStamp;
%                     new.onsetAzimuth=objFileMap.Data(2,1).onsetAzimuth;  %object 1 has the azimuth
%                     %RemoveOldObject(1);
%                     %RemoveOldObject(1);
%                     AddNewObject(new);
% 
%                 end
%             end
            
            %replace the top two objects with this single merged object...whoa
            %this is probably not thread safe
            
            
            %don't respond to situations when there are two consecutive
            %objects of the same type...wait that's not ideal...it's probably the first one
            %that we want...come back to this
            
        end
    end
    
end


