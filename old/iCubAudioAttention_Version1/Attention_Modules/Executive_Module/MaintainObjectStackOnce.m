function [removedObjects]=MaintainObjectStackOnce(stack,numObjectsMap,isBusyMap,verbose)

%%%%%%%%%%%%%%%%
%this module scans the object stack once and does "housekeeping".  Mainly it removes
%old objects.
%
%you can call this from within a loop to periodically prune the stack


%display('Performing maintenance on object stack'); 

if(isempty(verbose))
    verbose=0;
end


n=numObjectsMap.Data(1,1).numObjects;

objectLifetime = 1.0;  %seconds...how many seconds should an object stay on the stack

removedObjects=0; %default to zero

while(isBusyMap.Data(1,1).isBusy==1)
    %the object stack is being written by another process, so block here
    if(verbose==1)
        display('waiting patiently for my turn to modify the object stack');
    end
end

isBusyMap.Data(1,1).isBusy=1;

if(stack.Data(1,1).isDefault~=1) %if the object at the top of the stack is the default object there's nothing to maintain
    
    %scan the objects for old objects, the oldest object should be the
    %second from the bottom, but this makes it explicit
    if(verbose==1)
        display('Found a non-default object');
    end
    for i=1:n-1 % never time out the bottom object because that will be the default object
        if(toc(stack.Data(i,1).timeStamp)>objectLifetime)
            RemoveOldObject(stack,numObjectsMap,i);
            if(verbose==1)
                display(['Object ' num2str(i) ' has been on the stack for ' num2str(toc(stack.Data(i,1).timeStamp)) ' seconds.  Removing it now.']);
            end
        end
    end

end
   
isBusyMap.Data(1,1).isBusy=0;

return;

end


