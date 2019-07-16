These MATLAB functions memory map an audio data file and allow you to scan through it at any rate and
with any window sizes you want.

A note about timing:  The approach taken at this point is to synchronize the MATLAB processes to the 
audio data stream once at startup and then explicitly lag behind real-time.  No further synchronizing 
is done.  Frame reads are scheduled by keeping track of time rather than frame counts.  This is potential 
bad, but it works well to a first approximation.  However the code uses tic() and toc() to keep track of time.  
These are apparently very precise on mac/unix and unpredictably precise on windows.   So if you're on windows 
and something seems flaky, suspect the timing in the call to GetNextFrame() as a good first place to go troubleshooting.
