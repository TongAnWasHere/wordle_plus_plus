#include "storage.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "auth.h"
#include "game_logic.h"

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

fs::path FindProjectRoot() {
  fs::path current = fs::current_path();

  for (int i = 0; i < 8; i++) {
    if (fs::exists(current / "CMakeLists.txt")) {
      return current;
    }

    if (fs::exists(current / "data" / "wordbank_3.json") ||
        fs::exists(current / "data" / "wordbank_5.json") ||
        fs::exists(current / "data" / "wordbank_6.json") ||
        fs::exists(current / "data" / "wordbank.json")) {
      return current;
    }

    if (!current.has_parent_path()) {
      break;
    }

    current = current.parent_path();
  }

  return fs::current_path();
}

fs::path PlayersFilePath() {
  return FindProjectRoot() / "data" / "players.json";
}

fs::path LegacyWordBankFilePath() {
  return FindProjectRoot() / "data" / "wordbank.json";
}

fs::path WordBankFilePath(int wordLength) {
  return FindProjectRoot() / "data" / ("wordbank_" + to_string(wordLength) +
                                       ".json");
}

bool LoadJsonFromFile(const fs::path& path, json& data) {
  ifstream ifs(path);
  if (!ifs) {
    return false;
  }

  try {
    ifs >> data;
  } catch (...) {
    return false;
  }

  return true;
}

void SaveJsonToFile(const fs::path& path, const json& data) {
  ofstream ofs(path);
  if (!ofs) {
    return;
  }

  ofs << data.dump(2) << '\n';
}

vector<Player> LoadPlayers() {
  json data;
  if (!LoadJsonFromFile(PlayersFilePath(), data) || !data.is_object()) {
    return {};
  }

  auto playersJson = data.find("players");
  if (playersJson == data.end() || !playersJson->is_array()) {
    return {};
  }

  vector<Player> players;
  for (const auto& item : *playersJson) {
    if (!item.is_object()) {
      continue;
    }

    players.push_back({
        item.value("username", ""),
        item.value("password", ""),
        item.value("wins", 0),
        item.value("streak", 0),
        false,
        false,
    });
  }

  return players;
}

void SavePlayers(const vector<Player>& players) {
  json data;
  data["players"] = json::array();

  for (const auto& player : players) {
    if (!IsNormalPlayer(player)) {
      continue;
    }

    json item;
    item["username"] = player.username;
    item["password"] = player.password;
    item["wins"] = player.wins;
    item["streak"] = player.streak;
    data["players"].push_back(item);
  }

  SaveJsonToFile(PlayersFilePath(), data);
}

vector<string> LoadWordBankFromPath(const fs::path& path, int wordLength) {
  json data;
  if (!LoadJsonFromFile(path, data) || !data.is_object()) {
    return {};
  }

  auto wordsJson = data.find("words");
  if (wordsJson == data.end() || !wordsJson->is_array()) {
    return {};
  }

  vector<string> words;
  for (const auto& item : *wordsJson) {
    if (!item.is_string()) {
      continue;
    }

    string word = ToUpperWord(item.get<string>());
    if (IsValidWord(word, wordLength)) {
      words.push_back(word);
    }
  }

  sort(words.begin(), words.end());
  words.erase(unique(words.begin(), words.end()), words.end());

  return words;
}

vector<string> LoadWordBank(int wordLength) {
  fs::path path = WordBankFilePath(wordLength);

  if (wordLength == 5 && !fs::exists(path)) {
    return LoadWordBankFromPath(LegacyWordBankFilePath(), wordLength);
  }

  return LoadWordBankFromPath(path, wordLength);
}

void SaveWordBank(const vector<string>& words, int wordLength) {
  vector<string> sortedWords = words;
  sort(sortedWords.begin(), sortedWords.end());
  sortedWords.erase(unique(sortedWords.begin(), sortedWords.end()),
                    sortedWords.end());

  json data;
  data["words"] = json::array();

  for (const auto& word : sortedWords) {
    data["words"].push_back(word);
  }

  SaveJsonToFile(WordBankFilePath(wordLength), data);
}
