function [didAddObject]=AddNewObject(OStruct)
%ADDNEWOBJECT
%takes a new object struct and adds it to the object stack
%default behaviour should be that new objects get automatically selected
%and go on the top of the stack

%AddNewObject must decide whether to integrate (merge with other recent
%objects on the stack) or substitute (displace other objects, namely the
%one at the top of the stack...masking)

%substitution is not implemented yet

maxIntegrationTime=0.50; %seconds, upper time bound on whether two objects are coincident enough to be called the same object
integrationTime_nanos=maxIntegrationTime*1000000000.0;


[objFileMap,numObjectsMap,numObjects]=MapObjectFile;


%get a list of all the features stored in the objects
features=fieldnames(OStruct); %size of this cell array tells you how many features


oldTime=objFileMap.Data(1,1).timeStamp;
newTime=OStruct.timeStamp;
deltaTime=double(newTime-oldTime);  %a bit of type casting voodoo:  remember that tic returns system time in nanos as a uint64

oldName=objFileMap.Data(1,1).name;
newName=cast(OStruct.name,'uint16'); %a bit more voodoo, object files encode name as an array of uint16 chars

if((~strcmp(oldName,newName)) &&  (deltaTime < integrationTime_nanos)) %add by integration
    display(['Integrating new object with object at top of stack with time delta ' num2str(deltaTime/1000000000.0)]);

    %integrate:  this is tricky, policies need to be set up for how to
    %handle different situations, extend this
    
    if(strcmp(newName,'frqObjxx')) % incomming object is a frequency transient so give it the onset azimuth of the older (spatial) object
        OStruct.onsetAzimuth=objFileMap.Data(1,1).onsetAzimuth;
    end  
    %else the incomming object is a spatial transient so keep and use its angle data
    %copy frequency data when that gets implemented
        
    for i=1:length(features) %run through all the feature fields
        if(strcmp(features{i},'name'))
            objFileMap.Data(1,1).(features{i})=cast(OStruct.(features{i}),'uint16'); %a hoop I jumped through so that you can assign with object names as strings...thank me later...-matt
        else
            objFileMap.Data(1,1).(features{i})=OStruct.(features{i});
        end
    end
    
else %add by shifting everything down and adding new object to top of stack
    display(['Adding new object to top of stack at:  ' num2str(newTime/1000000000.0) ' with time delta ' num2str(deltaTime/1000000000.0)]);
    
    %shift everything down...this is slow and inefficient
    index=numObjects; %work from the bottom of the stack up
    while(index>=1) %note that at initial startup, numObjects will be zero and only the default object will be added below
        
        for i=1:length(features) %run through all the feature fields, this is inefficient but MATLAB won't replace structs in memmaped fields, this could be reworked by directly writing to the file if it needs to be sped up...or you could switch to a pointer system
            
            objFileMap.Data(index+1,1).(features{i})=objFileMap.Data(index,1).(features{i});
            
        end
        
        index=index-1; %work up the stack from the bottom (to do: trap an error due to running off the bottom of the stack if you add too many objects)
        
    end
    
    %now add the new object at the top
    
    for i=1:length(features) %run through all the feature fields
        if(strcmp(features{i},'name'))
            objFileMap.Data(1,1).(features{i})=cast(OStruct.(features{i}),'uint16'); %a hoop I jumped through so that you can assign with object names as strings...thank me later...-matt
        else
            objFileMap.Data(1,1).(features{i})=OStruct.(features{i});
        end
    end
    
    %increment the number of objects on the stack
    numObjectsMap.Data(1,1).numObjects=numObjects+1;
    didAddObject=1; %flag that we added an object to the stack

    
    
end

end








