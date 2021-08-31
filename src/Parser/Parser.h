#pragma once

#include <string>

#include "../Data/Savegame.h"

Savegame loadSavegame(const std::string& filepath);

std::vector<std::string> tokenizeLine(const std::string& line);

War convertToWar(const std::vector<std::string>& tokenStream);

Battle convertToBattle(const std::vector<std::string>& tokenStream);

void parseWarHistory(War& war, const std::vector<std::string>& tokenStream);