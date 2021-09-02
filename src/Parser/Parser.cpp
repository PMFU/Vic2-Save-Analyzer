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

static bool getDateFromPdxString(const std::string& suspectString, Date& d)
{
	bool result = false;

	std::cout << "Date Str: " << suspectString << "\n";

	if(!contains(suspectString, "battle"))
	{
		if(suspectString.length() < 12)
		{
			int year = 0;
			int month = 0;
			int day = 0;

			int indexPoint = suspectString.find_first_of('.');
			if(indexPoint == std::string::npos)
			{
				std::cout << "Date String Parsing Issue! |" << suspectString << "|\n";
				result = false;
				return result;
			}

			year = std::stoi(suspectString.substr(0, indexPoint)); // Year

			auto monthdaystr = suspectString.substr(indexPoint + 1, suspectString.length() - (indexPoint + 1));
			indexPoint = monthdaystr.find_first_of('.');

			if(indexPoint == std::string::npos)
			{
				std::cout << "Date String Parsed Wrong! |" << suspectString << "|\n";
				result = false;
				return result;
			}
		
			result = true;

			month = std::stoi(monthdaystr.substr(0, indexPoint));

			day = std::stoi(monthdaystr.substr(indexPoint + 1, monthdaystr.length() - (indexPoint + 1)));
			
			d = Date(year, month, day);
		}
	}

	return result;
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
					buffer.push_back('\0');
					break;
				}	
			}
			
			lines.emplace_back(buffer);
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

Savegame loadSavegameFromString(const std::string& fileStream)
{
	std::stringstream stream;

	stream.str(fileStream);

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
					buffer.push_back('\0');
					break;
				}	
			}
			
			lines.emplace_back(buffer);
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

	bool isLeftOperand = true; 

	bool isBattle = false;
	int curBattleScope = 9999;
	std::vector<std::string> battleStream;

	/*bool isHistory = false;
	int curHistoryScope = 9999;
	std::vector<std::string> historyStream;*/
	
	std::string stringliteral;
	std::string currentSection;

	for(auto index = 0; index < tokenStream.size(); ++index)
	{
		const auto& tkn = tokenStream[index];
		if(tkn.empty())
		{
			continue;
		}

		std::string token;

		if(tkn.at(0) == '\t')
		{
			auto x = tkn.find_first_not_of('\t');
			if(x == std::string::npos) { continue; }

			token = tkn.substr(x, tkn.length() - x);
		}
		else
		{
			token = tkn;
		}

		if(isToken(token.at(0)))
		{
			if(isBattle)
			{
				battleStream.emplace_back(token);
			}

			switch (token.at(0))
			{	//Begin Switch
				case '"':
				{
					stringliteral.clear();
					index += 1;

					while(tokenStream[index].at(0) != '"')
					{
						//Add the token to the battle stream if in it
						if(isBattle)
						{
							battleStream.emplace_back(tokenStream[index]);
						}

						//Add the token to the string literal string
						stringliteral.append(tokenStream[index]);
						index += 1;
					}

					if(isBattle)
					{
						battleStream.emplace_back("\"");
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

					//End the battle token stream
					if((curBattleScope - 1) == scopeDepth)
					{
						isBattle = false;
						curBattleScope = 9999;

						/*if(battleStream.empty()) //(curBattleScope - 1) >= scopeDepth
						{
							break;
						}*/

						if(battleStream.at(0) != battle)
						{
							battleStream.clear();
							break;
						}

						w.battles.emplace_back(convertToBattle(battleStream));
						battleStream.clear();
					}

					break;
				}

				//Others
				case '\n':
				{
					isLeftOperand = true;

					if(isBattle)
					{
						break;
					}

					if(currentSection == name) { w.name = stringliteral; break; }
					if(currentSection == battle) { isBattle = true; curBattleScope = scopeDepth + 1; break; }
					if(currentSection == history) { /*currentSection = history;*/ break; }
					if(currentSection == original_wargoal) { /*currentSection = original_wargoal;*/ break; }
					if(currentSection == original_defender) { w.defenders.emplace_back(original_defender); break; }
					if(currentSection == original_attacker) { w.attackers.emplace_back(original_attacker); break; }
					if(currentSection == casus_belli) { w.wargoal = casus_belli; break; }
					
					//Starting date for war
					if(currentSection == action)
					{ 
						if(!getDateFromPdxString(stringliteral, w.start))
						{
							std::cout << "Issue with getting the date for the war start.\n";
						}
						break; 
					}

					currentSection.clear();
					stringliteral.clear();

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
			if(isBattle)
			{
				battleStream.emplace_back(token);
				continue;
			}

			if(isLeftOperand)
			{
				if(token == name) { currentSection = name; continue; }
				if(token == battle) { currentSection = battle; continue; }
				if(token == history) { currentSection = history; continue; }
				if(token == original_wargoal) { currentSection = original_wargoal; continue; }
				if(token == original_defender) { currentSection = original_defender; continue; }
				if(token == original_attacker) { currentSection = original_attacker; continue; }
				if(token == casus_belli) { currentSection = casus_belli; continue; }
				if(token == action) { currentSection = action; continue; }

				std::cout << "THIS IS THE UNPARSED TOKEN: |" << token << "| \n";

				currentSection = token; 
			}
		}
	}

	return w;
}

Battle convertToBattle(const std::vector<std::string>& tokenStream)
{
	Battle b;

	/*std::cout << "BATTLE TOKEN STREAM:\n";
	for(const auto& t : tokenStream)
	{
		std::cout << t << '|';
	}
	std::cout << "\nEND BATTLE TOKEN STREAM\n\n";*/

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

	bool isLeftOperand = true; 

	bool isActualBattle = false;
	
	int sideStatus = 0; //0 is none, 1 is atk, 2 is def

	std::string currentSection;
	std::string stringliteral;
	std::string unitname;

	currentSection.clear();
	stringliteral.clear();
	unitname.clear();

	for(auto index = 0; index < tokenStream.size(); ++index)
	{
		const auto& token = tokenStream[index];

		if(token.empty())
		{
			continue;
		}

		if(isToken(token.at(0)))
		{
			switch (token.at(0))
			{	//Begin Switch
				case '"':
				{
					stringliteral.clear();
					index += 1;

					while(tokenStream[index].at(0) != '"')
					{
						//Add the token to the string literal string
						stringliteral.append(tokenStream[index]);
						index += 1;
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

					if(stringliteral.empty())
					{
						std::cout << "String Literal is empty\n";
						break;
					}

					if(currentSection == name) { b.name = stringliteral; break; }
					if(currentSection == battle) { break; }
					if(currentSection == attacker) { sideStatus = 1; break; }
					if(currentSection == defender) { sideStatus = 2; break; }
					if(currentSection == country) { (sideStatus > 1) ? b.countryDEF = stringliteral: b.countryATK = stringliteral; break; }
					if(currentSection == leader) { (sideStatus > 1) ? b.leaderDEF = stringliteral: b.leaderATK = stringliteral; break; }
					
					//std::cout << "THE CURRENT SELECTION: |" << currentSection << "|\n";
					//std::cout << "THE STRING LITERAL: |" << stringliteral << "|\n";
					

					if(currentSection == result) { contains(stringliteral, "y") ? b.doesAttackerWin = true : b.doesAttackerWin = false; break; }

					if(currentSection == losses) { (sideStatus > 1) ? b.deflosses = std::stoi(stringliteral): b.atklosses = std::stoi(stringliteral); break; }
					if(currentSection == location) { b.location = std::stoi(stringliteral); break; }

					if(currentSection == "unit") 
					{
						//std::cout << "THE UNIT TYPE: |" << unitname << "|\n";
						
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
			else
			{
				stringliteral.clear();
				stringliteral = token;
			}
		}
	}

	return b;
}

void parseWarHistory(War& war, const std::vector<std::string>& tokenStream)
{
	Battle b;

	const std::string battle = "battle";
	const std::string name = "name";
	const std::string location = "location";
	const std::string result = "result";
	const std::string attacker = "attacker";
	const std::string defender = "defender";

	const std::string country = "country";
	const std::string leader = "leader";
	const std::string losses = "losses";

	const std::string add_attacker = "add_attacker";
	const std::string add_defender = "add_defender";
	const std::string rem_attacker = "rem_attacker";
	const std::string rem_defender = "rem_defender";

	//Scope and Flags
	int scopeDepth = 0; //Everytime there is '{', increment, and when '}', decrement
	int bracketDepth = 0; //Everytime there is '[', increment, and when ']', decrement
	int parenthesisDepth = 0; //Everytime there is '(', increment, and when ')', decrement

	bool isLeftOperand = true; 

	bool isActualBattle = false;
	
	int sideStatus = 0; //0 is none, 1 is atk, 2 is def

	std::string currentSection;
	std::string stringliteral;
	std::string unitname;

	currentSection.clear();
	stringliteral.clear();
	unitname.clear();

	for(auto index = 0; index < tokenStream.size(); ++index)
	{
		const auto& token = tokenStream[index];

		if(token.empty())
		{
			continue;
		}

		if(isToken(token.at(0)))
		{
			switch (token.at(0))
			{	//Begin Switch
				case '"':
				{
					stringliteral.clear();
					index += 1;

					while(tokenStream[index].at(0) != '"')
					{
						//Add the token to the string literal string
						stringliteral.append(tokenStream[index]);
						index += 1;
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

					if(stringliteral.empty())
					{
						std::cout << "String Literal is empty\n";
						break;
					}

					if(currentSection == name) { b.name = stringliteral; break; }
					if(currentSection == battle) { break; }
					if(currentSection == attacker) { sideStatus = 1; break; }
					if(currentSection == defender) { sideStatus = 2; break; }
					if(currentSection == country) { (sideStatus > 1) ? b.countryDEF = stringliteral: b.countryATK = stringliteral; break; }
					if(currentSection == leader) { (sideStatus > 1) ? b.leaderDEF = stringliteral: b.leaderATK = stringliteral; break; }
					
					//std::cout << "THE CURRENT SELECTION: |" << currentSection << "|\n";
					//std::cout << "THE STRING LITERAL: |" << stringliteral << "|\n";
					

					if(currentSection == result) { (stringliteral == "yes") ? b.doesAttackerWin = true : b.doesAttackerWin = false; break; }

					if(currentSection == losses) { (sideStatus > 1) ? b.deflosses = std::stoi(stringliteral): b.atklosses = std::stoi(stringliteral); break; }
					if(currentSection == location) { b.location = std::stoi(stringliteral); break; }

					if(currentSection == "unit") 
					{
						//std::cout << "THE UNIT TYPE: |" << unitname << "|\n";
						
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
			else
			{
				stringliteral.clear();
				stringliteral = token;
			}
		}
	}
}