%Simple behaviour that takes the object off the top of the stack and
%reports its contents

%******!!!!!Remember to add the yarp path into the java machine by typing
%javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp'); at the
%command line

%initialize a new object file
MakeNewObjectFile(20); %20 is more objects than you should ever need

rate=4; %hz...how fast to update this view of the object
useYARP=1;

[objFileMap,numObjMap,nObjectsInStack,isBusyMap]=MapObjectFile;
done=0;

timeOfObjectSentToYARP=uint64(0);  %we need a way to keep track of objects that have been reported to yarp, we'll use the timestamp as a unique ID

if (useYARP==1)   %remember to add yarp to the MATLAB java path:  javaaddpath('/Applications/yarp/MATLAB Java Classes/jyarp');
    LoadYarp;
    import yarp.Port;
    import yarp.Bottle;
    
    port=Port;
    
    port.open('/AudioAttention/AudioAngle');
         
    pause(3);
end



while(~done)
    t=tic;
    clc;
    currentNumObjects=numObjMap.Data(1,1).numObjects;
    display(['there are ' num2str(currentNumObjects) ' objects in the object stack:']);
    n=1;
    display(['the ' num2str(n) ' objects at the top of the stack are:']);
    for i=1:n
        tempObj=objFileMap.Data(n,1);  %a little fiddling to get the name to read out as a string rather than ascii
        tempObj.name=cast(tempObj.name,'char');
        display(tempObj);
    end
    
    %you could plot the azimuth
    
    if(objFileMap.Data(1,1).isSelected==1)
        azimuth=objFileMap.Data(1,1).onsetAzimuth;
        hold off;
        [x,y] = pol2cart(azimuth/360.0*2*pi,1); %convert angle and unit radius to cartesian
        compass(x,y);
        drawnow;
        
        if((useYARP==1) && (objFileMap.Data(1,1).timeStamp~=timeOfObjectSentToYARP)) %we didn't already report this one
            display(['sending ' num2str(azimuth) ' to YARP']);
            SendAngleToYarp(azimuth,port);
            timeOfObjectSentToYARP=objFileMap.Data(1,1).timeStamp;
        end
        
    else
        %draw a blank compass plot
        display('No object is selected');
        [x,y] = pol2cart(0,0); %convert angle and unit radius to cartesian
        compass(x,y);
        drawnow;
    end
    
    
    %call MaintainObjectStackOnce to clear out any old objects
   MaintainObjectStackOnce(objFileMap,numObjMap,currentNumObjects,isBusyMap);
    
    while(toc(t) < 1/rate)
        %block
    end
    
    
end

