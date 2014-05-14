function [ objectFileMap,numObjectsMap,numObjects ] = MapObjectFile()

%handles memory mapping of an object file and returns the mapped file
sizeOfHeader_bytes=8;  %a double at the front of the object file tells how many objects are on the stack (including the default object)

numObjectsMap=memmapfile('/tmp/ObjectFiles/objects.dat','format',{'double' [1 1] 'numObjects'},'writable',true);
numObjects=numObjectsMap.Data(1,1).numObjects;

%using the template returned by GetNewEmptyObject we can find out what we
%need to set up a file full of objects, this is so we only have one place
%where objects are defined (i.e. in the template)

s=dir('/tmp/ObjectFiles/objects.dat');
objectFileSize_bytes=s.bytes-sizeOfHeader_bytes;  %find out how big the file is so we can memory map the entire thing...the header is one double, everything else is object data
[~,featureStructure,objectSize_bytes]=GetNewEmptyObject;  %find out how big each object representation is

objectFileSize_objects=objectFileSize_bytes/objectSize_bytes;  %that should work out to be integer

objectFileMap=memmapfile(   '/tmp/ObjectFiles/objects.dat','format', featureStructure, 'writable',true,'repeat',objectFileSize_objects, 'offset',sizeOfHeader_bytes);  %offset by the size of the header
                                        

return;

end

