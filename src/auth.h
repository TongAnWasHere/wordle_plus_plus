#pragma once

#include <string>
#include <vector>

#include "models.h"

bool IsNormalPlayer(const Player& player);
Player MakeGuestPlayer();
Player MakeAdminPlayer();
Player MakeRegisteredPlayer(const std::string& username,
                            const std::string& password);

std::vector<Player>::iterator FindPlayerIteratorByUsername(
    std::vector<Player>& players,
    const std::string& username);

std::vector<Player>::const_iterator FindPlayerIteratorByUsername(
    const std::vector<Player>& players,
    const std::string& username);

bool IsValidUsername(const std::string& username);
bool UsernameTaken(const std::vector<Player>& players,
                   const std::string& username);
bool CheckNormalLogin(const std::vector<Player>& players,
                      const std::string& username,
                      const std::string& password,
                      Player& resultPlayer);
void SaveCurrentPlayer(const Player& currentPlayer,
                       std::vector<Player>& players);
