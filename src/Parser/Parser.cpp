#include "Parser.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <Types/nlohmann/json.h>

static const std::vector<char> tokenSplitter = 
{
	'{', '}', '"', ']', '[', '\n', '=', '(', ')' //, ' '
};

static const std::string tokenSplitterStr = 
{
	'{', '}', '"', '[', ']', '\n', '=', '(', ')' //, ' '
};

static const std::string warStart = "previous_war=";

static inline bool contains(const std::string& parentString, const std::string& containStr)
{
	if(parentString.find(containStr) != std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static inline bool lineContainsWarStart(const std::string& containStr)
{
	const std::string str("previous_");
	if(containStr.find(str) != std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}

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

	

	std::vector<std::vector<std::string>> info;
	bool continueLoop = true;
	std::string buffer = "";

	//This loop for now just gets the sections that are for wars
	while(continueLoop)
	{
		// contains(warStart, buffer)
		if(lineContainsWarStart(buffer))
		{
			std::vector<std::string> lines;

			std::getline(stream, buffer);
			//stream >> buffer;


			while(!lineContainsWarStart(buffer))	//buffer != warStart
			{
				lines.emplace_back(buffer);

				std::getline(stream, buffer);
			}

			info.emplace_back(lines);
		}
		else
		{
			std::getline(stream, buffer);
			
			if(stream.eof())
			{
				continueLoop = false;
			}
		}
	}
	//---//

	
	//This can be parallelized as long as the order for [warTokens] tokens is the same
	std::vector<std::vector<std::string>> warsTokens;
	for(const auto& wars : info)
	{	
		std::vector<std::string> warTokensVec;	
		for(const auto& lines : wars)
		{
			auto tokenLine = tokenizeLine(lines);
			
			for(const auto& tok : tokenLine)
			{
				warTokensVec.emplace_back(tok);
			}

		}
		warsTokens.push_back(warTokensVec);
	}

	Savegame savegame;

	for(const auto& wtoken : warsTokens)
	{
		savegame.addWar(convertToWar(wtoken));
	}

	return savegame;
}

static inline bool isToken(const char c) noexcept
{
	return (tokenSplitterStr.find(c) != std::string::npos);
}

std::vector<std::string> tokenizeLine(const std::string& line)
{
	std::vector<std::string> tokens;
	std::string curToken;

	//For every character in the line, check if it's a token splitter
	//If it splits the token, then push the existing token to the vector, and then the token splitter
	for(const char c : line)
	{
		if(!isToken(c))
		{
			curToken.push_back(c);
		}
		else
		{
			//Push the current string token to the vector
			if(!curToken.empty())
			{
				tokens.emplace_back(curToken);
			}
			curToken.clear();

			//Put the token splitting character into the vector
			const std::string tokenChar(1, c);
			tokens.emplace_back(tokenChar);
		}
	}

	//Another check in case of unfinished token or smth
	if(!curToken.empty())
	{
		tokens.emplace_back(curToken);
		curToken.clear();
	}

	std::cout << "Found " << tokens.size() << " tokens.\n";

	return tokens;
}

War convertToWar(const std::vector<std::string>& tokenStream)
{
	War w;

	//All the strings for the actions/commands/keywords relevant
	const std::string battle = "battle";
	const std::string name = "name";
	const std::string history = "history";
	const std::string original_wargoal = "original_wargoal";
	const std::string original_defender = "original_defender";
	const std::string original_attacker = "original_attacker";
	const std::string casus_belli = "casus_belli";
	const std::string actor = "actor";
	const std::string receiver = "receiver";
	const std::string action = "action";
	
	//Scope and Flags
	int scopeDepth = 0; //Everytime there is '{', increment, and when '}', decrement
	int bracketDepth = 0; //Everytime there is '[', increment, and when ']', decrement
	int parenthesisDepth = 0; //Everytime there is '(', increment, and when ')', decrement
	bool strLiteral = false; //Toggle when '"' is encountered, when true, directly parse everything into the lvalue operand
	bool isLeftOperand = true; 

	bool setBattle = false;

	std::string currentSection = "";

	std::vector<std::string> battleStream;
	std::string stringliteral;

	for(auto index = 0; index < tokenStream.size(); ++index)
	{
		const auto& token = tokenStream[index];

		if(isToken(token.at(0)))
		{
			if(bracketDepth >= 3)
			{
				battleStream.emplace_back(token);
			}

			switch (token.at(0))
			{	//Begin Switch
				case '"':
				{
					strLiteral = !strLiteral;
					break;
				}
				case '(':
				{
					parenthesisDepth += 1;
					break;
				}
				case ')':
				{
					parenthesisDepth -= 1;
					break;
				}
				case '[':
				{
					bracketDepth += 1;
					break;
				}
				case ']':
				{
					bracketDepth -= 1;
					break;
				}
				case '{':
				{
					scopeDepth += 1;
					break;
				}
				case '}':
				{
					scopeDepth -= 1;
					break;
				}

				//Others
				case '\n':
				{
					isLeftOperand = false;
					break;
				}
				case '=':
				{
					isLeftOperand = false;
					break;
				}

				default:
				{
					break;
				}
			}	//End Switch
		}
		else
		{
			if(bracketDepth >= 3)
			{
				battleStream.emplace_back(token);
				continue;
			}

			if(battleStream.size() > 10)
			{
				if(battleStream.back() == "}")
				{
					battleStream.pop_back();
				}

				w.battles.emplace_back(convertToBattle(battleStream));

				battleStream.clear();
			}

			if(strLiteral) { stringliteral.append(token); continue; }

			if(token == name) { currentSection = name; continue; }
			if(token == battle) { currentSection = battle; continue; }
			if(token == name) { currentSection = name; continue; }
			if(token == name) { currentSection = name; continue; }
			if(token == name) { currentSection = name; continue; }
			if(token == name) { currentSection = name; continue; }
			if(token == name) { currentSection = name; continue; }
		}

	}

	return w;
}

Battle convertToBattle(const std::vector<std::string>& tokenStream)
{
	Battle b;

	return b;
}