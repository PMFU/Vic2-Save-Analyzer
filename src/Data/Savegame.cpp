#include "Savegame.h"

Savegame::Savegame()
{

}

Savegame::~Savegame()
{

}

void Savegame::addWar(const War& war)
{
	m_warsmap.insert(std::pair(war.name, war));
}

const std::unordered_map<std::string, War>& Savegame::getWars() const noexcept
{
	return m_warsmap;
}