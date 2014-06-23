writerObj= VideoWriter('spectral_dynamics.avi','Uncompressed AVI');
writerObj.FrameRate=2.3438;
open(writerObj);

writeVideo(writerObj,videoFrames);
