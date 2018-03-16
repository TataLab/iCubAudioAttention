function [ objStructOut ] = GetObjectFeatures( objMap )
%GETOBJECT reads the current object and returns its features in a
%convenient struct


data=objMap.Data;
objStructOut.angle=data(1);
objStructOut.onsetTime=data(2);
objStructOut.salience=data(3);
objStructOut.selected=data(4);

end

