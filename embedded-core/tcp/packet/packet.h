#ifndef PACKET_H
#define PACKET_H

typedef enum
{
    Packet_Job_Initialize,
    Packet_Job_Heartbeat,
    Packet_Job_Data
} Packet_Job;

char* Packet_Build(Packet_Job job, const char* message);

#endif