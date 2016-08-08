/**
 * This class parses an XML file and stores one or more Config objects for
 * use during runtime.
 */

#ifndef __CONFIGPARSER_H
#define __CONFIGPARSER_H

#include <string>
#include <vector>
#include "Config.h"
#include <iostream>
#include <fstream>


class ConfigParser
{

public:
	/**
	 * Takes file name to parse. Must be in XML format and include proper definition tag
	 */

	static ConfigParser* getInstance(std::string);


	/**
	 * This takes the name of the configuration which must be defined uniquely.
	 * @param  name the configuration name
	 * @return      The configuration object
	 */
	Config getConfig(std::string name);

private:
	static ConfigParser* instance;


	ConfigParser(std::string);
	~ConfigParser();

	std::vector<Config> *configList;

	void process(std::string s, std::ifstream &myfile);

};


#endif
