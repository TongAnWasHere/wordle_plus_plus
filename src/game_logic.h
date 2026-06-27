#pragma once

#include <string>
#include <vector>

#include "models.h"

struct Row {
  std::vector<Tile> tiles;

  Row() = default;
  explicit Row(int wordLength) : tiles(wordLength) {}
};

bool IsValidWord(const std::string& word, int wordLength);
std::string ToUpperWord(std::string word);
std::string RowToWord(const Row& row);
std::string PickRandomWord(const std::vector<std::string>& words);
std::string PickDailyWord(const std::vector<std::string>& words);
void EvaluateGuess(Row& row, std::string target, int wordLength);
