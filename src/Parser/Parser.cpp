#include "Parser.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <Types/nlohmann/json.h>

const std::vector<char> openSection = 
{
	'{',
	'"',
	'[',
	' ',
	' '
};

const std::vector<char> closeSection = 
{
	'}',
	'"',
	']',
	' ',
	' '
};

const std::string warStart = "previous_war=";

Savegame loadSavegame(const std::string& filepath)
{
	std::ifstream file(filepath);

	if(!file.is_open())
	{
		std::cout << "Could not open the file!\n";
	}

	std::stringstream stream;
	stream << file.rdbuf();

	Date d;

	
	std::string buffer = "";
	bool continueLoop = true;

	std::vector<std::vector<std::string>> info;

	while(continueLoop)
	{
		if(buffer == warStart)
		{
			std::vector<std::string> lines;

			stream >> buffer;

			while(buffer != warStart)
			{
				lines.emplace_back(buffer);
				stream >> buffer;
			}
			info.emplace_back(lines);
		}
		else
		{
			stream >> buffer;
		}
	}

	
	
}