#pragma once

#include "Battle.h"

#include <Types/Types.h>

struct War
{
	std::string name;

	std::vector<Battle> battles;

	std::vector<std::string> attackers;
	std::vector<std::string> defenders;

	bool doesAttackerWin;

	std::string wargoal;

	Date start;
	Date end;
};