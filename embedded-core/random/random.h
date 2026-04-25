#ifndef RANDOM_H
#define RANDOM_H

#define RANDOM_MAX_UUID_V4_LENGTH 37

/**
 * @brief This method generates a UUID (v4)
 *
 * @param uuid_buffer Should always have the size of RANDOM_MAX_UUID_V4_LENGTH.
 *
 *
 */
void Random_Generate_UUID_v4(char *uuid_buffer);

#endif