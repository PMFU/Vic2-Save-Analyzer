#pragma once

#include <string>
#include <vector>

struct Unit
{
	std::string name;
	uint64_t size;
};


struct Battle
{
	std::string name;
	short location;
	
	std::string countryATK;
	std::string countryDEF;

	std::string leaderATK;
	std::string leaderDEF;

	bool doesAttackerWin;

	uint64_t atklosses;
	uint64_t deflosses;

	std::vector<Unit> atkUnits;
	std::vector<Unit> defUnits;
};