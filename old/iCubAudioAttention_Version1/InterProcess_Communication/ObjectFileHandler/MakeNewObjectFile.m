function MakeNewObjectFile(stackSize_objects)

%Create a new file on the disk that holds object files.  It should
%be memory mapped by other processes that want to change or access its data

%it should be initialized to hold a default object that can be pushed down
%by new objects

%%%%%%%%%%%%%%%%%%%%%%%%%%%
%ObjectFileFormat:
%always use GetNewEmptyObject to make a template so that we have an easy
%way to update the fields everywhere
%first field is always header!


%field:                 NumBytes:
%numObjects                         1 double
%'name'                                 8 chars (chars are uint16 in matlab)
%'rank'                                   1 double
%'azimuth'                              1 double
%'isNew'                                 double, 0 means old, 1 means new (i.e. gets
%                                           priority
%'isSelected'                           1 double true or false, there can be only
%                                            one (for now)                        
%'data'                                    not implemented yet

%the object struct repeats in the objects file.  Note the sequence within
%the struct is critical.  It won't get memory mapped properly if you mix
%these fields up
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


[o,~,sizeO_bytes]=GetNewEmptyObject;

headerBytes=16;  %a double at the front of the object file tells how many objects are on the stack (including the default object), a second double acts as a semaphore to prevent race conditions

%dummy variable to fill with zeros
temp=uint8(zeros(1,headerBytes + stackSize_objects*sizeO_bytes));
%temp(1)=0; %initialize to 0 and immediately add a default object (the stack can never be empty...hmmm, that's deep)

%open and initialize the file with zeros
%now the file is the correct size to hold stackSize_objects worth of
%objects
tempdir=dir('/tmp/ObjectFiles');
if(isempty(tempdir))  %make sure the directory structure exists
    mkdir('/tmp/ObjectFiles');
end
clear tempdir;

fid=fopen('/tmp/ObjectFiles/objects.dat','w');
fwrite(fid,temp);
fclose(fid);

%immediately add the default object as a special case

numObjectsMap=memmapfile('/tmp/ObjectFiles/objects.dat','format',{'double' [1 1] 'numObjects'},'writable',true);
numObjectsMap.Data(1,1).numObjects=1;


[objFileMap,~,~]=MapObjectFile;


%get a list of all the features stored in the objects
features=fieldnames(objFileMap.Data(1,1)); %size of this cell array tells you how many features

o.name='default1';
o.timeStamp=tic; %a useful reference, perhaps?
o.isDefault=1;
for i=1:length(features) %run through all the feature fields
        if(strcmp(features{i},'name'))
            objFileMap.Data(1,1).(features{i})=cast(o.(features{i}),'uint16'); %a hoop I jumped through so that you can assign with object names as strings...thank me later...-matt
        else
            objFileMap.Data(1,1).(features{i})=o.(features{i});
        end
end

    


