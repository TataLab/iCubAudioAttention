function [  ] = RemoveOldObject( objFileMap,numObjectsMap,numToRemove )
%REMOVEOLDOBJECT takes an object number and removes it from the stack
%note this is probably a really inefficient way to sort a list, but it is
%straightforward

%[objFileMap,numObjectsMap,~]=MapObjectFile;


if (objFileMap.Data(numToRemove,1).isDefault==1)
    display('Problems! Somebody is trying to remove the default object from the stack');
else %remove this object and shift everything below it up one on the stack
    features=fieldnames(objFileMap.Data(numToRemove,1)); %size of this cell array tells you how many features
    
    %keep going until you get to the default obj
    isDefault=0;
    while(~isDefault)
        
        for i=1:length(features) %run through all the feature fields
            objFileMap.Data(numToRemove,1).(features{i})=objFileMap.Data(numToRemove+1,1).(features{i});
        end
        %update for the next time around
        isDefault=objFileMap.Data(numToRemove,1).isDefault;
        numToRemove=numToRemove+1;
        
    end
end

%update the object count
numObjectsMap.Data(1,1).numObjects = numObjectsMap.Data(1,1).numObjects-1;

return
end


