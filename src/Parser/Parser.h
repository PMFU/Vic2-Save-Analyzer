#pragma once

#include <string>

#include "../Data/Savegame.h"

Savegame loadSavegame(const std::string& filepath);

War convertToWar(const std::vector<std::string>& rawLines);

// Battle convertToBattle