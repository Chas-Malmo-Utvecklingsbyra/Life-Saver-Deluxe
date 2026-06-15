#ifndef COMMANDS_H
#define COMMANDS_H

/**
 * @brief Checks the fragmentation of the program
 */
int Command_Handle_Fragment(int argc, char **argv);

/**
 * @brief Gives information about all created tasks
 */
int Command_Handle_Tasks(int argc, char **argv);

/**
 * @brief Prints out how long ESP32 has been running
 */
int Command_Handle_Uptime(int argc, char **argv);

/**
 * @brief Restarts the ESP32
 */
int Command_Handle_Restart(int argc, char **argv);

#endif