#ifndef UARTIO_H
#define UARTIO_H
#include "global.h"
#include <stdio.h>

void uartIOPutchar(char c, FILE *stream);
char uartIOGetchar(FILE *stream);
void uartIOInit(void);

extern FILE uartIOOutput;
extern FILE uartIOInput;

#endif
