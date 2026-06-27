#pragma once

#include <string>
#include <vector>

#include "models.h"

std::vector<Player> LoadPlayers();
void SavePlayers(const std::vector<Player>& players);

std::vector<std::string> LoadWordBank(int wordLength);
void SaveWordBank(const std::vector<std::string>& words, int wordLength);
