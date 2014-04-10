AudioStreamHandler listens to audio data sent over a YARP network.  It is an unabashed hack of
yarphear (and does everything yarphear does).  Its party trick is that it writes frames of audio
data not just into memory, but also to a pre-initialized file.  This is useful so that several
instances of MATLAB can memory map that file and read audio data out at their own pace and each with
their own time windows and configurations.

