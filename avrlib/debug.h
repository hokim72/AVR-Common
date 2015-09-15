// This file contains a set of functions which may be useful for general
// debugging.

#ifndef DEBUG_H
#define DEBUG_H

#include "global.h"
#include <avr/io.h>

// function prototypes

// Print a part of memory as a formatted hex table which ascii translation
void debugPrintHexTable(uint16_t length, uint8_t* buffer);

#endif
