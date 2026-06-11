#ifndef PACKET_H
#define PACKET_H

typedef enum
{
    Packet_Job_Initialize,
    Packet_Job_Heartbeat,
    Packet_Job_Data,
    Packet_Job_Debug,
    Packet_Job_Acknowledge,
    Packet_Job_Unknown
} Packet_Job;

/**
 * 
 * @brief Builds a Packet for commmunicating with our ESP32S3 via TCP
 * 
 * This Packet_Build uses the Arena allocator, which means that it needs to be
 * cleared via Arena_Reset() after usage to not harm other areas of the code.
 *
 * @param job What type of Job to send.
 * @param message The data which should be put into the packet and sent.
 * 
 * @return Returns a char* to the Packet in the arena allocator memory.
 * @warning Arena_Reset() should be used to clear the memory after having used the packet. DO NOT USE FREE!
 *
 * 
 */
char* Packet_Build(Packet_Job job, const char* message);

/**
 * 
 * @brief Converts the Packet_Job type to the string version
 * 
 *
 * @param job The Packet_Job type to convert.
 * 
 * @return Returns the string version of Packet_Job
 * 
 * @note The return value should NOT need be freed
 * 
 */
char* Packet_Job_To_String(Packet_Job job);

/**
 * 
 * @brief Converts the string to a Packet_Job enumerable
 * 
 * This does the opposite of Packet_Job_To_String.
 *
 * @param string The string to convert to Packet_Job.
 * 
 * @return Returns the Packet_Job enumerable corresponding to the string.
 * 
 */
Packet_Job Packet_Job_From_String(const char* string);

#endif