#ifndef FLOAT_H
#define FLOAT_H

/**
 * Floating Point Math Module
 *
 * Provides trigonometric, exponential, logarithmic, and other
 * mathematical functions operating on floating point numbers.
 *
 * All functions accept any numeric type (INT32, INT64, FLOAT)
 * and return CELL_FLOAT results.
 */

// Register all floating point words with the dictionary
void add_float_words(void);

#endif  // FLOAT_H