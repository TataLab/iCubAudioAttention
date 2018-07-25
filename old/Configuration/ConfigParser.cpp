// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include "ConfigParser.h"
#include <regex>
#include <iostream>
#include <fstream>

ConfigParser* ConfigParser::instance;

ConfigParser* ConfigParser::getInstance(std::string file)
{
	if (!instance)
	{
		std::cout << "[INFO] Parsing file." << std::endl;
		instance = new ConfigParser(file);
	}
	return instance;
}

ConfigParser::ConfigParser(std::string file)
{
	std::cout << "[INFO] Loading Config File\n";

	//configList
	std::ifstream myfile (file);

	std::string s;

	configList = new std::vector<Config>();

	if (myfile.is_open())
	{
		getline (myfile, s);
		s = std::regex_replace(s, std::regex("^ +| +$|( ) +"), "$1");
		std::regex headTag("<\\?xml(?: [a-zA-Z_:][a-zA-Z0-9\\._:-]*(?: )?=(?: )?\"[^\"]+\")*\\?>");
		if (std::regex_match (s, headTag)) std::cout << "Valid Identifier.\n";

		std::regex mainTag("(^[:whitespace:])*<[a-zA-Z][a-zA-Z0-9\\._:=]*(?: [a-zA-Z_:][a-zA-Z0-9\\._:-]*(?: )?=(?: )?\"[^\"]+\")*>([:whitespace:])*$");

		while (getline(myfile, s))
		{
			if (std::regex_match (s, mainTag)) process(s, myfile);
			else
			{
				std::cout << "[ERROR] error reading file.\n";
				break;
			}
		}
		std::cout << "[INFO] File Loaded. Dont forget to calculate!\n";
		myfile.close();
	}
	else std::cout << "[ERROR] Unable to open file";

}


void ConfigParser::process(std::string s, std::ifstream &myfile)
{
	Config config;
	std::regex closeTag("^([:whitespace:])*</[a-zA-Z][a-zA-Z0-9\\._:=]*(?: [a-zA-Z_:][a-zA-Z0-9\\._:-]*(?: )?=(?: )?\"[^\"]+\")*>([:whitespace:])*$");
	std::regex e("(^[:whitespace:])*<[a-zA-Z][a-zA-Z0-9\\._:=]*(?: [a-zA-Z_:][a-zA-Z0-9\\._:-]*(?: )?=(?: )?\"[^\"]+\")*>(.)*</[a-zA-Z][a-zA-Z0-9\\._:=]*(?: [a-zA-Z_:][a-zA-Z0-9\\._:-]*(?: )?=(?: )?\"[^\"]+\")*>([:whitespace:])*$");
	std::regex tagName("<[a-zA-Z][a-zA-Z0-9\\._:=]*(?: [a-zA-Z_:][a-zA-Z0-9\\._:-]*(?: )?=(?: )?\"[^\"]+\")*>");
	std::regex content(">(.)*<");

	std::smatch m;
	std::smatch mm;
	std::smatch mmm;

	while ( getline (myfile, s) )
	{

		if (std::regex_match (s, closeTag))
		{
			config.calculate();
			//config.printVariables();

			configList->push_back(config);
			return;
		}

		while (std::regex_search (s, m, e))
		{
			if (std::regex_search (s, mm, tagName))
			{
				std::string data;
				if (std::regex_search (s, mmm, content))
				{
					data = mmm.str().substr(1, mmm.str().length() - 2);
					data = std::regex_replace(data, std::regex("^ +| +$|( ) +"), "$1");
				}
				std::string label = mm.str().substr(1, mm.str().length() - 2);
				if (label == "name")
				{
					config.setName(data);
				}
				else if (label == "C")
				{
					config.setC(std::stod(data.c_str()));
				}
				else if (label == "micDistance")
				{
					config.setMicDistance(std::stod(data.c_str()));
				}
				else if (label == "samplingRate")
				{
					config.setSamplingRate(std::atoi(data.c_str()));
				}
				else if (label == "phaseAlign")
				{
					config.setPhaseAlign(data.c_str());
				}
				else if (label == "nMics")
				{
					config.setNMics(std::atoi(data.c_str()));
				}
				else if (label == "nBands")
				{
					config.setNBands(std::atoi(data.c_str()));
				}
				else if (label == "requiredLagFrames")
				{
					config.setRequiredLagFrames(std::atoi(data.c_str()));
				}
				else if (label == "numFramesInBuffer")
				{
					config.setNumFramesInBuffer(std::atoi(data.c_str()));
				}
				else if (label == "lowCf")
				{
					config.setLowCf(std::atoi(data.c_str()));
				}
				else if (label == "highCf")
				{
					config.setHighCf(std::atoi(data.c_str()));
				}
				else if (label == "frameOverlap")
				{
					config.setFrameOverlap(std::atoi(data.c_str()));
				}
				else if (label == "attentionCaptureThreshold")
				{
					config.setAttentionCaptureThreshold(std::atoi(data.c_str()));
				}
				else if (label == "radialResolutionDegrees")
				{
					config.setRadialResolutionDegrees(std::stod(data.c_str()));
				}
				else if (label == "frameSamples")
				{
					config.setFrameSamples(std::atoi(data.c_str()));
				}
				else if (label == "interpellateNSamples")
				{
					config.setInterpellateNSamples(std::atoi(data.c_str()));
				}
				else
				{
					std::cout << "[WARN] The tag '" << label << "', does not exist!\n";
				}
			}

			s = m.suffix().str();
		}

	}
}

Config ConfigParser::getConfig(std::string name)
{
	Config con;
	for (std::vector<Config>::iterator it = configList->begin(); it != configList->end(); ++it)
	{
		if (it->getName() == name)
		{
			con = *it;
			return con;
		}
	}
	throw std::runtime_error(std::string("[ERROR] Invalid Config"));
}

ConfigParser::~ConfigParser()
{
	delete configList;
}
