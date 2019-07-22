![Preprocessor](/doc/images/Model_FeatureExtraction.png?raw=false "Preprocessor")

Audio Preprocessor
===

This module uses a series of signal processing algorithms found in the [audioCubLib](https://github.com/TataLab/iCubAudioAttention/tree/master/audioCubLib) to spectrally and spatially decompose the auditory scene around the robot.


Parameters
---

This module takes quite a few parameters, which can all be tuned. You can either individually change the parameters by calling the application with command line arguments:

```bash
audioPreprocessor --<group> <param> <value>
```

or by modifing the .ini file. The default context for this module is ```audio_attention``` and the default name for the config file is ```audio_attention_config.ini```. These can be changed through the command line arguments:

```bash
audioPreprocessor --context <parameter> --from <parameter>.ini
```

The parameters found in the configuration file are organised by groups. They are as follows:

```robotspec```
* ```azimuthIndex``` ( int ) &rightarrow; the index from a yarp bottle that should be used as the input azimuth (See ```/headAngle:i``` in ports descriptions below). [Default is 2]
* ```numMics``` ( int ) &rightarrow; the number of microphones that are on the robot. [Default is 2]
* ```micDistance``` ( double ) &rightarrow; the distance (in meters) between the left and right microphone. [Default is 0.145]

```sampling```
* ```speedOfSound``` ( double ) &rightarrow; the speed of sound. [Default is 336.628]
* ```samplingRate``` ( int ) &rightarrow; the number of samples captured in a second by the microphones. [Default is 48000]
* ```numFrameSamples``` ( int ) &rightarrow; the number of samples in a single frame passed to the preprocessor. [Default is 4096].
* ```sampleNormaliser``` ( double ) &rightarrow; used to scale down the input yarp sound object before filtering. [Default is 1024.0]

```processing```
* ```numBands``` ( int ) &rightarrow; the number of center-frequencies to use for the gammatone filter bank. The center-frequencies are dynamically set beased one the ```lowCf``` and ```highCf``` values. [Default is 128]
* ```lowCf``` ( double ) &rightarrow; used to set the lowest center-frequency used by the gammatone filter bank. [Default is 380.0]
* ```highCf``` ( double ) &rightarrow; used to set the highest center-frequency used by the gammatone filter bank. [Default is 7800.0]
* ```erbSpaced``` ( boolean ) &rightarrow; if enabled, gives the ```numBands```-center-frequencies an equivalent rectangular bandwidth distribution. If disabled, gives a linear distribution to the center-frequencies. [Defaut is true]
* ```halfRec``` ( boolean ) &rightarrow; enables half-wave rectifying which drops negative values from the basilar membrane response. [Default is false]
* ```computeEnvelope``` ( boolean ) &rightarrow; if enabled computes the envelope and does a band-pass of each band-by-beam pairing before extrapolating. When used, the ```numFrameSamples``` should be increased to 16384 or 32768 and the ```numBands``` should be reduced to 64 (this computation can be used to isolate for natural human speech). [Default is false]
* ```downSampEnv``` ( int ) &rightarrow; the number of samples to scale down the beamformed audio by, prior to computing the envelope. This increases overall computation speeds at a slight cost to accuracy. [Default is 4]
* ```downSampMethod``` ( string ) &rightarrow; the method to be used when down sampling (drop | rms | mean). [Default is drop]
* ```bandPassFreq``` ( double ) &rightarrow; the center-frequency that will be used in the band-pass of the envelope. [Default is 5.0]
* ```bandPassWidth``` ( double ) &rightarrow; the width of the band-pass filter. [Default is 0.5]
* ```angleRes``` ( int ) &rightarrow; the number of indicies that should be used for each degree's resolution. i.e. 1=360, 2=720, 3=1080. [Default is 1]
* ```windowLength``` ( int ) &rightarrow; number of samples to use when windowing a frame. [Default is 128]
* ```hopLength``` ( int ) &rightarrow; number of samples to jump to for the next window. [Default is 128]
* ```windowMethod``` ( string ) &rightarrow; the operation to be used when windowing a frame (rms | mean). [Default is rms]
* ```downSampOut``` ( int ) &rightarrow; the number of samples to scale down yarp matrices with ```numFrameSamples``` as their column value, prior to being written on their port. [Default is 1]
* ```numOmpThreads``` ( int ) &rightarrow; if the preprocessor was compiled with the flag ```ENABLE_OMP```, this value changes the number of OpenMP threads that get spawned. [Default is 4]


Ports
-----
This module has two input ports, and about fifteen output ports. The output ports are simply the resulting matrices from the processing, and are there simply for convenience. Additional processing for those ports are not done, unless they're connected to. 

All port names are preceeded by the modules name. The default for this is ```/audioPreprocessor```.

```Input Ports```
* ```/rawAudio:i``` ( Sound ) &rightarrow; expects a sound object. This frame of audio will be the base for the processing.
* ```/headAngle:i``` ( Bottle ) &rightarrow; expects a bottle object. This bottle should contain information about the robots pose in the world. This bottle should come from one of two places, either the ```yarprobotinterface``` from YARP or the ```iKinGazeCtrl``` from icub-main. The contents of the bottle should be the approximate azimuth of the robot. So if you are using ```yarprobotinterface```, the value used should be the ```pan``` of the head (the index of the bottle that will be taken can be adjusted from the ```robotspec``` parameter ```azimuthIndex```; by default this is 2). If you are using ```iKinGazeCtrl```, the value used should be the ```azimuth``` of the robot (which takes into consideration the roll, pitch, and torso of the robot). The value received will be used to offset an ```egocentric``` perspective of the auditory scene into an ```allocentric``` perspective.

```Output Ports```
* ```/gammatoneFilteredAudio:o``` ( Matrix ) &rightarrow; the basilar membrane response from the gammatone filtered raw audio. [Dim: numMics x numBands, numFrameSamples // downSampOut]
* ```/gammatoneFilteredPower:o``` ( Matrix ) &rightarrow; the RMS of the basilar membrane response of the gammatone filtered raw audio. [Dim: numBands, numMics]
* ```/gammatoneFilteredWindow:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/beamformedAudio:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/beamformedWindow:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/beamformedRmsAudio:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/beamformedRmsPower:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/allocentricAudio:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/hilbertEnvelope:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/hilbertEnvelopeWindow:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/bandPassedEnvelope:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/bandPassedEnvelopeWindow:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/bandPassedRmsEnvelope:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/bandPassedRmsPower:o``` ( Matrix ) &rightarrow; [Dim: , ]
* ```/allocentricEnvelope:o``` ( Matrix ) &rightarrow; [Dim: , ]