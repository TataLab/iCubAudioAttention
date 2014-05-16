%Simple behaviour that takes the object off the top of the stack and
%reports its contents

rate=4; %hz...how fast to update this view of the object

% %memory map the top object file
% objFileMap=memmapfile(   '/tmp/ObjectFiles/objects.dat','format',   {'uint16' [1 8] 'name';
%                                             'uint8' [1 1] 'rank';
%                                             'double' [1 1] 'azimuth';
%                                             'uint8' [1 1] 'isNew';
%                                             'uint8' [1 1] 'isSelected'}, 'writable',true, 'offset',1);
      
[objFileMap,numObjMap,nObjectsInStack,isBusyMap]=MapObjectFile;
done=0;

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
%    hold off;
%     [x,y] = pol2cart(tempObj.azimuth/360.0*2*pi,1); %convert angle and unit radius to cartesian
%     compass(x,y);
%     hold on;
%     [x0,y0] = pol2cart(tempObj.onsetAzimuth/360.0*2*pi,1); %convert angle and unit radius to cartesian
%     compass(x0,y0);
%     drawnow;
    
    %call MaintainObjectStackOnce to clear out any old objects
   MaintainObjectStackOnce(objFileMap,numObjMap,currentNumObjects,isBusyMap);
    

    
    while(toc(t) < 1/rate)
        %block
    end
    
    
end

