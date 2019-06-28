function [ objMap ] = RegisterNewObject( obj , name )
%MAKENEWOBJECT send it a data struct and a name and this function will
%register a new object into the object file

%object files are row vectors of doubles (or else it gets complicated to
%memory map)

%convert the object into a vector of doubles
data=[];
fields=fieldnames(obj);
for i=1:length(fields) %for every feature in the object

    temp=getfield(obj,fields{i});%#ok %pull out the feature from the structure
    
    data=[data temp];%#ok %cat it into one long row vector

end

%check if the file exists, if not build it
o=dir([name '.tmp']);
if(isempty(o))
    display([name ' does not exist. Will create new object file.']);
    fileID=fopen([name '.tmp'] ,'w');
    fwrite(fileID,data,'double');
    fclose(fileID);
    objMap  = memmapfile([name '.tmp'],'writable',true, 'format', {'double' [1 length(data)] 'o'});

    
else
    display([name ' already exists.  Will overwrite the current object.']);
    objMap  = memmapfile([name '.tmp'],'writable',true, 'format',{ 'double' [1 length(data)] 'o'});
    objMap.Data(1,1).o=data;
    
end






end

