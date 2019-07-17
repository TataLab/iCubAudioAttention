![iCubAudioAttention](doc/images/ULETH_iCub_head.png?raw=false "iCubAudioAttention")

:robot: iCubAudioAttention :loud_sound:
===

A collection of signal processing algorithms and audio capture tools implemented using YARP for the iCub humanoid robot.


Installation
------------

* This repository depends on [YARP](https://github.com/robotology/yarp) (minimum version 3.2), [icub-main](https://github.com/robotology/icub-main), and [icub-contrib-common](https://github.com/robotology/icub-contrib-common), which can all be found on the robotology github. [OpenCV](https://github.com/opencv/opencv) is an optional dependency, for compiling visualisation tools. [YCM](https://github.com/robotology/ycm) (YARP CMake) is another optional dependency that offers extra CMake modules for YARP. If YCM is not found, local versions of the YCM modules will be used.
* For step-by-step installation instructions from a fresh install, you can follow our guide [here](https://github.com/TataLab/iCubAudioAttention/blob/refact/doc/installation_instructions/fresh_install.md).

```bash
git clone https://github.com/TataLab/iCubAudioAttention.git
cd iCubAudioAttention && mkdir build && cd build
ccmake ..
```

* In the CCMake interface you can enable/disable flags
    * BUILD_SHARED_LIBS &rightarrow; Compiles audioCubLib as a shared object, instead of static.
    * COMPILE_TESTS &rightarrow; Compiles some modules for testing other modules.
    * ENABLE_COMPILE_ON_PC104 &rightarrow; Only compiles modules that would be run on the robot.
    * ENABLE_OMP &rightarrow; Compiles processing modules with OpenMP multithreading.
    * ICUBCONTRIB_INSTALL_WITH_RPATH &rightarrow; Sets an rpath after installing modules.
    * ICUBCONTRIB_SHARED_LIBRARY &rightarrow; Compiles audioCubLib as a shared object, instead of static.

```bash
make -j
make install
```

Applications for yarpmanager will be installed in the ICUBcontrib directory in templates/applications. These should be modified and installed in the normal applications directory to be used.


Modules Description
-------------------

[Audio Preprocessor](https://github.com/TataLab/iCubAudioAttention/blob/refact/modules/audioPreprocessor/doc/README.md) : Used to spectrally and spatially decompose a yarp sound object. Can optionally do additional processing to isolate for acoustic dynamics found in natural human speech.

[Audio Bayesian Map](https://github.com/TataLab/iCubAudioAttention/blob/refact/modules/audioBayesianMap/doc/README.md) : Takes output from the audioPreprocessor in the form of an allocentric map, and performs Bayesian updates by combining new evidence with a running prior knowledge of the auditory scene.

[Audio Experiment](https://github.com/TataLab/iCubAudioAttention/blob/refact/modules/audioExperiment/doc/README.md) : Contains a variety of tools and modules to streamline the process of recording audio and running experiments.

[Frequency Visualisation](https://github.com/TataLab/iCubAudioAttention/blob/refact/modules/frequencyVisualisation/doc/README.md) : A module that receives a yarp matrix, and plots it as a yarp image. Allows live visualisation of the modules.

[Remote Interface](https://github.com/TataLab/iCubAudioAttention/blob/refact/modules/remoteInterface/doc/README.md) : A simple straight forward interface for streaming yarp sound objects recorded from the microphones on the robots head.

