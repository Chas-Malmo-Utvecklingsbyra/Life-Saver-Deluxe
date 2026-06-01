#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>

typedef int (*Console_Command_On_Called)(int argc, char **argv);

typedef struct
{
    const char* command;
    const char* help;
    const char* hint;
    Console_Command_On_Called callback;
} Console_Command;

typedef struct
{
    Console_Command* commands;
    size_t commands_length;
    size_t commands_capacity;
} Console;

bool Console_Command_Add(Console_Command command);
void Console_Initialize();
void Console_Dispose();

#endif