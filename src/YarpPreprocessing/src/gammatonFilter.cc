// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include "gammatonFilter.h"

#define erb(x)         ( 24.7 * ( 4.37e-3 * ( x ) + 1.0 ) )

GammatonFilter::GammatonFilter(std::string file)
{
	fileName = file;
	loadFile();

	makeErbCFs();

	for (int i = 0; i < nBands * nMics; i++)
	{
		float *temp = new float[frameSamples];
		filteredAudio.push_back(temp);
	}

	P = new float [8 * nBands];

}

GammatonFilter::~GammatonFilter()
{

}

void GammatonFilter::inputAudio(float* inAudio)
{
	inputSignal = inAudio;
	generatFilter(0);
	generatFilter(1);
}

void GammatonFilter::generatFilter(int MicNumber)
{

	double u0r = 0;
	double u0i = 0;
	double oldcs = 0;
	double oldphase = 0.0;
	double tpt = (M_PI + M_PI) / samplingRate;
	for (int ch = 0; ch < nBands; ch++)
	{

		oldphase = 0.0;

		p0r = 0, p1r = 0, p2r = 0, p3r = 0, p4r = 0, p0i = 0, p1i = 0, p2i = 0, p3i = 0, p4i = 0;

		double tptbw = tpt * erb(cfs[ch]) * BW_CORRECTION;
		double a = exp(-tptbw);
		double gain = (tptbw * tptbw * tptbw * tptbw) / 3;

		double a1 = 4.0 * a;
		double a2 = -6.0 * a * a;
		double a3 = 4.0 * a * a * a;
		double a4 = -a * a * a * a;
		double a5 = a * a;

		double coscf = cos(tpt * cfs[ch]);
		double sincf = sin(tpt * cfs[ch]);
		double qcos = 1;
		double qsin = 0;

		for (int t = 0; t < (frameSamples * nMics); t++)
		{
			if (t % 2 != MicNumber)
				continue;
			float xx = inputSignal[t];

			p0r = qcos * xx + a1 * p1r + a2 * p2r + a3 * p3r + a4 * p4r;
			p0i = qsin * xx + a1 * p1i + a2 * p2i + a3 * p3i + a4 * p4i;

			if (fabs(p0r) < VERY_SMALL_NUMBER)
				p0r = 0.0F;
			if (fabs(p0i) < VERY_SMALL_NUMBER)
				p0i = 0.0F;

			u0r = p0r + a1 * p1r + a5 * p2r;
			u0i = p0i + a1 * p1i + a5 * p2i;

			p4r = p3r; p3r = p2r; p2r = p1r; p1r = p0r;
			p4i = p3i; p3i = p2i; p2i = p1i; p1i = p0i;


			filteredAudio[ch + (MicNumber * nBands)][t / 2] = (u0r * qcos + u0i * qsin) * gain;
			if (hrect && filteredAudio[ch + (MicNumber * nBands)][t / 2] < 0) {
				filteredAudio[ch + (MicNumber * nBands)][t / 2] = 0;
			}

			qcos = coscf * (oldcs = qcos) + sincf * qsin;
			qsin = coscf * qsin - sincf * oldcs;
		}
	}
}

void GammatonFilter::loadFile()
{

	ConfigParser *confPars;
	try {
		confPars = ConfigParser::getInstance(fileName);
		Config pars = (confPars->getConfig("default"));

		samplingRate = pars.getSamplingRate();
		lowerCF = pars.getLowCf();
		higherCF = pars.getHighCf();
		nBands = pars.getNBands();
		frameSamples = pars.getFrameSamples();
		nMics = pars.getNMics();
		align = false;
		hrect = false;
	}
	catch (int a) {

	}


}

void GammatonFilter::makeErbCFs()
{
	double minERB = HzToErbRate(lowerCF);
	double highERB = HzToErbRate(higherCF);
	double incAmount = (highERB - minERB) / (nBands - 1);

	for (int i = 0; i < nBands; i++) {
		cfs.push_back(ErbRateToHz(incAmount * i + minERB));
	}

}

double GammatonFilter::HzToErbRate(double Hz)
{
	return (21.4 * log10(0.00437 * Hz + 1));
}
double GammatonFilter::ErbRateToHz(double Erb)
{
	return (pow(10, Erb / 21.4) - 1) / 0.00437;
}

std::vector< float* > GammatonFilter::getFilteredAudio()
{
	return filteredAudio;
}
