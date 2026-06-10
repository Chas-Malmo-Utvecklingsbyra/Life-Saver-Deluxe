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

char* Packet_Build(Packet_Job job, const char* message);

char* Packet_Job_To_String(Packet_Job job);

Packet_Job Packet_Job_From_String(const char* string);

#endif