#include <conio.h>
#include <stdio.h>
#include <windows.h>

#include "line_editor.h"

static HANDLE hConsole;
static CONSOLE_SCREEN_BUFFER_INFO originalConsoleInfo;

void terminal_raw_mode_enter(void) {
  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleScreenBufferInfo(hConsole, &originalConsoleInfo);

  // Enable ANSI escape sequence processing on Windows 10+
  DWORD mode;
  if (GetConsoleMode(hConsole, &mode)) {
    SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
}

void terminal_raw_mode_exit(void) {
  // Restore original console state if needed
  // (Windows console doesn't need explicit restoration like termios)
}

// Windows Console API terminal control functions
void terminal_clear_eol(void) {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hConsole, &csbi);

  COORD coord = csbi.dwCursorPosition;
  DWORD charsWritten;
  DWORD length = csbi.dwSize.X - coord.X;

  FillConsoleOutputCharacter(hConsole, ' ', length, coord, &charsWritten);
  SetConsoleCursorPosition(hConsole, coord);
}

void terminal_cursor_left(void) {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hConsole, &csbi);

  if (csbi.dwCursorPosition.X > 0) {
    COORD newPos = {csbi.dwCursorPosition.X - 1, csbi.dwCursorPosition.Y};
    SetConsoleCursorPosition(hConsole, newPos);
  }
}

void terminal_cursor_right(void) {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hConsole, &csbi);

  if (csbi.dwCursorPosition.X < csbi.dwSize.X - 1) {
    COORD newPos = {csbi.dwCursorPosition.X + 1, csbi.dwCursorPosition.Y};
    SetConsoleCursorPosition(hConsole, newPos);
  }
}

void terminal_show_cursor(void) {
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = TRUE;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void terminal_hide_cursor(void) {
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = FALSE;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Parse Windows key input using _getch()
key_event_t parse_key_sequence(void) {
  key_event_t event = {0};
  int c = _getch();

  if (c == '\r') {
    event.type = KEY_ENTER;
    return event;
  }

  if (c == '\b') {
    event.type = KEY_BACKSPACE;
    return event;
  }

  if (c == 0 || c == 224) {  // Extended key prefix on Windows
    // Read the extended key code
    c = _getch();
    switch (c) {
      case 72:  // Up arrow
        event.type = KEY_UP;
        break;
      case 80:  // Down arrow
        event.type = KEY_DOWN;
        break;
      case 75:  // Left arrow
        event.type = KEY_LEFT;
        break;
      case 77:  // Right arrow
        event.type = KEY_RIGHT;
        break;
      case 71:  // Home
        event.type = KEY_HOME;
        break;
      case 79:  // End
        event.type = KEY_END;
        break;
      case 83:  // Delete
        event.type = KEY_DELETE;
        break;
      default:
        // Unknown extended key - treat as normal
        event.type = KEY_NORMAL;
        event.character = c;
        break;
    }
    return event;
  }

  // Normal character
  event.type = KEY_NORMAL;
  event.character = c;
  return event;
}