#ifndef ERROR_H
#define ERROR_H

#include "context.h"

// Main error handling function
void error(context_t* ctx, const char* fmt, ...);

#endif  // ERROR_H