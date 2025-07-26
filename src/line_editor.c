#include "line_editor.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Global history buffer - simple and accessible
static history_buffer_t command_history = {0};

// History management functions
void history_init(history_buffer_t *hist) {
  hist->current_entry = -1;
  hist->total_entries = 0;
  hist->viewing_entry = -1;
}

void history_add(history_buffer_t *hist, const char *line) {
  // Skip empty lines
  if (!line || strlen(line) == 0) {
    return;
  }

  // Skip if same as most recent entry
  if (hist->total_entries > 0) {
    int recent_idx = hist->current_entry;
    if (strcmp(hist->history[recent_idx], line) == 0) {
      return;
    }
  }

  // Add to circular buffer
  hist->current_entry = (hist->current_entry + 1) % HISTORY_SIZE;
  strncpy(hist->history[hist->current_entry], line, INPUT_BUFFER_SIZE - 1);
  hist->history[hist->current_entry][INPUT_BUFFER_SIZE - 1] = '\0';

  if (hist->total_entries < HISTORY_SIZE) {
    hist->total_entries++;
  }

  // Reset view to current
  hist->viewing_entry = -1;
}

const char *history_get_previous(history_buffer_t *hist) {
  if (hist->total_entries == 0) {
    return NULL;
  }

  if (hist->viewing_entry == -1) {
    // First time going back - start at most recent
    hist->viewing_entry = hist->current_entry;
  } else {
    // Go further back
    int prev_entry;
    if (hist->total_entries < HISTORY_SIZE) {
      // Haven't filled buffer yet
      if (hist->viewing_entry > 0) {
        prev_entry = hist->viewing_entry - 1;
      } else {
        return NULL;  // At oldest entry
      }
    } else {
      // Buffer is full, wrap around
      prev_entry = (hist->viewing_entry - 1 + HISTORY_SIZE) % HISTORY_SIZE;
      if (prev_entry == hist->current_entry) {
        return NULL;  // Back to newest, don't wrap
      }
    }
    hist->viewing_entry = prev_entry;
  }

  return hist->history[hist->viewing_entry];
}

const char *history_get_next(history_buffer_t *hist) {
  if (hist->viewing_entry == -1) {
    return NULL;  // Already at current line
  }

  int next_entry;
  if (hist->total_entries < HISTORY_SIZE) {
    // Haven't filled buffer yet
    if (hist->viewing_entry < hist->current_entry) {
      next_entry = hist->viewing_entry + 1;
    } else {
      hist->viewing_entry = -1;
      return NULL;  // Back to current line
    }
  } else {
    // Buffer is full
    next_entry = (hist->viewing_entry + 1) % HISTORY_SIZE;
    if (next_entry == (hist->current_entry + 1) % HISTORY_SIZE) {
      hist->viewing_entry = -1;
      return NULL;  // Back to current line
    }
  }

  hist->viewing_entry = next_entry;
  return hist->history[hist->viewing_entry];
}

void history_reset_view(history_buffer_t *hist) { hist->viewing_entry = -1; }

// Line buffer management functions
static void insert_char_at_cursor(line_buffer_t *line, char c) {
  if (line->length >= INPUT_BUFFER_SIZE - 1) {
    return;  // Buffer full
  }

  // Shift characters right from cursor position
  for (size_t i = line->length; i > line->cursor_pos; i--) {
    line->buffer[i] = line->buffer[i - 1];
  }

  // Insert new character
  line->buffer[line->cursor_pos] = c;
  line->length++;
  line->cursor_pos++;
}

static void delete_char_at_cursor(line_buffer_t *line) {
  if (line->cursor_pos == 0) {
    return;  // Nothing to delete
  }

  // Shift characters left from cursor position
  for (size_t i = line->cursor_pos - 1; i < line->length - 1; i++) {
    line->buffer[i] = line->buffer[i + 1];
  }

  line->length--;
  line->cursor_pos--;
}

static void clear_line_display(line_buffer_t *line) {
  // Move cursor to start of line
  while (line->cursor_pos > 0) {
    terminal_cursor_left();
    line->cursor_pos--;
  }

  // Clear to end of line
  terminal_clear_eol();
  fflush(stdout);
}

static void load_history_into_line(line_buffer_t *line, const char *hist_text) {
  // Clear current line display
  clear_line_display(line);

  // Load history text
  size_t hist_len = strlen(hist_text);
  if (hist_len >= INPUT_BUFFER_SIZE) {
    hist_len = INPUT_BUFFER_SIZE - 1;
  }

  memcpy(line->buffer, hist_text, hist_len);
  line->buffer[hist_len] = '\0';
  line->length = hist_len;
  line->cursor_pos = hist_len;

  // Display the text
  printf("%s", line->buffer);
  fflush(stdout);
}

static void redraw_from_cursor(line_buffer_t *line) {
  // Clear from cursor to end of line
  terminal_clear_eol();

  // Print remaining characters from cursor position
  printf("%.*s", (int)(line->length - line->cursor_pos),
         line->buffer + line->cursor_pos);

  // Move cursor back to original position
  size_t chars_printed = line->length - line->cursor_pos;
  for (size_t i = 0; i < chars_printed; i++) {
    terminal_cursor_left();
  }

  fflush(stdout);
}

static void move_cursor_left(line_buffer_t *line) {
  if (line->cursor_pos > 0) {
    line->cursor_pos--;
    terminal_cursor_left();
    fflush(stdout);
  }
}

static void move_cursor_right(line_buffer_t *line) {
  if (line->cursor_pos < line->length) {
    line->cursor_pos++;
    terminal_cursor_right();
    fflush(stdout);
  }
}

static void move_cursor_to_start(line_buffer_t *line) {
  while (line->cursor_pos > 0) {
    line->cursor_pos--;
    terminal_cursor_left();
  }
  fflush(stdout);
}

static void move_cursor_to_end(line_buffer_t *line) {
  while (line->cursor_pos < line->length) {
    line->cursor_pos++;
    terminal_cursor_right();
  }
  fflush(stdout);
}

static void handle_backspace(line_buffer_t *line) {
  if (line->cursor_pos > 0) {
    delete_char_at_cursor(line);
    // Move cursor back and redraw from new position
    terminal_cursor_left();
    redraw_from_cursor(line);
  }
}

static void delete_char_forward(line_buffer_t *line) {
  if (line->cursor_pos >= line->length) {
    return;  // Nothing to delete (cursor at end)
  }

  // Shift characters left from cursor position
  for (size_t i = line->cursor_pos; i < line->length - 1; i++) {
    line->buffer[i] = line->buffer[i + 1];
  }

  line->length--;
  // Note: cursor_pos stays the same for forward delete
}

static void handle_delete(line_buffer_t *line) {
  if (line->cursor_pos < line->length) {
    delete_char_forward(line);
    // Redraw from current position (cursor doesn't move)
    redraw_from_cursor(line);
  }
}

static void handle_key_event(line_buffer_t *line, key_event_t event) {
  switch (event.type) {
    case KEY_NORMAL:
      if (event.character >= 32 && event.character < 127) {
        // When user starts typing, reset history view
        history_reset_view(&command_history);

        insert_char_at_cursor(line, event.character);
        // Print the character and redraw rest of line
        putchar(event.character);
        redraw_from_cursor(line);
      }
      break;

    case KEY_LEFT:
      move_cursor_left(line);
      break;

    case KEY_RIGHT:
      move_cursor_right(line);
      break;

    case KEY_UP: {
      const char *hist_text = history_get_previous(&command_history);
      if (hist_text) {
        load_history_into_line(line, hist_text);
      }
      break;
    }

    case KEY_DOWN: {
      const char *hist_text = history_get_next(&command_history);
      if (hist_text) {
        load_history_into_line(line, hist_text);
      } else {
        // Back to current (empty) line
        clear_line_display(line);
        line->length = 0;
        line->cursor_pos = 0;
      }
      break;
    }

    case KEY_HOME:
      move_cursor_to_start(line);
      break;

    case KEY_END:
      move_cursor_to_end(line);
      break;

    case KEY_BACKSPACE:
      // When user edits, reset history view
      history_reset_view(&command_history);
      handle_backspace(line);
      break;

    case KEY_DELETE:
      // When user edits, reset history view
      history_reset_view(&command_history);
      handle_delete(line);
      break;

    case KEY_ENTER:
      // Line complete - handled by caller
      break;
  }
}

// Main line editing function
void enhanced_get_line(char *buffer, size_t max_len) {
  line_buffer_t line = {0};

  // Initialize history on first use
  static bool history_initialized = false;
  if (!history_initialized) {
    history_init(&command_history);
    history_initialized = true;
  }

  terminal_raw_mode_enter();

  while (1) {
    key_event_t event = parse_key_sequence();

    if (event.type == KEY_ENTER) {
      break;
    }

    handle_key_event(&line, event);
  }

  terminal_raw_mode_exit();

  // Copy result to output buffer
  size_t copy_len = (line.length < max_len - 1) ? line.length : max_len - 1;
  memcpy(buffer, line.buffer, copy_len);
  buffer[copy_len] = '\0';

  // Add to history if non-empty
  if (copy_len > 0) {
    history_add(&command_history, buffer);
  }

  // Reset history view to current line
  history_reset_view(&command_history);

  printf("\n");
  fflush(stdout);
}