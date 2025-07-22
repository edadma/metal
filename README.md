# Metal

A modern systems programming language for embedded development and hardware prototyping.

## What is Metal?

Metal combines the directness and hardware control of Forth with modern conveniences like automatic memory management, rich data types, and Unicode support. It's designed for interactive development on microcontrollers - write code, test immediately, deploy the same code as firmware.

**Key Features:**
- **Interactive development** - REPL-driven programming with immediate feedback
- **Memory safety** - Reference counting with deterministic cleanup, no garbage collector
- **Rich data types** - Strings, objects, arrays, and first-class functions
- **Hardware integration** - Direct GPIO, interrupt handlers, real-time control
- **Unicode support** - UTF-32 strings for international text
- **Dual-core friendly** - Clean execution contexts for multi-threading

## Design Philosophy

Metal keeps Forth's "no syntax" approach - just words separated by spaces that manipulate a stack. But it adds modern data types that make real-world programming practical:

```metal
"Hello world" PRINT
{} 42 : x 3.14 : y CONSTANT point
[] 1 , 2 , 3 , { DUP * } MAP
```

## Core Concepts

### Cells
Everything in Metal is a 12-byte cell that can hold:
- 32-bit or 64-bit integers
- Double-precision floats  
- UTF-32 strings (1-2 characters embedded, longer strings allocated)
- Objects with properties
- Arrays of mixed types
- Function code
- Managed pointers

### Stack-Based Execution
Like Forth, Metal uses postfix notation and stack manipulation:
```metal
5 3 +        \ 5 + 3 = 8
DUP *        \ Square top of stack
```

### Memory Management
Reference counting happens automatically. No manual malloc/free, no garbage collection pauses:
```metal
"temp string" process DROP    \ String automatically freed
```

### Object System
Simple objects without inheritance or complex dispatch:
```metal
{} 42 : x 3.14 : y CONSTANT point
point . x PRINT               \ Access property x
```

## Target Platform

Initially targeting the Raspberry Pi Pico for prototyping, with plans for other microcontrollers. Metal is designed for resource-constrained environments where you need both performance and convenience.

## Status

**Early Development** - Core interpreter and cell system in progress.

## Goals

- Replace the edit-compile-flash-debug cycle with interactive development
- Make embedded programming accessible without sacrificing control
- Provide a unified environment from prototyping to production firmware
- Support international users with proper Unicode handling

## Why Metal?

Because embedded development should be **direct** (close to hardware), **safe** (memory managed), and **interactive** (immediate feedback). Metal gives you all three without compromise.

---

*Metal: Programming close to the metal, without the sharp edges.*
