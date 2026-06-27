#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "app.h"
#include "auth.h"
#include "game_logic.h"
#include "models.h"
#include "storage.h"

using namespace ftxui;
using namespace std;

Element vertical_spacer(int height) {
  return text("") | size(HEIGHT, EQUAL, height);
}

Element horizontal_spacer(int width) {
  return text("") | size(WIDTH, EQUAL, width);
}

string compact_button_label(const string& label) {
  return " " + label + " ";
}

string wide_button_label(const string& label) {
  return " " + label;
}

Element RenderTitle();

int GetGameBoxWidth(int wordLength) {
  int tileOuterWidth = tile_width + tile_border_width;
  int boardRowWidth = wordLength * tileOuterWidth;
  int boardInnerWidth = boardRowWidth + game_box_side_padding * 2;
  return boardInnerWidth + 2;
}

Element ColoredSubtitleLine(const string& left,
                            const string& middle,
                            const string& right) {
  return hbox({
             text(left) | color(Color::Green),
             text(middle) | color(Color::Yellow),
             text(right) | color(Color::Red),
         }) | center;
}

Element RenderTitle() {
  return vbox({
             text(" █████   ███   █████                                 █████    ████                                           ") | center,
             text("▒▒███   ▒███  ▒▒███                                 ▒▒███    ▒▒███                     ███            ███    ") | center,
             text(" ▒███   ▒███   ▒███      ██████     ████████      ███████     ▒███      ██████        ▒███           ▒███    ") | center,
             text(" ▒███   ▒███   ▒███     ███▒▒███   ▒▒███▒▒███    ███▒▒███     ▒███     ███▒▒███    ███████████    ███████████") | center,
             text(" ▒▒███  █████  ███     ▒███ ▒███    ▒███ ▒▒▒    ▒███ ▒███     ▒███    ▒███████    ▒▒▒▒▒███▒▒▒    ▒▒▒▒▒███▒▒▒ ") | center,
             text("  ▒▒▒█████▒█████▒      ▒███ ▒███    ▒███        ▒███ ▒███     ▒███    ▒███▒▒▒         ▒███           ▒███    ") | center,
             text("    ▒▒███ ▒▒███        ▒▒██████     █████       ▒▒████████    █████   ▒▒██████        ▒▒▒            ▒▒▒     ") | center,
             text("     ▒▒▒   ▒▒▒          ▒▒▒▒▒▒     ▒▒▒▒▒         ▒▒▒▒▒▒▒▒    ▒▒▒▒▒     ▒▒▒▒▒▒                                ") | center,
             vertical_spacer(1),
             vertical_spacer(1),

             ColoredSubtitleLine(
                 "██     ██  ▄▄▄  ▄▄▄▄  ▄▄▄▄     ",
                 "▄████  ▄▄ ▄▄ ▄▄▄▄▄  ▄▄▄▄  ▄▄▄▄ ▄▄ ▄▄  ▄▄  ▄▄▄▄    ",
                 "▄████   ▄▄▄  ▄▄   ▄▄ ▄▄▄▄▄"),

             ColoredSubtitleLine(
                 " ██ ▄█▄ ██ ██▀██ ██▄█▄ ██▀██   ",
                 "██  ▄▄▄ ██ ██ ██▄▄  ███▄▄ ███▄▄ ██ ███▄██ ██ ▄▄   ",
                 "██  ▄▄▄ ██▀██ ██▀▄▀██ ██▄▄  "),

             ColoredSubtitleLine(
                 "  ▀██▀██▀  ▀███▀ ██ ██ ████▀   ",
                 " ▀███▀  ▀███▀ ██▄▄▄ ▄▄██▀ ▄▄██▀ ██ ██ ▀██ ▀███▀   ",
                 " ▀███▀  ██▀██ ██   ██ ██▄▄▄ "),

             vertical_spacer(1),
         });
}

Elements RenderTiles(const Row& row) {
  Elements elems;

  for (const auto& tile : row.tiles) {
    string s(1, tile.ch == ' ' ? ' ' : tile.ch);
    Color bg = Color::GrayDark;

    if (tile.state == State::CORRECT) {
      bg = Color::Green;
    } else if (tile.state == State::MISPLACE) {
      bg = Color::Gold1;
    } else if (tile.state == State::INCORRECT) {
      bg = Color::Red;
    }

    elems.push_back(text(s) | bold | center | size(WIDTH, EQUAL, tile_width) |
                    size(HEIGHT, EQUAL, tile_height) | bgcolor(bg) |
                    color(Color::White) | borderEmpty);
  }

  return elems;
}

// Dialog helpers 

bool PlayInfoDialog(const string& title,
                       const string& confirmText,
                       string& username,
                       string& password) {
  auto screen = ScreenInteractive::Fullscreen();

  username = "";
  password = "";
  bool submitted = false;

  auto usernameInput = Input(&username, "username");
  InputOption passwordOption;
  passwordOption.content = &password;
  passwordOption.placeholder = "password";
  passwordOption.password = true;
  passwordOption.multiline = false;
  auto passwordInput = Input(passwordOption);

  auto confirm = Button(compact_button_label(confirmText), [&] {
    submitted = true;
    screen.ExitLoopClosure()();
  });

  auto cancel = Button(compact_button_label("Cancel"), [&] {
    submitted = false;
    screen.ExitLoopClosure()();
  });

  auto form = Container::Vertical({
      usernameInput,
      passwordInput,
      Container::Horizontal({confirm, cancel}),
  });

  auto renderer = Renderer(form, [&] {
    return vbox({
               text(title) | bold | center,
               separator(),
               vertical_spacer(1),
               hbox({
                   text("Username: ") | bold,
                   usernameInput->Render(),
               }) | center,
               vertical_spacer(1),
               hbox({
                   text("Password: ") | bold,
                   passwordInput->Render(),
               }) | center,
               vertical_spacer(1),
               separator(),
               hbox({
                   confirm->Render(),
                   horizontal_spacer(3),
                   cancel->Render(),
               }) | center,
           }) |
           border | size(WIDTH, GREATER_THAN, 30) | center;
  });

  screen.Loop(renderer);
  return submitted && !username.empty() && !password.empty();
}

void ConfirmationDialog(const string& message) {
  auto screen = ScreenInteractive::Fullscreen();
  auto okay = Button(compact_button_label("Continue"), screen.ExitLoopClosure());

  auto renderer = Renderer(okay, [&] {
    return vbox({
               vertical_spacer(1),
               text(message) | center,
               vertical_spacer(1),
               separator(),
               okay->Render() | center,
           }) |
           border | size(WIDTH, GREATER_THAN, 30) | center;
  });

  screen.Loop(renderer);
}

bool LoginDialog(string& username, string& password) {
  return PlayInfoDialog("Log In", "Log in", username, password);
}

bool SignupDialog(string& username, string& password) {
  return PlayInfoDialog("Sign Up", "Create", username, password);
}

bool SingleDialog(const string& title,
                       const string& label,
                       const string& placeholder,
                       const string& confirmText,
                       string& output) {
  auto screen = ScreenInteractive::Fullscreen();

  output = "";
  bool submitted = false;

  auto input = Input(&output, placeholder);

  auto confirm = Button(compact_button_label(confirmText), [&] {
    submitted = true;
    screen.ExitLoopClosure()();
  });

  auto cancel = Button(compact_button_label("Cancel"), [&] {
    submitted = false;
    screen.ExitLoopClosure()();
  });

  auto form = Container::Vertical({
      input,
      Container::Horizontal({confirm, cancel}),
  });

  auto renderer = Renderer(form, [&] {
    return vbox({
               text(title) | bold | center,
               separator(),
               vertical_spacer(1),
               hbox({
                   text(label),
                   input->Render(),
               }) | center,
               vertical_spacer(1),
               separator(),
               hbox({
                   confirm->Render(),
                   horizontal_spacer(3),
                   cancel->Render(),
               }) | center,
           }) |
           border | size(WIDTH, GREATER_THAN, 30) | center;
  });

  screen.Loop(renderer);

  if (!submitted) {
    return false;
  }

  if (output.empty()) {
    return false;
  }

  return true;
}

bool ShowPauseDialog() {
  auto screen = ScreenInteractive::Fullscreen();
  bool quit = false;

  auto resume = Button(compact_button_label("Resume"), [&] {
    quit = false;
    screen.ExitLoopClosure()();
  });

  auto quit_btn = Button(compact_button_label("Quit to Menu"), [&] {
    quit = true;
    screen.ExitLoopClosure()();
  });

  auto container = Container::Horizontal({resume, quit_btn});

  auto renderer = Renderer(container, [&] {
    return vbox({
               text("Game Paused.") | bold | center,
               separator(),
               hbox({
                   resume->Render(),
                   horizontal_spacer(3),
                   quit_btn->Render(),
               }) | center,
           }) |
           border | size(WIDTH, GREATER_THAN, 30) | center;
  });

  screen.Loop(renderer);
  return quit;
}

AfterGameAction ShowAfterGameOptions(bool isGuest,
                                     bool isAdmin,
                                     const string& username,
                                     bool won,
                                     int attemptsUsed) {
  auto screen = ScreenInteractive::Fullscreen();
  AfterGameAction choice = AfterGameAction::ReturnToMenu;

  auto play_again = Button(compact_button_label("Play Again"), [&] {
    choice = AfterGameAction::PlayAgain;
    screen.ExitLoopClosure()();
  });

  auto return_menu = Button(compact_button_label("Return to Menu"), [&] {
    choice = AfterGameAction::ReturnToMenu;
    screen.ExitLoopClosure()();
  });

  string sessionLabel = "Game finished for: " + username;
  if (isGuest) {
    sessionLabel = "Game finished for Guest";
  } else if (isAdmin) {
    sessionLabel = "Game finished for Admin";
  }

  string resultLabel = "Game over.";
  if (won) {
    resultLabel = "You won in " + to_string(attemptsUsed) +
                  (attemptsUsed == 1 ? " attempt." : " attempts.");
  }

  auto container = Container::Horizontal({play_again, return_menu});

  auto renderer = Renderer(container, [&] {
    return vbox({
               text(sessionLabel) | bold | center,
               separator(),
               vertical_spacer(1),
               text(resultLabel) | center,
               vertical_spacer(1),
               separator(),
               hbox({
                   play_again->Render(),
                   horizontal_spacer(3),
                   return_menu->Render(),
               }) | center,
           }) |
           border | size(WIDTH, GREATER_THAN, 30) | center;
  });

  screen.Loop(renderer);
  return choice;
}

int ShowPlayModeDialog() {
  auto screen = ScreenInteractive::Fullscreen();
  int wordLength = 0;

  auto mode3 = Button(wide_button_label("3-Letter Mode"), [&] {
    wordLength = 3;
    screen.ExitLoopClosure()();
  });

  auto mode5 = Button(wide_button_label("5-Letter Mode"), [&] {
    wordLength = 5;
    screen.ExitLoopClosure()();
  });

  auto mode6 = Button(wide_button_label("6-Letter Mode"), [&] {
    wordLength = 6;
    screen.ExitLoopClosure()();
  });

  auto back = Button(wide_button_label("Back"), [&] {
    wordLength = 0;
    screen.ExitLoopClosure()();
  });

  auto menu = Container::Vertical({mode3, mode5, mode6, back});

  auto renderer = Renderer(menu, [&] {
    return vbox({
               text("Select Play Mode") | bold | center,
               separator(),
               mode3->Render() | size(WIDTH, EQUAL, button_width) | center,
               mode5->Render() | size(WIDTH, EQUAL, button_width) | center,
               mode6->Render() | size(WIDTH, EQUAL, button_width) | center,
               back->Render() | size(WIDTH, EQUAL, button_width) | center,
           }) |
           border | center;
  });

  screen.Loop(renderer);
  return wordLength;
}

int ShowWordBankModeDialog() {
  auto screen = ScreenInteractive::Fullscreen();
  int wordLength = 0;

  auto bank3 = Button(wide_button_label("3-Letter Bank"), [&] {
    wordLength = 3;
    screen.ExitLoopClosure()();
  });

  auto bank5 = Button(wide_button_label("5-Letter Bank"), [&] {
    wordLength = 5;
    screen.ExitLoopClosure()();
  });

  auto bank6 = Button(wide_button_label("6-Letter Bank"), [&] {
    wordLength = 6;
    screen.ExitLoopClosure()();
  });

  auto back = Button(wide_button_label("Back"), [&] {
    wordLength = 0;
    screen.ExitLoopClosure()();
  });

  auto menu = Container::Vertical({bank3, bank5, bank6, back});

  auto renderer = Renderer(menu, [&] {
    return vbox({
               text("Select Word Bank") | bold | center,
               separator(),
               bank3->Render() | size(WIDTH, EQUAL, button_width) | center,
               bank5->Render() | size(WIDTH, EQUAL, button_width) | center,
               bank6->Render() | size(WIDTH, EQUAL, button_width) | center,
               back->Render() | size(WIDTH, EQUAL, button_width) | center,
           }) |
           border | center;
  });

  screen.Loop(renderer);
  return wordLength;
}

// Leaderboard

void ShowLeaderboardDialog(const vector<Player>& players) {
  auto screen = ScreenInteractive::Fullscreen();

  vector<Player> sorted = players;

  sort(sorted.begin(), sorted.end(), [](const Player& a, const Player& b) {
    if (a.streak != b.streak) {
      return a.streak > b.streak;
    }
    return a.wins > b.wins;
  });

  Elements lines;

  if (sorted.empty()) {
    lines.push_back(text("No players yet.") | center);
  } else {
    int rank = 1;
    for (const auto& player : sorted) {
      if (rank > leaderboard_entries_to_show) {
        break;
      }

      lines.push_back(
          text(to_string(rank) + ". " + player.username + "  |  Streak: " +
               to_string(player.streak) + "  |  Wins: " +
               to_string(player.wins)) |
          center);
      rank++;
    }
  }

  auto back = Button(compact_button_label("Back"), screen.ExitLoopClosure());

  auto renderer = Renderer(back, [&] {
    return vbox({
               text("██     ▄▄▄▄▄  ▄▄▄  ▄▄▄▄  ▄▄▄▄▄ ▄▄▄▄  ▄▄▄▄   ▄▄▄   ▄▄▄  ▄▄▄▄  ▄▄▄▄  ") | center,
               text("██     ██▄▄  ██▀██ ██▀██ ██▄▄  ██▄█▄ ██▄██ ██▀██ ██▀██ ██▄█▄ ██▀██ ") | center,
               text("██████ ██▄▄▄ ██▀██ ████▀ ██▄▄▄ ██ ██ ██▄█▀ ▀███▀ ██▀██ ██ ██ ████▀ ") | center,
               vertical_spacer(1),
               separator(),
               vbox(lines) | center,
               separator(),
               back->Render() | center,
           }) |
           border | center;
  });

  screen.Loop(renderer);
}

// Word bank admin

void ShowWordBankDialog(vector<string>& wordBank, int wordLength) {
  const vector<string> titleLines = {
      "██     ██  ▄▄▄  ▄▄▄▄  ▄▄▄▄    █████▄  ▄▄▄  ▄▄  ▄▄ ▄▄ ▄▄ ",
      "██ ▄█▄ ██ ██▀██ ██▄█▄ ██▀██   ██▄▄██ ██▀██ ███▄██ ██▄█▀ ",
      " ▀██▀██▀  ▀███▀ ██ ██ ████▀   ██▄▄█▀ ██▀██ ██ ▀██ ██ ██ ",
  };
  int page = 0;

  while (true) {
    int totalPages =
        max(1, ((int)wordBank.size() + words_per_page - 1) / words_per_page);
    page = clamp(page, 0, totalPages - 1);
    int startIndex = page * words_per_page;
    int endIndex = min(startIndex + words_per_page, (int)wordBank.size());

    auto screen = ScreenInteractive::Fullscreen();
    WordBankDialogAction action = WordBankDialogAction::Back;

    auto previous = Button(compact_button_label("Previous"), [&] {
      action = WordBankDialogAction::Previous;
      screen.ExitLoopClosure()();
    });

    auto next = Button(compact_button_label("Next"), [&] {
      action = WordBankDialogAction::Next;
      screen.ExitLoopClosure()();
    });

    auto addWord = Button(compact_button_label("Add Word"), [&] {
      action = WordBankDialogAction::AddWord;
      screen.ExitLoopClosure()();
    });

    auto removeWord = Button(compact_button_label("Remove Word"), [&] {
      action = WordBankDialogAction::RemoveWord;
      screen.ExitLoopClosure()();
    });

    auto back = Button(compact_button_label("Back"), [&] {
      action = WordBankDialogAction::Back;
      screen.ExitLoopClosure()();
    });

    auto controls = Container::Vertical({
        Container::Horizontal({previous, next}),
        Container::Horizontal({addWord, removeWord, back}),
    });

    auto renderer = Renderer(controls, [&] {
      Elements titleElements;
      for (const auto& line : titleLines) {
        titleElements.push_back(text(line) | center);
      }

      Elements lineElements;
      if (wordBank.empty()) {
        lineElements.push_back(text("Word bank is empty.") | center);
      } else {
        for (int i = startIndex; i < endIndex; i++) {
          lineElements.push_back(text(to_string(i + 1) + ". " + wordBank[i]) |
                                 center);
        }
      }

      return vbox({
                 vbox(titleElements),
                 vertical_spacer(1),
                 separator(),
                 text("Page " + to_string(page + 1) + " / " +
                      to_string(totalPages)) |
                     center,
                 separator(),
                 vbox(lineElements) | center,
                 separator(),
                 hbox({
                     previous->Render(),
                     horizontal_spacer(3),
                     next->Render(),
                 }) | center,
                 hbox({
                     addWord->Render(),
                     horizontal_spacer(3),
                     removeWord->Render(),
                     horizontal_spacer(3),
                     back->Render(),
                 }) | center,
             }) |
             border | center;
    });

    screen.Loop(renderer);

    if (action == WordBankDialogAction::Previous) {
      if (page > 0) {
        page--;
      }
    } else if (action == WordBankDialogAction::Next) {
      if (page + 1 < totalPages) {
        page++;
      }
    } else if (action == WordBankDialogAction::AddWord) {
      string inputWord = "";
      bool gotInput = SingleDialog("Add Word", "Word: ",
                                        to_string(wordLength) + "-letter word",
                                        "Add", inputWord);

      if (!gotInput) {
        continue;
      }

      inputWord = ToUpperWord(inputWord);

      if (!IsValidWord(inputWord, wordLength)) {
        ConfirmationDialog("Word must be exactly " + to_string(wordLength) +
                           " letters.");
        continue;
      }

      if (find(wordBank.begin(), wordBank.end(), inputWord) != wordBank.end()) {
        ConfirmationDialog("That word already exists.");
        continue;
      }

      wordBank.push_back(inputWord);
      sort(wordBank.begin(), wordBank.end());
      SaveWordBank(wordBank, wordLength);
      ConfirmationDialog("Word added.");
    } else if (action == WordBankDialogAction::RemoveWord) {
      string inputWord = "";
      bool gotInput = SingleDialog("Remove Word", "Word: ",
                                        to_string(wordLength) + "-letter word",
                                        "Remove", inputWord);

      if (!gotInput) {
        continue;
      }

      inputWord = ToUpperWord(inputWord);
      auto word = find(wordBank.begin(), wordBank.end(), inputWord);

      if (word == wordBank.end()) {
        ConfirmationDialog("Word not found.");
        continue;
      }

      wordBank.erase(word);
      SaveWordBank(wordBank, wordLength);
      ConfirmationDialog("Word removed.");
    } else {
      return;
    }
  }
}

// Delete players 

void ShowDeletePlayersDialog(vector<Player>& players) {
  const vector<string> titleLines = {
      "████▄  ▄▄▄▄▄ ▄▄    ▄▄▄▄▄ ▄▄▄▄▄▄ ▄▄▄▄▄   █████▄ ▄▄     ▄▄▄  ▄▄ ▄▄ ▄▄▄▄▄ ▄▄▄▄ ",
      "██  ██ ██▄▄  ██    ██▄▄    ██   ██▄▄    ██▄▄█▀ ██    ██▀██ ▀███▀ ██▄▄  ██▄█▄",
      "████▀  ██▄▄▄ ██▄▄▄ ██▄▄▄   ██   ██▄▄▄   ██     ██▄▄▄ ██▀██   █   ██▄▄▄ ██ ██ ",
  };
  int page = 0;

  while (true) {
    vector<Player> normalPlayers;

    for (const auto& player : players) {
      if (IsNormalPlayer(player)) {
        normalPlayers.push_back(player);
      }
    }

    sort(normalPlayers.begin(), normalPlayers.end(),
         [](const Player& a, const Player& b) {
           return a.username < b.username;
         });

    int totalPages = max(
        1, ((int)normalPlayers.size() + words_per_page - 1) / words_per_page);
    page = clamp(page, 0, totalPages - 1);
    int startIndex = page * words_per_page;
    int endIndex = min(startIndex + words_per_page, (int)normalPlayers.size());

    auto screen = ScreenInteractive::Fullscreen();
    DeletePlayersDialogAction action = DeletePlayersDialogAction::Back;

    auto previous = Button(compact_button_label("Previous"), [&] {
      action = DeletePlayersDialogAction::Previous;
      screen.ExitLoopClosure()();
    });

    auto next = Button(compact_button_label("Next"), [&] {
      action = DeletePlayersDialogAction::Next;
      screen.ExitLoopClosure()();
    });

    auto deletePlayer = Button(compact_button_label("Delete Player"), [&] {
      action = DeletePlayersDialogAction::DeletePlayer;
      screen.ExitLoopClosure()();
    });

    auto back = Button(compact_button_label("Back"), [&] {
      action = DeletePlayersDialogAction::Back;
      screen.ExitLoopClosure()();
    });

    auto controls = Container::Vertical({
        Container::Horizontal({previous, next}),
        Container::Horizontal({deletePlayer, back}),
    });

    auto renderer = Renderer(controls, [&] {
      Elements titleElements;
      for (const auto& line : titleLines) {
        titleElements.push_back(text(line) | center);
      }

      Elements lineElements;
      if (normalPlayers.empty()) {
        lineElements.push_back(text("No players to delete.") | center);
      } else {
        for (int i = startIndex; i < endIndex; i++) {
          lineElements.push_back(
              text(to_string(i + 1) + ". " + normalPlayers[i].username +
                   "  |  Wins: " + to_string(normalPlayers[i].wins) +
                   "  |  Streak: " + to_string(normalPlayers[i].streak)) |
              center);
        }
      }

      return vbox({
                 vbox(titleElements),
                 vertical_spacer(1),
                 separator(),
                 text("Page " + to_string(page + 1) + " / " +
                      to_string(totalPages)) |
                     center,
                 separator(),
                 vbox(lineElements) | center,
                 separator(),
                 hbox({
                     previous->Render(),
                     horizontal_spacer(3),
                     next->Render(),
                 }) | center,
                 hbox({
                     deletePlayer->Render(),
                     horizontal_spacer(3),
                     back->Render(),
                 }) | center,
             }) |
             border | center;
    });

    screen.Loop(renderer);

    if (action == DeletePlayersDialogAction::Previous) {
      if (page > 0) {
        page--;
      }
    } else if (action == DeletePlayersDialogAction::Next) {
      if (page + 1 < totalPages) {
        page++;
      }
    } else if (action == DeletePlayersDialogAction::DeletePlayer) {
      string username = "";
      bool gotInput = SingleDialog("Delete Player", "Username: ",
                                        "username", "Delete", username);

      if (!gotInput) {
        continue;
      }

      if (username == admin_username) {
        ConfirmationDialog("Admin cannot be deleted.");
        continue;
      }

      auto player = FindPlayerIteratorByUsername(players, username);
      if (player == players.end() || !IsNormalPlayer(*player)) {
        ConfirmationDialog("Player not found.");
        continue;
      }

      players.erase(player);
      SavePlayers(players);
      ConfirmationDialog("Player deleted.");
    } else {
      return;
    }
  }
}

// FTXUI - Game UI

string BuildPlayerLabel(const Player& player) {
  if (player.isGuest) {
    return "Playing as Guest";
  }

  if (player.isAdmin) {
    return "Admin: " + player.username;
  }

  return "Player: " + player.username;
}

string BuildModeLabel(bool dailyMode) {
  return dailyMode ? "Mode: Daily" : "Mode: Normal";
}

GameResult RunGameFTXUI(const vector<string>& wordBank,
                        Player& player,
                        bool dailyMode,
                        int wordLength) {
  GameResult result;
  result.won = false;
  result.quitToMenu = false;
  result.attemptsUsed = 0;

  if (wordBank.empty()) {
    result.quitToMenu = true;
    return result;
  }

  string target = "";
  if (dailyMode) {
    target = PickDailyWord(wordBank);
  } else {
    target = PickRandomWord(wordBank);
  }

  array<Row, max_rows> rows;

  for (int r = 0; r < max_rows; r++) {
    rows[r] = Row(wordLength);
    for (int c = 0; c < wordLength; c++) {
      rows[r].tiles[c].ch = ' ';
      rows[r].tiles[c].state = State::BLANK;
    }
  }

  int currentRow = 0;
  int currentCol = 0;
  bool gameOver = false;
  string message = "";

  while (!gameOver && !result.quitToMenu) {
    bool pauseRequested = false;
    auto screen = ScreenInteractive::Fullscreen();

    auto renderer = Renderer([&] {
      int gameBoxWidth = GetGameBoxWidth(wordLength);
      Elements board;
      for (int r = 0; r < max_rows; r++) {
        board.push_back(hbox(RenderTiles(rows[r])) | center);
      }

      Elements info = {
          paragraphAlignCenter(BuildPlayerLabel(player)) | center,
          paragraphAlignCenter(BuildModeLabel(dailyMode)) | center,
      };

      if (player.isAdmin) {
        info.push_back(paragraphAlignCenter("Target word: " + target) | bold |
                       center);
      }

      Element rulesBox = vbox({
                               text("Wordle++ Rules") | bold | center,
                               separator(),
                               text("Guess the " + to_string(wordLength) +
                                    "-letter word!") |
                                   center,
                               text("You have 6 tries!") | center,
                               vertical_spacer(1),
                               text("Green = right letter, right position") |
                                   center,
                               text("Yellow = right letter, wrong position") |
                                   center,
                               text("Red = letter not in word") | center,
                               vertical_spacer(1),
                               text("ESC = pause") | center,
                           }) |
                           border;

      Element boardBox = vbox({
                               vbox(info) | center,
                               separator(),
                               vbox(board) | center,
                               separator(),
                               paragraphAlignCenter(message) | center,
                           }) |
                           border | size(WIDTH, EQUAL, gameBoxWidth);

      Element rulesColumn = vbox({
                               rulesBox | size(WIDTH, EQUAL, rules_box_width),
                               filler(),
                           });

      return hbox({
                 horizontal_spacer(rules_box_width),
                 boardBox | flex,
                 horizontal_spacer(rules_box_gap),
                 rulesColumn,
             }) |
             center;
    });

    auto component = CatchEvent(renderer, [&](Event event) -> bool {
      if (gameOver) {
        if (event == Event::Return) {
          screen.ExitLoopClosure()();
          return true;
        }
        return true;
      }

      if (event == Event::Escape) {
        pauseRequested = true;
        screen.ExitLoopClosure()();
        return true;
      }

      if (event.is_character() && currentCol < wordLength) {
        string s = event.character();

        if (!s.empty()) {
          unsigned char uc = (unsigned char)s[0];
          if (isalpha(uc)) {
            rows[currentRow].tiles[currentCol].ch =
                (char)toupper((unsigned char)uc);
            currentCol++;
            return true;
          }
        }
      }

      if (event == Event::Backspace && currentCol > 0) {
        currentCol--;
        rows[currentRow].tiles[currentCol].ch = ' ';
        return true;
      }

      if (event == Event::Return && currentCol == wordLength) {
        string guess = RowToWord(rows[currentRow]);

        EvaluateGuess(rows[currentRow], target, wordLength);

        if (guess == target) {
          gameOver = true;
          result.won = true;
          result.attemptsUsed = currentRow + 1;
          message = "Press Enter to continue.";
        } else {
          currentRow++;
          currentCol = 0;

          if (currentRow >= max_rows) {
            gameOver = true;
            result.won = false;
            result.attemptsUsed = max_rows;
            message = "The word was: " + target + ". Press Enter to continue.";
          }
        }

        return true;
      }

      return false;
    });

    screen.Loop(component);

    if (pauseRequested) {
      result.quitToMenu = ShowPauseDialog();
    }
  }

  if (!result.quitToMenu && IsNormalPlayer(player)) {
    if (result.won) {
      player.wins++;
      player.streak++;
    } else {
      player.streak = 0;
    }
  }

  return result;
}

// Session helpers 

void PlaySessionLoop(Player& currentPlayer,
                     vector<Player>& players,
                     const vector<string>& wordBank,
                     bool dailyMode,
                     int wordLength) {
  while (true) {
    GameResult result =
        RunGameFTXUI(wordBank, currentPlayer, dailyMode, wordLength);

    if (result.quitToMenu) {
      return;
    }

    SaveCurrentPlayer(currentPlayer, players);

    AfterGameAction choice =
        ShowAfterGameOptions(currentPlayer.isGuest,
                             currentPlayer.isAdmin,
                             currentPlayer.username,
                             result.won,
                             result.attemptsUsed);

    if (choice != AfterGameAction::PlayAgain) {
      return;
    }
  }
}

GuestMenuAction GuestMenu() {
  auto screen = ScreenInteractive::Fullscreen();
  GuestMenuAction action = GuestMenuAction::Exit;

  auto playAsGuest = Button(wide_button_label("Play as Guest"), [&] {
    action = GuestMenuAction::PlayAsGuest;
    screen.ExitLoopClosure()();
  });

  auto login = Button(wide_button_label("Log in"), [&] {
    action = GuestMenuAction::Login;
    screen.ExitLoopClosure()();
  });

  auto signup = Button(wide_button_label("Sign up"), [&] {
    action = GuestMenuAction::SignUp;
    screen.ExitLoopClosure()();
  });

  auto leaderboard = Button(wide_button_label("Leaderboard"), [&] {
    action = GuestMenuAction::Leaderboard;
    screen.ExitLoopClosure()();
  });

  auto exit = Button(wide_button_label("Exit"), [&] {
    action = GuestMenuAction::Exit;
    screen.ExitLoopClosure()();
  });

  auto menu = Container::Vertical(
      {playAsGuest, login, signup, leaderboard, exit});

  auto renderer = Renderer(menu, [&] {
    return vbox({
               RenderTitle(),
               text("Not logged in") | center,
               vertical_spacer(1),
               playAsGuest->Render() | size(WIDTH, EQUAL, button_width) |
                   center,
               text("or") | bold | center,
               login->Render() | size(WIDTH, EQUAL, button_width) | center,
               signup->Render() | size(WIDTH, EQUAL, button_width) | center,
               leaderboard->Render() | size(WIDTH, EQUAL, button_width) |
                   center,
               exit->Render() | size(WIDTH, EQUAL, button_width) | center,
           }) |
           center;
  });

  screen.Loop(renderer);
  return action;
}

AdminMenuAction AdminMenu() {
  auto screen = ScreenInteractive::Fullscreen();
  AdminMenuAction action = AdminMenuAction::Exit;

  auto play = Button(wide_button_label("Play"), [&] {
    action = AdminMenuAction::Play;
    screen.ExitLoopClosure()();
  });

  auto daily = Button(wide_button_label("Play Daily Mode"), [&] {
    action = AdminMenuAction::Daily;
    screen.ExitLoopClosure()();
  });

  auto wordBank = Button(wide_button_label("Word Bank"), [&] {
    action = AdminMenuAction::WordBank;
    screen.ExitLoopClosure()();
  });

  auto deletePlayers = Button(wide_button_label("Delete Player"), [&] {
    action = AdminMenuAction::DeletePlayers;
    screen.ExitLoopClosure()();
  });

  auto leaderboard = Button(wide_button_label("Leaderboard"), [&] {
    action = AdminMenuAction::Leaderboard;
    screen.ExitLoopClosure()();
  });

  auto logout = Button(wide_button_label("Log Out"), [&] {
    action = AdminMenuAction::Logout;
    screen.ExitLoopClosure()();
  });

  auto exit = Button(wide_button_label("Exit"), [&] {
    action = AdminMenuAction::Exit;
    screen.ExitLoopClosure()();
  });

  auto menu = Container::Vertical(
      {play, daily, wordBank, deletePlayers, leaderboard, logout, exit});

  auto renderer = Renderer(menu, [&] {
    return vbox({
               RenderTitle(),
               separator(),
               text("Logged in as: admin") | bold | center,
               vertical_spacer(1),
               play->Render() | size(WIDTH, EQUAL, button_width) | center,
               daily->Render() | size(WIDTH, EQUAL, button_width) | center,
               wordBank->Render() | size(WIDTH, EQUAL, button_width) | center,
               deletePlayers->Render() | size(WIDTH, EQUAL, button_width) |
                   center,
               leaderboard->Render() | size(WIDTH, EQUAL, button_width) |
                   center,
               logout->Render() | size(WIDTH, EQUAL, button_width) | center,
               exit->Render() | size(WIDTH, EQUAL, button_width) | center,
           }) |
           center;
  });

  screen.Loop(renderer);
  return action;
}

PlayerMenuAction PlayerMenu(const string& username) {
  auto screen = ScreenInteractive::Fullscreen();
  PlayerMenuAction action = PlayerMenuAction::Exit;

  auto play = Button(wide_button_label("Play"), [&] {
    action = PlayerMenuAction::Play;
    screen.ExitLoopClosure()();
  });

  auto daily = Button(wide_button_label("Play Daily Mode"), [&] {
    action = PlayerMenuAction::Daily;
    screen.ExitLoopClosure()();
  });

  auto leaderboard = Button(wide_button_label("Leaderboard"), [&] {
    action = PlayerMenuAction::Leaderboard;
    screen.ExitLoopClosure()();
  });

  auto logout = Button(wide_button_label("Log Out"), [&] {
    action = PlayerMenuAction::Logout;
    screen.ExitLoopClosure()();
  });

  auto exit = Button(wide_button_label("Exit"), [&] {
    action = PlayerMenuAction::Exit;
    screen.ExitLoopClosure()();
  });

  auto menu = Container::Vertical({play, daily, leaderboard, logout, exit});

  auto renderer = Renderer(menu, [&] {
    return vbox({
               RenderTitle(),
               separator(),
               text("Logged in as: " + username) | bold | center,
               vertical_spacer(1),
               play->Render() | size(WIDTH, EQUAL, button_width) | center,
               daily->Render() | size(WIDTH, EQUAL, button_width) | center,
               leaderboard->Render() | size(WIDTH, EQUAL, button_width) |
                   center,
               logout->Render() | size(WIDTH, EQUAL, button_width) | center,
               exit->Render() | size(WIDTH, EQUAL, button_width) | center,
           }) |
           center;
  });

  screen.Loop(renderer);
  return action;
}

void HandleLogin(Player& currentPlayer, vector<Player>& players) {
  string username = "";
  string password = "";

  if (!LoginDialog(username, password)) {
    return;
  }

  players = LoadPlayers();

  if (username == admin_username && password == admin_password) {
    currentPlayer = MakeAdminPlayer();
    ConfirmationDialog("Logged in as admin.");
    return;
  }

  Player loggedInPlayer;
  if (CheckNormalLogin(players, username, password, loggedInPlayer)) {
    currentPlayer = loggedInPlayer;
    ConfirmationDialog("Logged in as: " + currentPlayer.username);
  } else {
    ConfirmationDialog("Invalid username or password.");
  }
}

void HandleSignUp(Player& currentPlayer, vector<Player>& players) {
  string username = "";
  string password = "";

  if (!SignupDialog(username, password)) {
    return;
  }

  players = LoadPlayers();

  if (!IsValidUsername(username)) {
    ConfirmationDialog(
        "Invalid username. Letters, numbers, underscores only.");
    return;
  }

  if (UsernameTaken(players, username)) {
    ConfirmationDialog("Username already taken.");
    return;
  }

  Player newPlayer = MakeRegisteredPlayer(username, password);
  players.push_back(newPlayer);
  SavePlayers(players);
  currentPlayer = newPlayer;
  ConfirmationDialog("Account created. Logged in as: " + currentPlayer.username);
}

void StartCheck(Player& currentPlayer,
                             vector<Player>& players,
                             bool dailyMode,
                             int wordLength) {
  vector<string> wordBank = LoadWordBank(wordLength);

  if (wordBank.empty()) {
    ConfirmationDialog(dailyMode ? "The 5-letter word bank is empty."
                                 : "That word bank is empty.");
    return;
  }

  PlaySessionLoop(currentPlayer, players, wordBank, dailyMode, wordLength);
}

// Main loop

int RunApp() {
  vector<Player> players = LoadPlayers();
  Player currentPlayer = MakeGuestPlayer();

  while (true) {
    if (currentPlayer.isGuest) {
      switch (GuestMenu()) {
        case GuestMenuAction::Login:
          HandleLogin(currentPlayer, players);
          break;
        case GuestMenuAction::SignUp:
          HandleSignUp(currentPlayer, players);
          break;
        case GuestMenuAction::PlayAsGuest:
          currentPlayer = MakeGuestPlayer();
          {
            int wordLength = ShowPlayModeDialog();
            if (wordLength != 0) {
              StartCheck(currentPlayer, players, false,
                                      wordLength);
            }
          }
          break;
        case GuestMenuAction::Leaderboard:
          players = LoadPlayers();
          ShowLeaderboardDialog(players);
          break;
        case GuestMenuAction::Exit:
          return 0;
      }
      continue;

    } else if (currentPlayer.isAdmin) {
      switch (AdminMenu()) {
        case AdminMenuAction::Play:
          {
            int wordLength = ShowPlayModeDialog();
            if (wordLength != 0) {
              StartCheck(currentPlayer, players, false,
                                      wordLength);
            }
          }
          break;
        case AdminMenuAction::Daily:
          StartCheck(currentPlayer, players, true, 5);
          break;
        case AdminMenuAction::WordBank: {
          int wordLength = ShowWordBankModeDialog();
          if (wordLength != 0) {
            vector<string> wordBank = LoadWordBank(wordLength);
            ShowWordBankDialog(wordBank, wordLength);
          }
          break;
        }
        case AdminMenuAction::DeletePlayers:
          players = LoadPlayers();
          ShowDeletePlayersDialog(players);
          players = LoadPlayers();
          break;
        case AdminMenuAction::Leaderboard:
          players = LoadPlayers();
          ShowLeaderboardDialog(players);
          break;
        case AdminMenuAction::Logout:
          currentPlayer = MakeGuestPlayer();
          ConfirmationDialog("Logged out.");
          break;
        case AdminMenuAction::Exit:
          return 0;
      }
      continue;

    } else {
      switch (PlayerMenu(currentPlayer.username)) {
        case PlayerMenuAction::Play:
          {
            int wordLength = ShowPlayModeDialog();
            if (wordLength != 0) {
              StartCheck(currentPlayer, players, false,
                                      wordLength);
            }
          }
          break;
        case PlayerMenuAction::Daily:
          StartCheck(currentPlayer, players, true, 5);
          break;
        case PlayerMenuAction::Leaderboard:
          players = LoadPlayers();
          ShowLeaderboardDialog(players);
          break;
        case PlayerMenuAction::Logout:
          currentPlayer = MakeGuestPlayer();
          ConfirmationDialog("Logged out.");
          break;
        case PlayerMenuAction::Exit:
          return 0;
      }
      continue;
    }
  }

  return 0;
}
