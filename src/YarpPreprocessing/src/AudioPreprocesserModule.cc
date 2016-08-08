#include "AudioPreprocesserModule.h"

AudioPreprocesserModule::AudioPreprocesserModule()
{
	myerror = "\033[0;31m";
	myinfo = "\033[0;32m";
	mywarn = "\033[0;33m";
	myreset = "\033[0m";

	gammatonAudioFilter = new GammatonFilter();
	beamForm = new BeamFormer();

	//TODO how should this path be better protected
	fileName = "../../src/YarpPreprocessing/src/loadFile.xml";
	loadFile();

	rawAudio = new float[(frameSamples * nMics)];
	oldtime = 0;

	for (int i = 0; i < interpellateNSamples; i++) {
		std::vector<double> tempvector;
		for (int j = 0; j < nBands; j++) {
			double tempnumber = 0;
			tempvector.push_back(tempnumber);
		}
		highResolutionAudioMap.push_back(tempvector);
	}

	createMemoryMappedFile();
}

AudioPreprocesserModule::~AudioPreprocesserModule()
{
	delete gammatonAudioFilter;
	delete beamForm;
	delete rawAudio;
}

bool AudioPreprocesserModule::configure(ResourceFinder &rf)
{
	inPort = new BufferedPort<yarp::sig::Sound>();
	inPort->open("/iCubAudioAttention/Audio:i");

	//TODO this show not be done here

	if (yarp::os::Network::exists("/iCubAudioAttention/Audio:i"))
	{
		if (yarp::os::Network::connect("/sender","/iCubAudioAttention/Audio:i")==false)
		{
		    std::cout << myerror << "[ERROR]" << myreset << " Could not make connection to /sender. Exiting.\n";
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

double AudioPreprocesserModule::getPeriod()
{

	// TODO Should this all ways stay this low
	return 0.05;
}

bool AudioPreprocesserModule::updateModule()
{

	s = inPort->read(true);
	gettimeofday(&st, NULL);
	inPort->getEnvelope(ts);

	//TODO Remove this for speed
	printf("[INFO] Count:%d Time:%f \n", ts.getCount(), ts.getTime() - oldtime);
	oldtime = ts.getTime();
	if (ts.getCount() != lastframe + 1) {
		printf("[WARN] Too Slow\n");
	}

	int row = 0;
	for (int col = 0 ; col < frameSamples; col++) {
		NetInt16 temp_c = (NetInt16) s->get(col, 0);
		NetInt16 temp_d = (NetInt16) s->get(col, 1);
		rawAudio[row]   = (float) temp_c / normDivid;
		rawAudio[row + 1]	= (float) temp_d / normDivid;
		row += 2;

	}

	gammatonAudioFilter->inputAudio(rawAudio);
	beamForm->inputAudio(gammatonAudioFilter->getFilteredAudio());
	reducedBeamFormedAudioVector = beamForm->getReducedBeamAudio();

	spineInterp();
	memoryMapper();


	//TODO Change timing to use Yarp timing
	//Timing how long the module took
	lastframe = ts.getCount();
	gettimeofday(&en, NULL);
	seconds  = en.tv_sec  - st.tv_sec;
	useconds = en.tv_usec - st.tv_usec;
	mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;
	printf("Elapsed time: %ld milliseconds\n", mtime);
	return true;
}

bool AudioPreprocesserModule::interruptModule()
{
	fprintf(stderr, "[WARN] Interrupting\n");
	return true;
}

bool AudioPreprocesserModule::close()
{
	fprintf(stderr, "[INFO] Calling close\n");
	return true;
}

void AudioPreprocesserModule::createMemoryMappedFile()
{
	int memoryMapSize = ((nBands * interpellateNSamples)*2 + 2);

	double initializationArray [memoryMapSize];
	fid = fopen("/tmp/preprocessedAudioMap.tmp", "w");
	fwrite(initializationArray, sizeof(double), sizeof(initializationArray), fid);
	fclose(fid);
	mappedFileID = open("/tmp/preprocessedAudioMap.tmp", O_RDWR);
	std::cout << sizeof(initializationArray) << std::endl;
	mappedAudioData = (double *)mmap(0, (sizeof(initializationArray)), PROT_WRITE, MAP_SHARED , mappedFileID, 0);
}

void AudioPreprocesserModule::loadFile()
{

	ConfigParser *confPars;
	confPars = ConfigParser::getInstance(fileName);
	Config pars = (confPars->getConfig("default"));

	frameSamples = pars.getFrameSamples();
	nBands = pars.getNBands();
	nMics = pars.getNMics();

	interpellateNSamples = pars.getInterpellateNSamples();

	//TODO make sure this calculates based on the data in xml
	totalBeams = 39;


}

void AudioPreprocesserModule::memoryMapper()
{
	mappedAudioData[0] = ts.getCount();
	mappedAudioData[1] = ts.getTime();
	int count = 0;

	for (int i = 0; i < 180; i++)
	{
		for (int j = 0; j < nBands; j++)
		{
			mappedAudioData[(count++) + 2] = highResolutionAudioMap[i][j];
		}
	}
	for (int i = 179; i > 1; i--)
	{
		for (int j = 0; j < nBands; j++)
		{
			mappedAudioData[(count++) + 2] = highResolutionAudioMap[i][j];
		}
	}

}

void AudioPreprocesserModule::spineInterp()
{
	double offset = (interpellateNSamples / (double)totalBeams);

	for (int i = 0; i < nBands; i++)
	{
		int k = 0;
		double curroffset = offset;

		for (int j = 0; j < interpellateNSamples; j++)
		{
			if(j==(int)curroffset && k<(totalBeams-2))
			{
				highResolutionAudioMap[j][i] = reducedBeamFormedAudioVector[k++][i];
				curroffset += offset;
			}
			else highResolutionAudioMap[j][i] = interpl(j, curroffset, curroffset+offset, reducedBeamFormedAudioVector[k][i], reducedBeamFormedAudioVector[k+1][i]);
			if(j+1==interpellateNSamples){
				 highResolutionAudioMap[j][i] =  reducedBeamFormedAudioVector[++k][i];
			}

		}

	}


}

double AudioPreprocesserModule::interpl(int x, int x1, int x2, double y1, double y2)
{

  return y1 + ((y2-y1)/(x2-x1))*(x-x1);

}
