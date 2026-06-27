#include "auth.h"

#include <algorithm>
#include <cctype>

#include "storage.h"

using namespace std;

bool IsNormalPlayer(const Player& player) {
  return !player.isGuest && !player.isAdmin;
}

Player MakeGuestPlayer() {
  return {"Guest", "", 0, 0, true, false};
}

Player MakeAdminPlayer() {
  return {admin_username, admin_password, 0, 0, false, true};
}

Player MakeRegisteredPlayer(const string& username, const string& password) {
  return {username, password, 0, 0, false, false};
}

vector<Player>::iterator FindPlayerIteratorByUsername(vector<Player>& players,
                                                      const string& username) {
  return find_if(players.begin(), players.end(), [&](const Player& player) {
    return player.username == username;
  });
}

vector<Player>::const_iterator FindPlayerIteratorByUsername(
    const vector<Player>& players,
    const string& username) {
  return find_if(players.begin(), players.end(), [&](const Player& player) {
    return player.username == username;
  });
}

bool IsValidUsername(const string& username) {
  if (username.empty()) {
    return false;
  }

  if (username.size() > max_username_length) {
    return false;
  }

  for (char ch : username) {
    unsigned char c = (unsigned char)ch;
    if (!isalnum(c) && c != '_') {
      return false;
    }
  }

  return true;
}

bool UsernameTaken(const vector<Player>& players, const string& username) {
  return username == admin_username ||
         FindPlayerIteratorByUsername(players, username) != players.end();
}

bool CheckNormalLogin(const vector<Player>& players,
                      const string& username,
                      const string& password,
                      Player& resultPlayer) {
  auto player = FindPlayerIteratorByUsername(players, username);
  if (player != players.end() && IsNormalPlayer(*player) &&
      player->password == password) {
    resultPlayer = *player;
    return true;
  }

  return false;
}

void SaveCurrentPlayer(const Player& currentPlayer, vector<Player>& players) {
  if (!IsNormalPlayer(currentPlayer)) {
    return;
  }

  auto player = FindPlayerIteratorByUsername(players, currentPlayer.username);
  if (player == players.end()) {
    players.push_back(currentPlayer);
  } else {
    *player = currentPlayer;
  }

  SavePlayers(players);
}
