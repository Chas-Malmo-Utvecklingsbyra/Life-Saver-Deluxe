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
    char data[1024];
    uint16_t len;
    int socket;
} TCP_Server_Client;

typedef void (*TCP_Full_Data_Received)(TCP_Server_Client* client);

typedef struct
{
    int socket;
    struct sockaddr_in server_addr;
    TCP_Full_Data_Received callback; // This is called when message is received through TCP_Server recv function.
} TCP_Server;

typedef enum
{
    TCP_Server_Error_Socket,
    TCP_Server_Error_Bind,
    TCP_Server_Error_Listen,
    TCP_Server_Error_Send,
    TCP_Server_Success
} TCP_Server_Error;


/**
 * @brief This should be called first when wanting to setup TCP_Server
 *
 * TCP_Server_Setup creates a socket, binds and listens.
 * 
 *
 * @param out_server Sets socket and server_addr in TCP_Server struct.
 * @param port The port that will be binded to (0 to 65535).
 * @param callback The callback that is called when TCP_Server receives the full nullterminated data from a connection.
 *
 * @return On success returns TCP_Server_Success, otherwise returns a TCP_Server_Error enumerator.
 *
 */
TCP_Server_Error TCP_Server_Setup(TCP_Server *out_server, uint16_t port, TCP_Full_Data_Received callback);


/**
 * @brief This is the main work loop for the TCP_Server, which should be used after TCP_Server_Setup in a loop.
 *
 * @param server Takes in a TCP_Server struct
 *
 */
void TCP_Server_Work(TCP_Server *server);


TCP_Server_Error TCP_Server_Send(TCP_Server_Client* client, const void* data, size_t length);

#endif