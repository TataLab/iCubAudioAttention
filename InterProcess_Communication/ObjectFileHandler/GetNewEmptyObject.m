function [ newObject,featureStruct,newObjectSize_bytes ] = GetNewEmptyObject()
%returns a template object structure
%
%everything about the object structure is set up here
%add new data fields here!!!!

%if you add a field here don't forget to change the number of bytes
%appropriately and the feature structure!!

newObject.name=cast('emptyobj','uint16');
newObject.rank=double(0);
newObject.onsetAzimuth=double(0);
newObject.azimuth=double(0);
newObject.isNew=double(0);
newObject.isDefault=double(0);
newObject.isSelected=double(0);
newObject.timeStamp=uint64(0);
newObject.blank=double(0);


featureStruct=                      {'uint16' [1 8] 'name';
                                            'double' [1 1] 'rank';
                                            'double'  [1  1] 'onsetAzimuth';
                                            'double' [1 1] 'azimuth';
                                            'double' [1 1] 'isNew';
                                            'double' [1 1] 'isDefault';
                                            'double' [1 1] 'isSelected';
                                            'uint64' [1 1] 'timeStamp';
                                            'double' [1 1] 'blank'
                                            };
                                        
%should be (num chars in name *2) + num doubles * 8 
newObjectSize_bytes=(8*2)+(8*8);

return;

end

