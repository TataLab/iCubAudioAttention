#include "BayesianModule.h"
#include <iostream>
int myModed(int a, int b) {
    return  a >= 0 ? a % b : (a % b) + b;
}

BayesianModule::BayesianModule()
{
    //TODO what is the best way to do this (Where is the best place to do this)
    this->fileName = "../../src/Configuration/loadFile.xml";
    loadFile();

    for (int i = 0; i < nBands; i++)
    {
        std::vector<double> tempvector;
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            tempvector.push_back(1.0);
        }
        currentAudioMap.push_back(tempvector);
    }

    for (int i = 0; i < nBands; i++)
    {
        std::vector<double> tempvector;
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            tempvector.push_back(1.0);
        }
        probabilityMap.push_back(tempvector);
    }

    options.put("device", "remote_controlboard");
    options.put("local", "/AudioFocusAttention");
    options.put("remote", "/icub/head");
    robotHead = new yarp::dev::PolyDriver(options);
    robotHead->view(enc);

    if (enc == NULL) {
        printf("Cannot get interface to robot head\n");
        robotHead->close();
        return;
    }

    inputMatrix = new yarp::sig::Matrix(nBands, interpellateNSamples * 2);
    outputMatrix = new yarp::sig::Matrix(nBands, interpellateNSamples * 2);

    createMemoryMappedFile();

}

BayesianModule::~BayesianModule()
{

}

//normalize numbers to 0 - 1
void BayesianModule::normalizeCurrentAudioMap()
{
    for (int i = 0; i <  nBands; i++)
    {
        double sum = 0;
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            sum += currentAudioMap[i][j];
        }
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            currentAudioMap[i][j] /= sum;
        }
    }

}

bool BayesianModule::configure(yarp::os::ResourceFinder &rf)
{
    inPort = new yarp::os::BufferedPort<yarp::sig::Matrix>();
    inPort->open("/iCubAudioAttention/BayesianMap:i");

    outPort = new yarp::os::Port();
    outPort->open("/iCubAudioAttention/BayesianMap:o");

    if (yarp::os::Network::exists("/iCubAudioAttention/BayesianMap:i"))
    {
        if (yarp::os::Network::connect( "/iCubAudioAttention/Preprocesser:o", "/iCubAudioAttention/BayesianMap:i") == false)
        {
            std::cout <<"[ERROR]" <<" Could not make connection to /sender. Exiting.\n";
            return false;
        }
    }
    return true;
}

double BayesianModule::getPeriod()
{
    // TODO Should this all ways stay this low
    return 0.05;
}

bool BayesianModule::updateModule()
{


    inputMatrix = inPort->read(true);
    inPort->getEnvelope(ts);
    gettimeofday(&st, NULL);

    setAcousticMap();
    memoryMapper();
    outPort->setEnvelope(ts);
    outPort->write(*outputMatrix);

    gettimeofday(&en, NULL);
    seconds  = en.tv_sec  - st.tv_sec;
    useconds = en.tv_usec - st.tv_usec;
    mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;
    printf("[INFO] Count:%d Time:%ld milliseconds. \n", ts.getCount(),  mtime);

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

void BayesianModule::normalizePropabilityMap()
{
    for (int i = 0; i <  nBands; i++)
    {
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            int o =  myModed((j + (int)offset), interpellateNSamples * 2);
            probabilityMap[i][j] *= currentAudioMap[i][o];
        }
    }

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
    //TODO Figure out how to calculate this value for the red iCub
    enc->getEncoder(0, &offset);
    offset+=90;
}

void BayesianModule::createMemoryMappedFile()
{
    int memoryMapSize = ((nBands * (interpellateNSamples + 1)) * 2 );
    double initializationArray [memoryMapSize];
    fid = fopen("/tmp/bayesianProbabilityMap.tmp", "w");
    fwrite(initializationArray, sizeof(double), sizeof(initializationArray), fid);
    fclose(fid);
    mappingFileID = open("/tmp/bayesianProbabilityMap.tmp", O_RDWR);
    probabilityMapping = (double *)mmap(0, (sizeof(initializationArray)), PROT_WRITE, MAP_SHARED , mappingFileID, 0);
}

void BayesianModule::memoryMapper()
{
    int count = 0;
    for (int i = 0; i <  nBands; i++)
    {
        for (int j = 0; j <interpellateNSamples * 2; j++)
        {
            probabilityMapping[(count++)] = probabilityMap[i][j];
        }
    }
}

void BayesianModule::sendAudioMap()
{
    for (int i = 0; i < nBands; i++)
    {
        yarp::sig::Vector tempV(interpellateNSamples * 2);
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            tempV[j] = probabilityMap[j][i];
        }
        outputMatrix->setRow(i, tempV);
    }
}

void BayesianModule::setAcousticMap()
{
    int count = 0;
    for (int i = 0; i < nBands; i++)
    {
        for (int j = 0; j < interpellateNSamples * 2; j++)
        {
            currentAudioMap[i][j] = *(inputMatrix->data()+(count++));
        }
    }
    normalizeCurrentAudioMap();
    calcOffset();
    normalizePropabilityMap();
}

std::vector <std::vector <double>> BayesianModule::getProbabilityMap()
{
    return probabilityMap;
}

void BayesianModule::loadFile()
{
    ConfigParser *confPars;
    confPars = ConfigParser::getInstance(fileName);
    Config pars = (confPars->getConfig("default"));
    nBands = pars.getNBands();
    interpellateNSamples = pars.getInterpellateNSamples();

}
