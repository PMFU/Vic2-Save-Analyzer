#pragma once

#include <string>

#include "../Data/Savegame.h"

Savegame loadSavegame(const std::string& filepath);
Savegame loadSavegameFromString(const std::string& fileStream);

std::vector<std::string> tokenizeLine(std::string_view line);

War convertToWar(const std::vector<std::string>& tokenStream);

Battle convertToBattle(const std::vector<std::string>& tokenStream);

void parseWarHistory(War& war, const std::vector<std::string>& tokenStream);