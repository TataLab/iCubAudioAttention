function [ newValue ] = UpdateOldObject(whichObject, feature, value, mode )
%UpdateOldObject takes the object you want to update, makes a copy of it,
%compares it to the object you pass it, and updates any new fields

%typically one imagines that one would only update the top (i.e. selected)
%object, but interestingly, maybe not always

%mode is a string that optionally tells how to handle the new object


[objFileMap,~,~]=MapObjectFile;

if strcmp(mode,'average') %average the values
    objFileMap.Data(whichObject,1).(feature)=(objFileMap.Data(whichObject,1).(feature)+value)/2;
else
    objFileMap.Data(whichObject,1).(feature)=value;
end

newValue=objFileMap.Data(whichObject,1).(feature);

end

