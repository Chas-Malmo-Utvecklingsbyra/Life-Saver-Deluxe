#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#ifndef TCP_SERVER_BACKLOG
    #define TCP_SERVER_BACKLOG 5
#endif

#include <stdint.h>

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/inet.h>


typedef struct
{
    int socket;
    struct sockaddr_in server_addr;
} TCP_Server;

typedef enum
{
    TCP_Server_Error_Socket,
    TCP_Server_Error_Bind,
    TCP_Server_Error_Listen,
    TCP_Server_Success
} TCP_Server_Error;

TCP_Server_Error TCP_Server_Setup(TCP_Server *out_server, uint16_t port);

#endif