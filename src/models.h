#pragma once

#include <string>

struct Player {
  std::string username;
  std::string password;
  int wins;
  int streak;
  bool isGuest;
  bool isAdmin;
};

struct GameResult {
  bool won;
  bool quitToMenu;
  int attemptsUsed;
};

enum class State {
  BLANK,
  CORRECT,
  MISPLACE,
  INCORRECT
};

struct Tile {
  char ch;
  State state;
};

enum class GuestMenuAction {
  Login,
  SignUp,
  PlayAsGuest,
  Leaderboard,
  Exit
};

enum class AdminMenuAction {
  Play,
  Daily,
  WordBank,
  DeletePlayers,
  Leaderboard,
  Logout,
  Exit
};

enum class PlayerMenuAction {
  Play,
  Daily,
  Leaderboard,
  Logout,
  Exit
};

enum class AfterGameAction {
  PlayAgain,
  ReturnToMenu
};

enum class WordBankDialogAction {
  Previous,
  Next,
  AddWord,
  RemoveWord,
  Back
};

enum class DeletePlayersDialogAction {
  Previous,
  Next,
  DeletePlayer,
  Back
};

inline constexpr int max_username_length = 10;
inline constexpr int max_rows = 6;
inline constexpr int leaderboard_entries_to_show = 5;
inline constexpr int words_per_page = 15;
inline constexpr int button_width = 40;
inline constexpr int rules_box_width = 45;
inline constexpr int rules_box_gap = 20;
inline constexpr int tile_width = 7;
inline constexpr int tile_height = 3;
inline constexpr int tile_border_width = 2;
inline constexpr int game_box_side_padding = 4;

inline const std::string admin_username = "admin";
inline const std::string admin_password = "admin123";
