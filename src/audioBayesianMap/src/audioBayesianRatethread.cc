// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Marko Ilievski, Austin Kothig
  * email: m.ilievski@uleth.ca, matthew.tata@uleth.ca, francesco.rea@iit.it
  * Permission is granted to copy, distribute, and/or modify this program
  * under the terms of the GNU General Public License, version 2 or any
  * later version published by the Free Software Foundation.
  *
  * A copy of the license can be found at
  * http://www.robotcub.org/icub/license/gpl.txt
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details
*/

#include "audioBayesianRatethread.h"


AudioBayesianRatethread::AudioBayesianRatethread() : RateThread(THRATE) {
	robot = "icub";
}


AudioBayesianRatethread::AudioBayesianRatethread(std::string _robot, std::string _configFile, yarp::os::ResourceFinder &rf) : RateThread(THRATE) {
	robot = _robot;
	configFile = _configFile;
	loadFile(rf);
}


AudioBayesianRatethread::~AudioBayesianRatethread() {
	delete inputMatrix;
	delete outputMatrix;
	delete inPort;
	delete headAngleInPort;
	delete outPort;
    delete outProbabilityPort;
}


bool AudioBayesianRatethread::threadInit() {

	// Allocating the required vectors for the modules to function properly
	for (int i = 0; i < nBands; i++) {

		std::vector<double> tempvector;
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			tempvector.push_back(1.0);
		}

		// Vector containing the most recent audio map
		// that was sent to this module though yarp
		currentAudioMap.push_back(tempvector);

		// The noise map that was created, representing
		// the current ego locked noise in the environment
		noiseMap.push_back(tempvector);

		// A short Bayesian map term map
		shortMap.push_back(tempvector);

		// A medium term Bayesian map term map
		mediumMap.push_back(tempvector);

		// A long term Bayesian map term map
		longMap.push_back(tempvector);
	}
    
    // initialize probability angle map.
    for (int i = 0; i < interpolateNSamples * 2; i++) {
    	longProbabilityAngleMap.push_back(0.0);
    }

    //longProbabilityAngleMap.assign(interpolateNSamples * 2,0);



	// Allocates the required memory for the yarp matrix
	// that takes the input and output to this module
	inputMatrix = new yarp::sig::Matrix(nBands, interpolateNSamples * 2);
	outputMatrix = new yarp::sig::Matrix(nBands, interpolateNSamples * 2);
	outProbabilityMap = new yarp::sig::Vector(interpolateNSamples * 2);

	// Initializes the variable that keeps track of how
	// many frames have been gathered for the noise map
	noiseBufferMap = 0;
	first = true;


	// Creates a buffered yarp port as input into this module
	// with the name /iCubAudioAttention/BayesianMap:i
	// The expected input into this module is a yarp
	// matrix with the size of (beams * bands)
	// The matrix is created in the yarpPreprocessing module
	inPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
	if (!inPort->open("/iCubAudioAttention/BayesianMap:i")) {
		yError("unable to open port to receive input");
		return false;
	}

	headAngleInPort = new yarp::os::BufferedPort<yarp::os::Bottle>();
	if (!headAngleInPort->open(getName("/BayesianHeadAngle:i").c_str())) {
        yError("unable to open port to receive input");
        return false;  // unable to open; let RFModule know so that it won't run
	}

	// Creates a yarp port as output from this module
	// with the name /iCubAudioAttention/BayesianMap:o
	// The output of this module is a yarp matrix with size of (beams * bands)
	// The output corresponds to only the long term audio map
	outPort = new yarp::os::Port();
	if (!outPort->open("/iCubAudioAttention/BayesianMap:o")) {
		yError("unable to open port to send bayesian map");
        return false;
	}

	outProbabilityPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
	if (!outProbabilityPort->open("/iCubAudioAttention/ProbabilityMap:o")) {
		yError("unable to open port to send probability map");
        return false;
	}


	//TODO add output port for the medium and short term maps


	// get the noise map prior that was pre-recorded or bail out
	// open the previously recorded noiseMap prior
	FILE *fidNoise = fopen("./noiseMap.dat", "r");

	// if(fidNoise==NULL){
	// 	printf("\n\nYou need to pre-record a noise map prior\n\n");
	// 	return false;
	// }

	// double tempNoiseMap[nBands*interpolateNSamples*2];
	// fread(tempNoiseMap,sizeof(double),nBands*interpolateNSamples*2,fidNoise);
	// fclose(fidNoise);

	int count=0;
	// for (int i=0;i<nBands;i++) {
	//		std::vector<double> tempvector;
	// 		for int j=0;j<interpolateNSamples*2;j++){
	// 			tempvector.push_back(tempNoiseMap[count++]);
	// 		}
	// 		noiseMap.push_back(tempvector);
	// }
	//numberOfNoiseMaps = 100;

	return true;
}


void AudioBayesianRatethread::setName(std::string str) {
	this->name=str;
}


std::string AudioBayesianRatethread::getName(const char* p) {
	std::string str(name);
	str.append(p);
	return str;
}


void AudioBayesianRatethread::run() {

	// Reads the a matrix from the input port
	// This is a blocking call thus the module will
	// wait until it has acquired the required matrix
	inputMatrix = inPort->read(true);


	// Gathers the time/counter envelope that was
	// associated with the last message
	inPort->getEnvelope(ts);


	// Calls a function that will take the current
	// audio map and create the Bayesian maps
    // reads the angles passed via input port	
    setAcousticMap(); 


	if (outPort->getOutputCount()) {	    		
        // copies the data in the longMap vector into the outputMatrix
		// and sends the matrix along with the envelope via the output Port
		sendAudioMap(longMap);
	}
    

   // if (outProbabilityPort->getOutputCount()) {
        // copoes the data in the longProbabilityAngleMap vector into the 
        // outProbabilityMap and sends it along with the envelope via the 
        // outProbability port.
    	sendProbabilityMap(longProbabilityAngleMap);
   // }   

	// Calls the Memory maper and memory maps it to
	// the following file: /tmp/bayesianProbabilityLongMap.tmp
	stopTime = yarp::os::Time::now();
	yInfo("Count:%d Time:%f. \n", ts.getCount(),  stopTime-startTime);
	startTime = stopTime;
}


bool AudioBayesianRatethread::processing() {
	// here goes the processing...
	return true;
}


void AudioBayesianRatethread::threadRelease() {
	// stop all ports
	inPort->interrupt();
	outPort->interrupt();
	headAngleInPort->interrupt();

	// release all ports
	inPort->close();
	outPort->close();
	headAngleInPort->close();
}


void AudioBayesianRatethread::loadFile(yarp::os::ResourceFinder &rf) {

	// import all relevant data fron the .ini file
	yInfo("loading configuration file");
	try {
		nBands 				 =  rf.check("nBands",
										 yarp::os::Value("128"),
										 "numberBands (int)").asInt();

		interpolateNSamples  =  rf.check("interpolateNSamples",
										 yarp::os::Value("180"),
										 "interpellate N samples (int)").asInt();

		longTimeFrame 		 =  rf.check("longBufferSize",
										 yarp::os::Value("360"),
										 "long Buffer Size (int)").asInt();

		nMics  				 =  rf.check("nMics",
										 yarp::os::Value("2"),
										 "numberBands (int)").asInt();

		yInfo("nBands = %d", nBands);
		yInfo("nMics = %d", nMics);
		yInfo("interpolateNSamples = %d", interpolateNSamples );
	}

	catch (int a) {
		yError("Error in the loading of file");
	}

	yInfo("file successfully load");
}


void AudioBayesianRatethread::normalizePropabilityMap(std::vector <std::vector <double>> &probabilityMap) {

	// Loops though the Map given as input and normalizes each column
	// This normalization is done by summing up all the elements
	// together and then dividing each element in the column by the sum
	for (int i = 0; i <  nBands; i++) {
		double sum = 0;
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			sum += probabilityMap[i][j];
		}

		for (int j = 0; j < interpolateNSamples * 2; j++) {
			probabilityMap[i][j] /= sum;
		}
	}
}


void AudioBayesianRatethread::calcOffset() {

	//to do:  redesign this to make use of the SpatialSound class which contains
	//information about altitude and azimuth so that yarpBayesianMap never needs
	//to get the position of the head directly from the robot
	if (headAngleInPort->getInputCount()) { 
		headAngleBottle = headAngleInPort->read(true);   //blocking reading for synchr with the input
		offset = headAngleBottle->get(0).asDouble();
        offset += 270;
	}
    // Pushes the current offset into a buffer which
    // is needed to remove "old" audio maps
    bufferedOffSet.push(offset);
    yInfo("offset = %f\n",offset);
}


void AudioBayesianRatethread::sendAudioMap(std::vector <std::vector <double>> &probabilityMap) {

    // Loops though the input(probabilityMap) and does
    // a deep copy of all the elements into a the outPutMatrix
    // which is used to send out the output of this module
    for (int i = 0; i < nBands; i++) {
        yarp::sig::Vector tempV(interpolateNSamples * 2);
        for (int j = 0; j < interpolateNSamples * 2; j++) {
            tempV[j] = probabilityMap[i][j];
        }
        outputMatrix->setRow(i, tempV);
    }

    outPort->setEnvelope(ts);
	outPort->write(*outputMatrix);
}

void AudioBayesianRatethread::sendProbabilityMap(std::vector <double> &outputProbabilityMap) {
    
    yarp::sig::Matrix& m = outProbabilityPort->prepare();
    m.resize(1, interpolateNSamples*2);  
     
    yarp::sig::Vector tempV(interpolateNSamples * 2);

	for (int i = 0; i < interpolateNSamples * 2; i++) {
        tempV[i] = outputProbabilityMap[i];
	}

    m.setRow(0, tempV);

	outProbabilityPort->setEnvelope(ts);
    outProbabilityPort->write();
}


void AudioBayesianRatethread::findPeaks(std::vector<double> &peakMap, const std::vector<double> &probabilityMap) {

	// Comment this out until it works
	// store the size of the map
	int mapSize = probabilityMap.size() - 1;

	// some threshold parameter. can be changed.
	double thresh = 0.1;

	// scan through the probability map looking for peaks
	for (int i = 1; i < mapSize; ++i) {

		// look for the beginning of a peak
		if (probabilityMap[i] - probabilityMap[i-1] > thresh) {

			// find the highest points of the peak
			while (probabilityMap[i] - probabilityMap[i-1] > thresh && i < mapSize) {
				i++;
			}

			// exiting the previous loop means we are either currently
			// on a peak, or went past a peak of length 1
			peakMap[i-1] = 1;

			// mark the existence of peaks until the start of a decent
			while (myABS(probabilityMap[i] - probabilityMap[i-1]) <= thresh && i < mapSize) {
				peakMap[i] = 1;
				i++;
			}

			// then loop back and look for
			// the next beginning of a peak
		}
	}

	// after itterating through, the probability map,
	// check the boundary edge case
	if (probabilityMap[mapSize] - probabilityMap[mapSize-1] > thresh &&
		probabilityMap[0] - probabilityMap[1] > thresh) {
			peakMap[mapSize] = peakMap[0] = 1;
	}
}


void AudioBayesianRatethread::setAcousticMap() {

	// This takes the input matrix and copies
	// each element into the currentAudioMap
	int count = 0;
	for (int i = 0; i < nBands; i++) {
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			currentAudioMap[i][j] = *(inputMatrix->data()+(count++));
		}
	}

	// Calls a function that normalizations
	// the columns of the currentAudioMap
	normalizePropabilityMap(currentAudioMap);

	// Calls a function to calculated the offset of the
	// current audio map based on the position of the iCub head
	calcOffset();




	/*
	//To do:  this was rewritten to use a prerecorded noise map.  remove this section if it's working
	//Checks how many iterations of the noiseMaps have been created if not enough have been created then the current audio map is used to create the noise map
	if(noiseBufferMap<=numberOfNoiseMaps) {
		createNoiseMaps();
	}

	else {
		//If enough audio maps have been combined to create the noise map a function is called to remove the ego locked noise from the current audio map
		removeNoise(currentAudioMap);

		//The current audio map is then pushed into a buffer which contains all the audioMaps needed to create the a Bayesian Map
		bufferedMap.push(currentAudioMap);

		//A function is called to create the Bayesian Map.
		createBaysianMaps();
	}
	*/




	//the noise map a function is called to remove the ego locked noise from the current audio map
	removeNoise(currentAudioMap);

	//The current audio map is then pushed into a buffer which contains all the audioMaps needed to create the a Bayesian Map
	bufferedMap.push(currentAudioMap);
	//A function is called to create the Bayesian Map.
	createBaysianMaps();


	collapseMap(longMap,longProbabilityAngleMap);

}


void AudioBayesianRatethread::addMap(std::vector <std::vector <double> > &probabilityMap, std::vector <std::vector <double> > &inputCurrentAudioMap) {

	// Loops though the input(probabilityMap) which corresponds
	// to either the long, medium or short term Bayesian maps
	// And multiplies each element in that map by the same element
	// in the provided inputCurrentAudioMap to construct and up to
	// date Bayesian map.

	// In brief this adds the map inputCurrentAudioMap to the
	// Bayesian map corresponding to the input probabilityMap
	for (int i = 0; i <  nBands; i++) {
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			int o =  myModed((j + (int)offset), interpolateNSamples * 2);
			probabilityMap[i][j] *= inputCurrentAudioMap[i][o];
		}
	}
}


void AudioBayesianRatethread::removeMap(std::vector <std::vector <double> > &probabilityMap, std::vector <std::vector <double> > &inputCurrentAudioMap) {

	// Loops though the input(probabilityMap) which corresponds
	// to either the long, medium or short term Bayesian maps
	// And divides each element in that map by the same element
	// in the provided inputCurrentAudioMap to construct and up
	// to date Bayesian map.

	// In brief this removes the map inputCurrentAudioMap from
	// the Bayesian map corresponding to the input probabilityMap
	for (int i = 0; i <  nBands; i++) {
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			int o =  myModed((j + (int)bufferedOffSet.front()), interpolateNSamples * 2);
			probabilityMap[i][j] /= inputCurrentAudioMap[i][o];
		}
	}
}


void AudioBayesianRatethread::removeNoise(std::vector <std::vector <double>> &probabilityMap) {

	// In brief this removes the noise map from the Bayesian map
	// corresponding to the input probabilityMap
	for (int i = 0; i <  nBands; i++) {
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			int o =  myModed((j + (int)offset), interpolateNSamples * 2);
			probabilityMap[i][j] /= noiseMap[i][o];
		}
	}
}


void AudioBayesianRatethread::createNoiseMaps() {

	// This function loops though the currentAudioMap
	// and adds this map to the noise map

	// If this is the last iteration of the creation
	// of the noise map it also divides it by the
	// numberOfNoiseMaps.
	// (this is done to take the average of the numberOfNoiseMaps
	// specified in that variable to currentAudioMaps to create a noise map)

	for (int i = 0; i <  nBands; i++) {
		for (int j = 0; j < interpolateNSamples * 2; j++) {
			noiseMap[i][j] += currentAudioMap[i][j];

			if(noiseBufferMap==numberOfNoiseMaps){
				noiseMap[i][j]/=numberOfNoiseMaps;
			}
		}
	}
}


void AudioBayesianRatethread::createBaysianMaps() {

	// This checks if the buffer size is larger
	// then the desired long buffer length.

	// If true this means that the "oldest" audio
	// map needs to be removed from the Bayesian Map.
	if(bufferedMap.size() >= longTimeFrame) {

		// Calls a map that will remove the "oldest"
		// audio map from the long term Bayesian Map
		removeMap(longMap,bufferedMap.front());

		// Pops both the "oldest" audio map and the
		// offset that was associated with that map.
		bufferedMap.pop();
		bufferedOffSet.pop();
	}

	// Calls a function that adds the current
	// audio map to the long term Bayesian map
	addMap(longMap,bufferedMap.back());

	// Calls a function that normalizations
	// the columns of the long Bayesian Map
	normalizePropabilityMap(longMap);
}


std::vector <std::vector <double> > AudioBayesianRatethread::getLongProbabilityMap() {
	return longMap;
}


std::vector <std::vector <double> > AudioBayesianRatethread::getMediumProbabilityMap() {
	return mediumMap;
}


std::vector <std::vector <double> > AudioBayesianRatethread::getShortProbabilityMap() {
	return shortMap;
}


void AudioBayesianRatethread::collapseMap(const std::vector <std::vector <double>> &inputMap, std::vector <double> &outputProbabilityMap) {

	for (int j = 0; j < interpolateNSamples * 2; j++) {
		for (int i = 0; i <  nBands; i++) {
			//TODO This is extremely slow however I could not think of a better way to complete it.
			outputProbabilityMap[j]+=inputMap[i][j];
		}
	}

	double sum = 0;
	for (int j = 0; j < interpolateNSamples * 2; j++) {
		sum+=outputProbabilityMap[j];
	}

	for (int j = 0; j < interpolateNSamples * 2; j++) {
		outputProbabilityMap[j] /= sum;
	}
}
