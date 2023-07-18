#include "Savegame.h"

Savegame::Savegame()
{

}

void Savegame::addTestWar()
{
	//Test war
	War w;
	w.attackers.emplace_back("RUS");
	w.defenders.emplace_back("GER");
	w.doesAttackerWin = false;
	w.name = "Testing war";
	w.wargoal = "There is none, this is a test";

	//m_warsmap.insert(std::pair(w.name, w));
}

void Savegame::addWar(const War& war)
{
	m_warsmap.insert(std::pair(war.name, war));
}

void Savegame::addPriceHistory(const Prices& prices)
{
	m_pricesmap.insert(std::pair(m_pricesmap.size(), prices));
}

const std::unordered_map<std::string, War>& Savegame::getWars() const noexcept
{
	return m_warsmap;
}

const std::unordered_map<int, Prices>& Savegame::getPriceHistory() const noexcept
{
	return m_pricesmap;
}