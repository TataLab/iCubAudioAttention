%Simple behaviour that takes the object off the top of the stack and
%reports its angle

%it is selective for objects flagged by isSelected

rate=5; %hz...how fast to update this view of the object

[objFileMap,numObjMap,nObjectsInStack,isBusyMap]=MapObjectFile;
done=0;

while(~done)
    t=tic;
    clc;
    
    if(objFileMap.Data(1,1).isSelected==1)
        azimuth=objFileMap.Data(1,1).onsetAzimuth;
        display(['Angle to source: ' num2str(azimuth)]);
        
        %you could plot the azimuth
        
        
        hold off;
        [x,y] = pol2cart(azimuth/360.0*2*pi,1); %convert angle and unit radius to cartesian
        compass(x,y);
        drawnow;
    else
        %draw a blank compass plot
        display('No object is selected');
        [x,y] = pol2cart(0,0); %convert angle and unit radius to cartesian
        compass(x,y);
        drawnow;
    end
    
   
    timingProblem=1;
    while(toc(t) < 1/rate)
        %block
        timingProblem=0;
    end
    if(timingProblem==1)
        display('Problems:  ReportAttendedAngle might be lagging');
    end
    
    
   MaintainObjectStackOnce(objFileMap,numObjMap,nObjectsInStack,isBusyMap);
    
end

