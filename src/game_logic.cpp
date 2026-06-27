#include "game_logic.h"

#include <chrono>
#include <cctype>
#include <ctime>
#include <random>

using namespace std;

bool IsValidWord(const string& word, int wordLength) {
  if ((int)word.size() != wordLength) {
    return false;
  }

  for (char ch : word) {
    unsigned char c = (unsigned char)ch;
    if (!isalpha(c)) {
      return false;
    }
  }

  return true;
}

string ToUpperWord(string word) {
  for (char& ch : word) {
    ch = (char)toupper((unsigned char)ch);
  }
  return word;
}

string RowToWord(const Row& row) {
  string word;
  word.reserve(row.tiles.size());

  for (const auto& tile : row.tiles) {
    word += tile.ch;
  }

  return word;
}

mt19937& GlobalRng() {
  static mt19937 rng(
      (unsigned)chrono::steady_clock::now().time_since_epoch().count());
  return rng;
}

string PickRandomWord(const vector<string>& words) {
  if (words.empty()) {
    return "";
  }

  uniform_int_distribution<int> dist(0, (int)words.size() - 1);
  int index = dist(GlobalRng());
  return words[index];
}

string PickDailyWord(const vector<string>& words) {
  if (words.empty()) {
    return "";
  }

  auto now = chrono::system_clock::now();
  time_t t = chrono::system_clock::to_time_t(now);

  tm local_tm{};
#ifdef _WIN32
  localtime_s(&local_tm, &t);
#else
  local_tm = *std::localtime(&t);
#endif

  int seed = 0;
  seed = seed + (local_tm.tm_year + 1900) * 10000;
  seed = seed + (local_tm.tm_mon + 1) * 100;
  seed = seed + local_tm.tm_mday;

  int index = seed % (int)words.size();
  return words[index];
}

void EvaluateGuess(Row& row, string target, int wordLength) {
  string guess = RowToWord(row);

  for (int i = 0; i < wordLength; i++) {
    if (guess[i] == target[i]) {
      row.tiles[i].state = State::CORRECT;
      target[i] = '_';
    }
  }

  for (int i = 0; i < wordLength; i++) {
    if (row.tiles[i].state == State::CORRECT) {
      continue;
    }

    size_t pos = target.find(guess[i]);
    if (pos != string::npos) {
      row.tiles[i].state = State::MISPLACE;
      target[pos] = '_';
    } else {
      row.tiles[i].state = State::INCORRECT;
    }
  }
}
