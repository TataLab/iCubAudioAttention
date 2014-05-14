% Select An Object File to be attended
%scans object files looking for new ones and selects accordingly

%current selected object is the first one on the stack
%(this would be much cooler if the objects were in a linked list)

rate = .5; %how fast to update objects in hz
done=0; %for looping continuously

%queue up the mapping of the single byte in the object file that tells you
%how many objects there are
numObjectsMap=memmapfile('/tmp/ObjectFiles/objects.dat','format',{'uint8' [1 1] 'numObjects'});
numObjects=numObjectsMap.Data(1,1).numObjects;
prevNumObjects=0;

objFileMap=memmapfile(   '/tmp/ObjectFiles/objects.dat','format',   {'uint16' [1 8] 'name';
                                            'uint8' [1 1] 'rank';
                                            'double' [1 1] 'azimuth';
                                            'uint8' [1 1] 'isNew';
                                            'uint8' [1 1] 'isSelected'}, 'writable',true, 'offset',1,'repeat',numObjects);
    
objects=objFileMap.Data;

while(~done)
t=tic;


%current selected object is the first one on the stack
currentSelectedObject=1;

while(toc(t)<=1/rate) %block until time to scan again
        %do nothing
        display(['object ' int2str(currentSelectedObject) ' with azimuth ' num2str(objects(currentSelectedObject,1).azimuth) ' is selected']);

end




end