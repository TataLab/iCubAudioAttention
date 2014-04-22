function NewObjectFile(newName)

%Create a new file on the disk that represents an object file.  It should
%be memory mapped by other processes that want to change or access its data

%ObjectFileFormat:

%field:                 NumBytes:
%'name'                             12 chars
%'rank'                               1 uint8
%'azimuth'                          1 double
%'isNew'                                0 means old, 1 means new (i.e. gets
%                                           priority

%'data'                                not implemented yet

%total bytes should be: 12*2 + 1*1 +1*1 + 1*8 = 34

%build the object file's contents
O.name=newName;
O.rank=uint8(1);
O.azimuth=double(0.0);
O.isNew=uint8(1);

%dummy variable to fill with zeros
temp=uint8(zeros(1,34));

%open and initialize the file with zeros
fid=fopen(['/tmp/ObjectFiles/' O.name],'w');
fwrite(fid,temp);
fclose(fid);

%memory map the file to write the proper data structure in
m=memmapfile(['/tmp/ObjectFiles/' O.name],'format',   {'uint16' [1 12] 'name';
                                                'uint8' [1 1] 'rank';
                                                'double' [1 1] 'azimuth';
                                                'uint8' [1 1] 'isNew'}, 'writable',true);

 m.Data(1,1).name=cast(O.name,'uint16');
 m.Data(1,1).rank=O.rank;
 m.Data(1,1).azimuth=O.azimuth;
 m.Data(1,1).isNew=O.isNew;  %this flags new objects that should be acted on ASAP
 
       
                                           
 
 
end

                                            




