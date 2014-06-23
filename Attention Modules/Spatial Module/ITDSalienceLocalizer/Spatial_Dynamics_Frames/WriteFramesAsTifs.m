framePath='spatial_dyanamics_frame_';
for i=1:length(videoFrames)
    f=[framePath num2str(i) '.tif'];
    imwrite(videoFrames(i).cdata,f,'tif');
end
