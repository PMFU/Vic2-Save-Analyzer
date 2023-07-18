#pragma once

#include "War.h"

#include <unordered_map>


struct Prices
{
	std::vector<std::string> good_names;
	std::vector<float> price_values;
};

class Savegame
{
public:
	Savegame();

	void addTestWar();

	void addWar(const War& war);

	void addPriceHistory(const Prices& prices);

	const std::unordered_map<std::string, War>& getWars() const noexcept;
	const std::unordered_map<int, Prices>& getPriceHistory() const noexcept;
private:
	std::unordered_map<std::string, War> m_warsmap;

	
	std::unordered_map<int, Prices> m_pricesmap;
};