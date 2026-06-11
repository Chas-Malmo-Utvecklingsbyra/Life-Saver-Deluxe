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


/**
 * 
 * @brief Add commands to the Console
 * 
 * This function adds useable Commands to the Console. 
 * Commands should always be added BEFORE initializing the Console.
 * 
 * @param command The Console_Command to add to the command list
 * 
 * @return Returns true on Success and false on Failed!
 *
 * @note Adding Commands will realloc the dynamically allocated "list" to fit the size of the command.
 * 
 */
bool Console_Command_Add(Console_Command command);

/**
 * 
 * @brief Initialize the Console
 * 
 * This Initialize the Console to take in input for debugging purposes.
 * This communicates using UART.
 * 
 * @warning Console_Command_Add should be used before using Console_Initialize()
s * 
 */
void Console_Initialize();

/**
 * 
 * @brief Dispose the internal Console struct
 * 
 * This is mostly used to dispose the internal Console struct
 * to then Initialize it again. This does NOT need to be use at all
 * as this lives out the full program. 
 * 
 * @param data The data to send to the client.
 * @param length The length of the data which should be sent to the client.
 * 
 * @return TCP_Client_Error depending on the result of the action. TCP_Client_Success for success!
 *
 * @note Return values may be TCP_Client_Success, TCP_Client_Error_Send or TCP_Client_Error_Socket
 * 
 */
void Console_Dispose();

#endif