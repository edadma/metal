#ifndef REPL_H
#define REPL_H

#include "compat.h"
#include "context.h"

// Main REPL system
void repl(context_t* ctx);

// REPL control primitives
void native_quit(context_t* ctx);  // QUIT ( -- ) Restart REPL loop
void native_bye(context_t* ctx);   // BYE ( -- ) Exit system

#endif  // REPL_H