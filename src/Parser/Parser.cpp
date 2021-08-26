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
	if(parentString.empty() || containStr.empty())
	{
		return false;
	}

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
	file.close();

	std::cout << "The stream has been loaded. There are " << stream.str().length() << " characters.\n";

	Date d;

	std::vector<std::vector<std::string>> info;

	for(std::string buffer; std::getline(stream, buffer, '\n'); )
	{
		if(lineContainsWarStart(buffer))
		{
			std::cout << "Found War!\n";

			std::vector<std::string> lines;
			std::getline(stream, buffer, '\n');
			buffer.push_back('\n');

			while((!lineContainsWarStart(buffer)) && (buffer != "}\n"))
			{
				lines.emplace_back(buffer);

				if(std::getline(stream, buffer, '\n'))
				{
					buffer.push_back('\n');
				}
				else
				{
					buffer.push_back('\n');
					break;
				}
				
			}

			info.emplace_back(lines);
		}
	}

	
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
		
		if(!warTokensVec.empty())
		{
			warsTokens.push_back(warTokensVec);
			std::cout << "Made a war token list.\n";
		}	
	}

	Savegame savegame;

	for(const auto& wtoken : warsTokens)
	{
		if(wtoken.size() == 0)
		{
			continue;
		}
		else
		{
			savegame.addWar(convertToWar(wtoken));
			std::cout << "Made a war.\n";
		}
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

	// /std::cout << "Found " << tokens.size() << " tokens.\n";

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
	std::string stringliteral = "";


	for(auto index = 0; index < tokenStream.size(); ++index)
	{
		const auto& token = tokenStream[index];
		if(token.empty())
		{
			continue;
		}

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
					isLeftOperand = true;

					if(bracketDepth >= 3)
					{
						break;
					}

					if(currentSection == name) { w.name = stringliteral; break; }
					if(currentSection == battle) { break; }
					if(currentSection == history) { /*currentSection = history;*/ break; }
					if(currentSection == original_wargoal) { /*currentSection = original_wargoal;*/ break; }
					if(currentSection == original_defender) { w.defenders.emplace_back(original_defender); break; }
					if(currentSection == original_attacker) { w.attackers.emplace_back(original_attacker); break; }
					if(currentSection == casus_belli) { w.wargoal = casus_belli; break; }

					currentSection.clear();

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

				//w.battles.emplace_back(convertToBattle(battleStream));

				battleStream.clear();
			}

			if(strLiteral) { stringliteral.append(token); continue; }

			if(isLeftOperand)
			{
				if(token == name) { currentSection = name; continue; }
				if(token == battle) { currentSection = battle; continue; }
				if(token == history) { currentSection = history; continue; }
				if(token == original_wargoal) { currentSection = original_wargoal; continue; }
				if(token == original_defender) { currentSection = original_defender; continue; }
				if(token == original_attacker) { currentSection = original_attacker; continue; }
				if(token == casus_belli) { currentSection = casus_belli; continue; }

				currentSection.clear();
			}
		}
	}

	return w;
}

Battle convertToBattle(const std::vector<std::string>& tokenStream)
{
	Battle b;

	std::cout << "BATTLE TOKEN STREAM:\n";
	for(const auto& t : tokenStream)
	{
		std::cout << t;
	}
	std::cout << "\nEND BATTLE TOKEN STREAM\n\n";

	const std::string battle = "battle";
	const std::string name = "name";
	const std::string location = "location";
	const std::string result = "result";
	const std::string attacker = "attacker";
	const std::string defender = "defender";

	const std::string country = "country";
	const std::string leader = "leader";
	const std::string losses = "losses";

	//Scope and Flags
	int scopeDepth = 0; //Everytime there is '{', increment, and when '}', decrement
	int bracketDepth = 0; //Everytime there is '[', increment, and when ']', decrement
	int parenthesisDepth = 0; //Everytime there is '(', increment, and when ')', decrement
	bool strLiteral = false; //Toggle when '"' is encountered, when true, directly parse everything into the lvalue operand
	bool isLeftOperand = true; 

	bool isActualBattle = false;
	
	int sideStatus = 0; //0 is none, 1 is atk, 2 is def

	std::string currentSection = "";
	std::string stringliteral;
	std::string unitname;

	for(auto index = 0; index < tokenStream.size(); ++index)
	{
		const auto& token = tokenStream[index];

		if(isToken(token.at(0)))
		{
			switch (token.at(0))
			{	//Begin Switch
				case '"':
				{
					if(strLiteral)
					{
						strLiteral = false;
					}
					else
					{
						stringliteral.clear();
						strLiteral = true;
					}

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
					isLeftOperand = true;

					if(currentSection == name) { b.name = stringliteral; break; }
					if(currentSection == battle) { break; }
					if(currentSection == location) { b.location = std::stoi(stringliteral); break; }
					if(currentSection == result) { (stringliteral == "yes") ? b.doesAttackerWin = true : b.doesAttackerWin = false; break; }
					if(currentSection == attacker) { sideStatus = 1; break; }
					if(currentSection == defender) { sideStatus = 2; break; }
					if(currentSection == country) { (sideStatus > 1) ? b.countryDEF = stringliteral: b.countryATK = stringliteral; break; }
					if(currentSection == leader) { (sideStatus > 1) ? b.leaderDEF = stringliteral: b.leaderATK = stringliteral; break; }
					if(currentSection == losses) { (sideStatus > 1) ? b.deflosses = std::stoi(stringliteral): b.atklosses = std::stoi(stringliteral); break; }
					
					if(currentSection == "unit") 
					{
						Unit u;
						u.name = unitname;
						u.size = std::stoi(stringliteral);

						(sideStatus > 1) ? b.defUnits.emplace_back(u): b.atkUnits.emplace_back(u);
					}
					

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

			if(strLiteral) { stringliteral.append(token); continue; }

			if(isLeftOperand)
			{
				if(token == name) { currentSection = name; continue; }
				if(token == battle) { currentSection = battle; continue; }
				if(token == location) { currentSection = location; continue; }
				if(token == result) { currentSection = result; continue; }
				if(token == attacker) { currentSection = attacker; continue; }
				if(token == defender) { currentSection = defender; continue; }
				if(token == country) { currentSection = country; continue; }
				if(token == leader) { currentSection = leader; continue; }
				if(token == losses) { currentSection = losses; continue; }

				currentSection = "unit";
				unitname = token;
			}
		}
	}

	return b;
}