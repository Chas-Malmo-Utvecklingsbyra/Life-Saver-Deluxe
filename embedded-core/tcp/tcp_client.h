#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdint.h>

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/inet.h>

#include <stdatomic.h>

typedef enum
{
    TCP_Client_Error_Socket,
    TCP_Client_Error_Connect,
    TCP_Client_Error_Send,
    TCP_Client_Success,
} TCP_Client_Error;

typedef struct
{
    int socket;
    struct sockaddr_in server_addr;
    atomic_bool has_initialized;
} TCP_Client;

/**
 * @brief Initializes the TCP_Client
 *
 * Initializes socket and memsets server_addr to 0.
 *  
 * @param client The TCP_Client to initialize
 *
 * @return Returns a TCP_Client_Success on a successful initalization otherwise returns a TCP_Client_Error enumerator
 *
 * @warning After a successful intiialization, the TCP_Client needs to close the socket after it is done. This is done via TCP_Client_Dispose(TCP_Client* client).
 *
 */
TCP_Client_Error TCP_Client_Initialize(TCP_Client *client);


/**
 * @brief Connects to a TCP_Server via IP and PORT
 *
 * @param client The TCP_Client to connect with
 * @param ip The IP Address to connect with
 * @param port The port (0-65535) to connect with
 *
 * @return Returns a TCP_Client_Success on a successful initalization otherwise returns a TCP_Client_Error enumerator
 * 
 *
 */
TCP_Client_Error TCP_Client_Connect(TCP_Client *client, const char *ip, uint16_t port);


/**
 * @brief Disposes TCP_Client
 *  
 * Closes client socket and sets it to -1
 * 
 * @param client The TCP_Client that is gonna get disposed
 *
 *
 */
void TCP_Client_Dispose(TCP_Client *client);

/**
 * 
 * @brief Sends a TCP message to a TCP_Client
 * 
 * TCP_Client_Send sends data to the client using sockets.
 *
 * @param data The data to send to the client.
 * @param length The length of the data which should be sent to the client.
 * 
 * @return TCP_Client_Error depending on the result of the action. TCP_Client_Success for success!
 *
 * @note Return values may be TCP_Client_Success, TCP_Client_Error_Send or TCP_Client_Error_Socket
 * 
 */
TCP_Client_Error TCP_Client_Send(TCP_Client* client, const void* data, size_t length);

#endif