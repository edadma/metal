#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "line_editor.h"
#include "pico/time.h"

static int original_stdin_flags = 0;
static bool non_blocking_active = false;

// Default do-nothing maintenance function
static void default_maintenance(void) {
  // Do nothing - maintains current behavior
}

// Global maintenance callback (set to default)
maintenance_callback_t g_maintenance_callback = default_maintenance;

// Function to set maintenance callback
void set_maintenance_callback(maintenance_callback_t callback) {
  g_maintenance_callback = callback ? callback : default_maintenance;
}

// Non-blocking getchar replacement with maintenance callback support
static int getchar_with_maintenance(void) {
  int c;
  while ((c = fgetc(stdin)) == EOF) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      return EOF;  // Real error occurred
    }
    // No input available, call maintenance function
    g_maintenance_callback();
    // Small delay to avoid busy-waiting
    sleep_ms(1);  // 1ms delay
  }
  return c;
}

// Pico doesn't need terminal mode changes - USB serial is already "raw"
// But we still need to set up non-blocking I/O for the maintenance callback
void terminal_raw_mode_enter(void) {
  if (non_blocking_active) return;

  // Save original stdin flags and set non-blocking mode
  original_stdin_flags = fcntl(STDIN_FILENO, F_GETFL);
  fcntl(STDIN_FILENO, F_SETFL, original_stdin_flags | O_NONBLOCK);

  non_blocking_active = true;
}

void terminal_raw_mode_exit(void) {
  if (!non_blocking_active) return;

  // Restore original stdin flags (removes non-blocking mode)
  fcntl(STDIN_FILENO, F_SETFL, original_stdin_flags);

  non_blocking_active = false;
}

void terminal_clear_eol(void) { printf("\033[K"); }

void terminal_cursor_left(void) { printf("\033[D"); }

void terminal_cursor_right(void) { printf("\033[C"); }

void terminal_show_cursor(void) { printf("\033[?25h"); }

void terminal_hide_cursor(void) { printf("\033[?25l"); }

// Parse Pico escape sequences
key_event_t parse_key_sequence(void) {
  key_event_t event = {0};
  int c = getchar_with_maintenance();

  if (c == EOF) {
    event.type = KEY_ENTER;
    return event;
  }

  if (c == '\r' || c == '\n') {
    event.type = KEY_ENTER;
    return event;
  }

  if (c == '\b' || c == 127) {
    event.type = KEY_BACKSPACE;
    return event;
  }

  if (c == '\033') {  // ESC
    // Read '['
    c = getchar_with_maintenance();
    if (c == '[') {
      // Read the command character
      c = getchar_with_maintenance();
      switch (c) {
        case 'A':
          event.type = KEY_UP;
          break;
        case 'B':
          event.type = KEY_DOWN;
          break;
        case 'D':
          event.type = KEY_LEFT;
          break;
        case 'C':
          event.type = KEY_RIGHT;
          break;
        case 'H':
          event.type = KEY_HOME;
          break;
        case 'F':
          event.type = KEY_END;
          break;
        case '1':
          // Handle sequences like ESC[1~ (Home key variant)
          c = getchar_with_maintenance();
          if (c == '~') {
            event.type = KEY_HOME;
          } else {
            event.type = KEY_NORMAL;
            event.character = c;
          }
          break;
        case '3':
          // Handle sequences like ESC[3~ (Delete key)
          c = getchar_with_maintenance();
          if (c == '~') {
            event.type = KEY_DELETE;
          } else {
            event.type = KEY_NORMAL;
            event.character = c;
          }
          break;
        case '4':
          // Handle sequences like ESC[4~ (End key variant)
          c = getchar_with_maintenance();
          if (c == '~') {
            event.type = KEY_END;
          } else {
            event.type = KEY_NORMAL;
            event.character = c;
          }
          break;
        default:
          event.type = KEY_NORMAL;
          event.character = c;
          break;
      }
    } else {
      event.type = KEY_NORMAL;
      event.character = c;
    }
    return event;
  }

  // Normal character
  event.type = KEY_NORMAL;
  event.character = c;
  return event;
}