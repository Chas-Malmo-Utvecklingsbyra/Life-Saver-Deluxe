#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdio.h>

typedef enum 
{
    File_System_Success,
    File_System_Error_File,
    File_System_Error_File_Write,
    File_System_Error_File_Read,
    File_System_Error_Spiffs,
    File_System_Error_Unknown // Worst case scenario
} File_System_Error;

typedef enum
{
    File_System_Type_Spiffs,
} File_System_Type;

typedef struct
{
    File_System_Type type;
    const char *path;
} File_System;


/**
 * 
 * @brief Initializes the whole Filesystem for saving files.
 *
 * @param file_system Initializes File_System struct with values.
 * @param type The type of File_System which could be SPIFFS or FAT (which is not implemented).
 *
 * @return Returns a File_System_Success on success but will return any other enumerator that fits the error best.
 *
 * @note FAT is not yet implemented.
 * @warning 
 * Spiffs does not support sub-directories and 
 * when the chip experiences a power loss during a file system operation it could result in SPIFFS corruption, 
 * which may be able to be recovered.
 *
 */
File_System_Error File_System_Initialize(File_System *file_system, File_System_Type type);


/**
 * 
 * @brief Writes to a file in the File System.
 * 
 *
 * @param file_system Uses the information from File_System struct to set path.
 * @param file_name The name of the file to write to. (sub-directories are not allowed in path when using SPIFFS).
 * @param text The to write to the file.
 *
 * @return Returns a File_System_Success on success but will return any other enumerator that fits the error best.
 *
 * @warning 
 * When using SPIFFS the file_name is not allowed to have any paths and will create wrong file name.
 *
 */
File_System_Error File_System_Write_File(File_System *file_system, const char *file_name, const char *text);

/**
 * 
 * @brief Reads a file in the File System.
 * 
 *
 * @param file_system Uses the information from File_System struct to set path.
 * @param file_name The name of the file to read.
 * @param out_buffer Pointer to the buffer that is gonna take the data from the file.
 * @param length The size of the out_buffer, this is to ensure there is no buffer overflow when trying to read.
 *
 * @return Returns a File_System_Success on success but will return any other enumerator that fits the error best.
 *
 */
File_System_Error File_System_Read_File(File_System *file_system, const char* file_name, char* out_buffer, size_t length);

#endif