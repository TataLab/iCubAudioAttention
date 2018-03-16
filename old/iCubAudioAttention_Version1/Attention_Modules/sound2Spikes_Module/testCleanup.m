function testCleanup()
vid=0;
for i=0:1000000000000000000
    disp('In main func');
end
% cleanup(vid);
 onCleanup(@()cleanup(vid));    
end

function cleanup(vid)
    disp('Cleaning up and closing audio and video devices');
%     stop(vid);
end