// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C)2017  Department of Neuroscience - University of Lethbridge
  * Author:Matt Tata, Marko Ilievski
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


#include "BayesianModule.h"
#include <iostream>

//TO DO:  get this into resource finder
#define MEMORY_MAP_SIZE nBands * 360 

int myModed(int a, int b) {
    return  a >= 0 ? a % b : (a % b) + b;
}

double myABS(double a) {
    return  a >= 0 ? a : ((a) * -1);
}

BayesianModule::BayesianModule()
{


    //Set the file which the module uses to grab the config information
    this->fileName = "../../src/Configuration/loadFile.xml";
    //calls the parser and the config file to configure the needed variables in this class
    loadFile();

    //Allocating the required vectors for the modules to function properly  
    for (int i = 0; i < nBands; i++)
    {   
        std::vector<double> tempvector;
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            tempvector.push_back(1.0);
        }
        //Vector containing the most recent audio map that was sent to this module though yarp
        currentAudioMap.push_back(tempvector);

        //The noise map that was created, representing the current ego locked noise in the environment
        noiseMap.push_back(tempvector);

        //A short Bayesian map term map
        shortMap.push_back(tempvector);
        //A medium term Bayesian map term map
        mediumMap.push_back(tempvector);
        //A long term Bayesian map term map
        longMap.push_back(tempvector);
    }
   
   	longProbabilityAngleMap.assign(interpellateNSamples * 2,0);

    //Allocates the required memory for the yarp matrix that takes the input and output to this module
    inputMatrix = new yarp::sig::Matrix(nBands, interpellateNSamples * 2);
    outputMatrix = new yarp::sig::Matrix(nBands, interpellateNSamples * 2);

    //Calls the function that initalizes the the data required for memory mapping the
    //short, medium and long term Bayesian maps as needed
    createMemoryMappedFile();

    //Initializes the variable that keeps track of how many frames have been gathered for the noise map
    noiseBufferMap = 0;
    first = true;

	
    
}

BayesianModule::~BayesianModule()
{

}

//This sets up the needed input and output ports needed by this module
bool BayesianModule::configure(yarp::os::ResourceFinder &rf)
{
    printf(" C O N F I G U R I N G\n");
    robotName = rf.check("robot", Value("icub"),"Robot name (string)").asString();
    printf("name of robot is %s\n",robotName.c_str());
    
    std::string headPort = "/" + robotName + "/head";
    //initialize the options with the icub
    options.put("device", "remote_controlboard");
    options.put("local", "/AudioFocusAttention");
    options.put("remote", headPort.c_str());
    //options.put("remote", "/icub/head");
    
    //loads the robot head module with the options configured above
    robotHead = new yarp::dev::PolyDriver(options);
    robotHead->view(enc);
    robotHead->view(pos);
    //Checks that the robot head was properly initialized and assigned to both eco(encoder data) and pos(position data)
    if (enc == NULL || pos == NULL) {
        printf("Cannot get interface to robot head\n");
        robotHead->close();
        return false;
    }
    
    
    //Creates a buffered yarp port as input into this module with the name /iCubAudioAttention/BayesianMap:i
    //The expected input into this module is a yarp matrix with the size of (beams * bands) 
    //The matrix is created in the yarpPreprocessing module
    inPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    inPort->open("/iCubAudioAttention/BayesianMap:i");

    //Creates a yarp port as output from this module with the name /iCubAudioAttention/BayesianMap:o
    //The output of this module is a yarp matrix with size of (beams * bands)
    //The output corresponds to only the long term audio map
    outPort = new yarp::os::Port();
    outPort->open("/iCubAudioAttention/BayesianMap:o");
    //TODO add output port for the medium and short term maps

    //TODO I believe that this connection should not be established here
    //Checks if the output of the YarpPreProcessing module exists if it does it creates a connection between the two modules, if it does not this function will return an error
    if (yarp::os::Network::exists("/iCubAudioAttention/Preprocesser:o"))
    {
        if (yarp::os::Network::connect( "/iCubAudioAttention/Preprocesser:o", "/iCubAudioAttention/BayesianMap:i") == false)
        {
            std::cout <<"[ERROR]" <<" Could not make connection to /sender. Exiting.\n";
            return false;
        }
    }
        
    if (rf.check("audioConfig")) {
        configFile=rf.findFile(rf.find("audioConfig").asString().c_str());
        if (configFile=="") {
            return false;
        }
    }
    else {
        configFile.clear();
    }
        
    //get the noise map prior that was pre-recorded or bail out
    FILE *fidNoise = fopen("./noiseMap.dat", "r"); //open the previously recorded noiseMap prior
	if(fidNoise==NULL){
		printf("\n\nYou need to pre-record a noise map prior\n\n");
		return false;
	}

	double tempNoiseMap[nBands*interpellateNSamples*2];
	fread(tempNoiseMap,sizeof(double),nBands*interpellateNSamples*2,fidNoise);
	fclose(fidNoise);
	
	int count=0;
	for(int i=0;i<nBands;i++){
	
		std::vector<double> tempvector;
		for(int j=0;j<interpellateNSamples*2;j++){
			tempvector.push_back(tempNoiseMap[count++]);
		}
		noiseMap.push_back(tempvector);
	}
		
	
	
    //numberOfNoiseMaps = 100;
    
    
    return true;
}

double BayesianModule::getPeriod()
{
    // TODO Should this all ways stay this low
    return 0.05;
}

//Does all the processing for this module
bool BayesianModule::updateModule()
{
    //Reads the a matrix from the input port
    //This is a blocking call thus the module will wait until it has acquired the required matrix
    inputMatrix = inPort->read(true);

    //Gathers the time/counter envelope that was associated with the last message
    inPort->getEnvelope(ts);
    //Asks linux for the exact time of day currently and saves it in the st(start) variable
    gettimeofday(&st, NULL);

    //Calls a function that will take the current audio map and create the Bayesian maps
    setAcousticMap();

    //copies the data in the longMap vector into the outputMatrix and sends the matrix along with the envelope via the output Port 
    sendAudioMap(longMap);

    //Calls the Memory maper and memory maps it to the following file: /tmp/bayesianProbabilityLongMap.tmp
    memoryMapper(longMap,probabilityMappingLong);
    outPort->setEnvelope(ts);
    outPort->write(*outputMatrix);

    //Asks linux for the exact time of day currently and saves it in the en(end) variable
    gettimeofday(&en, NULL);
    //Calculates the difference between the start and end times of this module to figure out exactly how long this module took to run
    seconds  = en.tv_sec  - st.tv_sec;
    useconds = en.tv_usec - st.tv_usec;
    mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;
    printf("[INFO] Count:%d Time:%ld milliseconds. \n", ts.getCount(),  mtime);
    return true;
}

bool BayesianModule::interruptModule()
{
    fprintf(stderr, "[INFO] Calling close\n");
    return true;
}

bool BayesianModule::close()
{
    fprintf(stderr, "[INFO] Calling close\n");
    return true;
}

void BayesianModule::normalizePropabilityMap(std::vector <std::vector <double>> &probabilityMap)
{
    //Loops though the Map given as input and normalizes each column
    //This normalization is done by summing up all the elements together and then dividing each element in the column by the sum
    for (int i = 0; i <  nBands; i++)
    {
        double sum = 0;
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            sum += probabilityMap[i][j];
        }
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            probabilityMap[i][j] /= sum;
        }
    }

}

void BayesianModule::calcOffset()
{

	//to do:  redesign this to make use of the SpatialSound class which contains information about altitude and azimuth so that yarpBayesianMap never needs to get the 
	//position of the head directly from the robot



    //TODO Figure out how to calculate this value for the red iCub
    //Checks the current position of joint 0 of the head and stores it in the offset variable
    enc->getEncoder(0, &offset);
    offset+=270;
    //Pushes the current offset into a buffer which is needed to remove "old" audio maps
    bufferedOffSet.push(offset);
    printf("offset = %f\n",offset);
    
}

void BayesianModule::createMemoryMappedFile()
{
    //int memoryMapSize = ((nBands * (interpellateNSamples + 1)) * 2 )+1;
    
    double initializationArrayShort [MEMORY_MAP_SIZE];
    double initializationArrayMedium [MEMORY_MAP_SIZE];
    double initializationArrayLong[MEMORY_MAP_SIZE];
    
    printf("MEMORY_MAP_SIZE is %d",MEMORY_MAP_SIZE);
    fidShort = fopen("/tmp/bayesianProbabilityShortMap.tmp", "w");
    fwrite(initializationArrayShort, sizeof(double), sizeof(initializationArrayShort), fidShort);
    fclose(fidShort);
    fidMedium = fopen("/tmp/bayesianProbabilityMediumMap.tmp", "w");
    fwrite(initializationArrayMedium, sizeof(double), sizeof(initializationArrayMedium), fidMedium);
    fclose(fidMedium);
    fidLong = fopen("/tmp/bayesianProbabilityLongMap.tmp", "w");
    fwrite(initializationArrayLong, sizeof(double), sizeof(initializationArrayLong), fidLong);
    fclose(fidLong);
    mappingFileIDShort = open("/tmp/bayesianProbabilityShortMap.tmp", O_RDWR);
    probabilityMappingShort= (double *)mmap(0, (sizeof(initializationArrayShort)), PROT_WRITE, MAP_SHARED , mappingFileIDShort, 0);
    mappingFileIDMedium = open("/tmp/bayesianProbabilityMediumMap.tmp", O_RDWR);
    probabilityMappingMedium= (double *)mmap(0, (sizeof(initializationArrayMedium)), PROT_WRITE, MAP_SHARED , mappingFileIDMedium, 0);
    mappingFileIDLong = open("/tmp/bayesianProbabilityLongMap.tmp", O_RDWR);
    probabilityMappingLong= (double *)mmap(0, (sizeof(initializationArrayLong)), PROT_WRITE, MAP_SHARED , mappingFileIDLong, 0);


    double initializationLongProbabilityAngleMapArray[(interpellateNSamples+1) * 2];
    fidLongProbabilityAngle = fopen("/tmp/preprocessedGammaToneFilteredAudio.tmp", "w");
    fwrite(initializationLongProbabilityAngleMapArray, sizeof(double), sizeof(initializationLongProbabilityAngleMapArray), fidLongProbabilityAngle);
    fclose(fidLongProbabilityAngle);
    mappingFileIDLongProbabilityAngle = open("/tmp/preprocessedGammaToneFilteredAudio.tmp", O_RDWR);
    probabilityMappingLongProbabilityAngle = (double *)mmap(0, (sizeof(initializationLongProbabilityAngleMapArray)), PROT_WRITE, MAP_SHARED , mappingFileIDLongProbabilityAngle, 0);
}

void BayesianModule::memoryMapper(std::vector <std::vector <double>> probabilityMap, double* probabilityMappingFileID)
{
    //Loops though the input(probabilityMap) and does a deep copy of all the elements into a memory mapped location identified by the probabilityMappingFileID
    int count = 0;
    for (int i = 0; i <  nBands; i++)
    {
        for (int j = 0; j <interpellateNSamples * 2; j++)
        {
        	//printf("count = %d\n",count);

            probabilityMappingFileID[(count++)] = probabilityMap[i][j];
        }
    }
    //probabilityMappingFileID[(count++)] = offset;
}

void BayesianModule::sendAudioMap(std::vector <std::vector <double>> &probabilityMap)
{
    //Loops though the input(probabilityMap) and does a deep copy of all the elements into a the outPutMatrix which is used to send out the output of this module
    for (int i = 0; i < nBands; i++)
    {
        yarp::sig::Vector tempV(interpellateNSamples * 2);
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            tempV[j] = probabilityMap[i][j];
        }
        outputMatrix->setRow(i, tempV);
    }
}

void findPeaks(std::vector<double> &peakMap, const std::vector<double> &probabilityMap)
{
    //Comment this out until it works
    // store the size of the map
    int mapSize = probabilityMap.size() - 1;

    // some threshold parameter. can be changed.
    double thresh = 0.1;

    // scan through the probability map looking for peaks
    for (int i = 1; i < mapSize; ++i)
    {
        // look for the beginning of a peak
        if (probabilityMap[i] - probabilityMap[i-1] > thresh)
        {
            // find the highest points of the peak
            while (probabilityMap[i] - probabilityMap[i-1] > thresh && i < mapSize) i++;

            // exiting the previous loop means we are either currently 
            // on a peak, or went past a peak of length 1
            peakMap[i-1] = 1;

            // mark the existence of peaks until the start of a decent
            while (myABS(probabilityMap[i] - probabilityMap[i-1]) <= thresh && i < mapSize)
            {
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
        probabilityMap[0] - probabilityMap[1] > thresh)
        peakMap[mapSize] = peakMap[0] = 1;
        
}

void BayesianModule::setAcousticMap()
{
    //This takes the input matrix and copies each element into the currentAudioMap
    int count = 0;
    for (int i = 0; i < nBands; i++)
    {
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            currentAudioMap[i][j] = *(inputMatrix->data()+(count++));
        }
    }
    //Calls a function that normalizations the columns of the currentAudioMap
    normalizePropabilityMap(currentAudioMap);   
    //Calls a function to calculated the offset of the current audio map based on the position of the iCub head
    calcOffset();

	/*To do:  this was rewritten to use a prerecorded noise map.  remove this section if it's working
    //Checks how many iterations of the noiseMaps have been created if not enough have been created then the current audio map is used to create the noise map
    if(noiseBufferMap<=numberOfNoiseMaps)
        createNoiseMaps();
    else{
        //If enough audio maps have been combined to create the noise map a function is called to remove the ego locked noise from the current audio map
        removeNoise(currentAudioMap);
        //The current audio map is then pushed into a buffer which contains all the audioMaps needed to create the a Bayesian Map
        bufferedMap.push(currentAudioMap);
        //A function is called to create the Bayesian Map. 
        createBaysianMaps();
    }*/
    
    //the noise map a function is called to remove the ego locked noise from the current audio map
    removeNoise(currentAudioMap);

    //The current audio map is then pushed into a buffer which contains all the audioMaps needed to create the a Bayesian Map
	bufferedMap.push(currentAudioMap);
    //A function is called to create the Bayesian Map. 
    createBaysianMaps();


    collapseMap(longMap,longProbabilityAngleMap);
    memoryProbabilityAngleMap(longProbabilityAngleMap);
    //noiseBufferMap++;
}

void BayesianModule::addMap(std::vector <std::vector <double>> &probabilityMap, std::vector <std::vector <double>> &inputCurrentAudioMap)
{   
    //Loops though the input(probabilityMap) which corresponds to either the long, medium or short term Bayesian maps
    //And multiplies each element in that map by the same element in the provided inputCurrentAudioMap to construct and up to date Bayesian map
    //In brief this adds the map inputCurrentAudioMap to the Bayesian map corresponding to the input probabilityMap
    for (int i = 0; i <  nBands; i++)
    {
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            int o =  myModed((j + (int)offset), interpellateNSamples * 2);
            probabilityMap[i][j] *= inputCurrentAudioMap[i][o];   
        }
    }
}
void BayesianModule::removeMap(std::vector <std::vector <double>> &probabilityMap, std::vector <std::vector <double>> &inputCurrentAudioMap)
{   
    //Loops though the input(probabilityMap) which corresponds to either the long, medium or short term Bayesian maps
    //And divides each element in that map by the same element in the provided inputCurrentAudioMap to construct and up to date Bayesian map
    //In brief this removes the map inputCurrentAudioMap from the Bayesian map corresponding to the input probabilityMap
    for (int i = 0; i <  nBands; i++)
    {
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            int o =  myModed((j + (int)bufferedOffSet.front()), interpellateNSamples * 2);
            probabilityMap[i][j] /= inputCurrentAudioMap[i][o];
        }
    }
}
void BayesianModule::removeNoise(std::vector <std::vector <double>> &probabilityMap)
{   
    //In brief this removes the noise map from the Bayesian map corresponding to the input probabilityMap
    for (int i = 0; i <  nBands; i++)
    {
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            int o =  myModed((j + (int)offset), interpellateNSamples * 2);
            probabilityMap[i][j] /= noiseMap[i][o];
        }
    }
}
void BayesianModule::createNoiseMaps()
{   
    //This function loops though the currentAudioMap and adds this map to the noise map
    //If this is the last iteration of the creation of the noise map it also divides it by the numberOfNoiseMaps 
    //(this is done to take the average of the numberOfNoiseMaps specified in that variable to currentAudioMaps to create a noise map)
    for (int i = 0; i <  nBands; i++)
    {
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            noiseMap[i][j] += currentAudioMap[i][j];
            if(noiseBufferMap==numberOfNoiseMaps){
               noiseMap[i][j]/=numberOfNoiseMaps;
            }         
        }
    }
}
void BayesianModule::createBaysianMaps()
{   
    //This checks if the buffer size is larger then the desired long buffer length.
    //If true this means that the "oldest" audio map needs to be removed from the Bayesian Map.
    if(bufferedMap.size()>=longTimeFrame)
    {
        //Calls a map that will remove the "oldest" audio map from the long term Bayesian Map
        removeMap(longMap,bufferedMap.front());

        //Pops both the "oldest" audio map and the offset that was associated with that map.
        bufferedMap.pop();
        bufferedOffSet.pop();
    }
    //Calls a function that adds the current audio map to the long term Bayesian map
    addMap(longMap,bufferedMap.back());
    //Calls a function that normalizations the columns of the long Bayesian Map
    normalizePropabilityMap(longMap);
}

std::vector <std::vector <double>> BayesianModule::getLongProbabilityMap()
{
    return longMap;
}

std::vector <std::vector <double>> BayesianModule::getMediumProbabilityMap()
{
    return mediumMap;
}

std::vector <std::vector <double>> BayesianModule::getShortProbabilityMap()
{
    return shortMap;
}

void BayesianModule::loadFile()
{
    yarp::os::ResourceFinder rf;
    int argc;
    char** argv;
    yInfo("Resource Finder looks into %s", configFile.c_str());
    rf.setDefaultConfigFile("../../app/iCubAudioAttention/conf/audioConfig.ini");
    rf.setDefaultContext("iCubAudioAttention/conf");
    rf.setVerbose(true);
    rf.configure(argc, argv);

    int _nBands  = rf.check("nBands", 
                           Value("128"), 
                           "numberBands (int)").asInt();

    int _interpellateNSamples  = rf.check("interpellateNSamples", 
                           Value("180"), 
                           "interpellate N samples (int)").asInt();

    int _shortTimeFrame = rf.check("shortBufferSize", 
                           Value("10"), 
                           "short Buffer Size (int)").asInt();

    int _mediumTimeFrame = rf.check("mediumBufferSize", 
                           Value("100"), 
                           "medium Buffer Size (int)").asInt();

    int _longTimeFrame = rf.check("longBufferSize", 
                           Value("360"), 
                           "long Buffer Size (int)").asInt();

    //Creates or if previously created used the create Configuration Parser, this is used to read the config.xml file
    ConfigParser *confPars;
    //Give the parser the path to the .xml file
    confPars = ConfigParser::getInstance(fileName);
    //Selects the default setting to be used from the give .xml file
    Config pars = (confPars->getConfig("default"));
    //Grabs the number of bands
    nBands = pars.getNBands();
    //Grabs the number of interpolated samples
    interpellateNSamples = pars.getInterpellateNSamples();

    //The time frames of different Buffers
    shortTimeFrame = pars.getShortBufferSize();
    mediumTimeFrame = pars.getMediumBufferSize();
    longTimeFrame = pars.getLongBufferSize();


}

//TODO Check that this is working
void BayesianModule::collapseMap(const std::vector <std::vector <double>> &inputMap, std::vector <double> &outputProbabilityMap)
{
    for (int j = 0; j < interpellateNSamples * 2; j++)
    {

        for (int i = 0; i <  nBands; i++)
        {
            //TODO This is extremely slow however I could not think of a better 
            outputProbabilityMap[j]+=inputMap[i][j];
        }

    }
    double sum = 0;
    for (int j = 0; j < interpellateNSamples * 2; j++)
    {
        sum+=outputProbabilityMap[j];
    }
    for (int j = 0; j < interpellateNSamples * 2; j++)
    {
        outputProbabilityMap[j]/=sum;
    }
}

void BayesianModule::memoryProbabilityAngleMap(std::vector <double> probabilityMap)
{
    for (int i = 0; i < interpellateNSamples * 2; i++)
    {
        probabilityMappingLongProbabilityAngle[i]+=probabilityMap[i];
    }
}
