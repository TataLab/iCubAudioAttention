## ObjectRecognitionInfer ##
This is a simple modified code of the Tensorflow inference in C++ to work with Yarp.

### Dependencies ###
This module was tested only on a LInux machine and depends on the following :

- Yarp
- [TensorFlowCC](https://github.com/FloopCZ/tensorflow_cc) (Github repository installing and configuring TensorFlow for CMake) 
- OpenCV

Optionally a Cuda environment to launch inference on GPU

### General description ###
This is a C++ code that load a trained graph ( Tensorflow format .pb) with associated label and wrap it with 
Yarp.

The code is quite simple, from the Yarp input port an image is extracted and run into the graph with
preprocessing according to the architecture used for training. Each architecture depends on input transformation
such as normalisation ( substract the mean and divide by the standard deviation) this parameters are
dependent to the architecture.


### Yarp Input/Output ###
**RPC commands**
 - Get label => run the inference on the graph and output the maximum label
 
 **Inputs**:
 
 - /ObjectRecognitionInfer/imageRGB:i => Image stream from where to take snap to launch inference
 
 **Outputs**
 
 - None
 
 ### How to use it ##
 The module need 3 mandatory parameters taken from the objectRecognitionInfer.ini : 
 
 - **graph_path** => Path to the graph to use
 - **labels_path** => Path to the associated labels
 - **model_name** => Name of the model, this is used to compute the right parameters for preprocessing (mean, std, image_size)
 

### Model_name possible values : 
To use the mobileNet architecture you have to specify it like this  **mobilenet_<number_weight>_<image_size>** where : 
- number_weight can be in  {0.25|0.50|0.75|1}
- image_size can be in {224|192|168|128}

To use [inceptionV3](https://arxiv.org/abs/1512.00567) just write **inception_v3**
