#pragma once

#include "War.h"

#include <unordered_map>

class Savegame
{
public:
	Savegame();

	~Savegame();

	void addWar(const War& war);

	const std::unordered_map<std::string, War>& getWars() const noexcept;


private:
	std::unordered_map<std::string, War> m_warsmap;
};