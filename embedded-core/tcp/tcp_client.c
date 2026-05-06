#include "tcp_client.h"
#include "internet/internet.h"
#include "esp_log.h"

TCP_Client_Error TCP_Client_Initialize(TCP_Client *client)
{

    client->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket < 0)
    {
        return TCP_Client_Error_Socket;
    }

    memset(&client->server_addr, 0, sizeof(client->server_addr));

    client->has_initialized = false;

    return TCP_Client_Success;
}

TCP_Client_Error TCP_Client_Connect(TCP_Client *client, const char *ip, uint16_t port)
{
    // This is to avoid issues with trying to close a socket that is cooked
    if (client->socket < 0)
        return TCP_Client_Error_Socket;

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_addr.s_addr = inet_addr(ip);
    client->server_addr.sin_port = htons(port);
    
    if (connect(client->socket, (struct sockaddr*)&client->server_addr, sizeof(client->server_addr)) < 0 )
    {
        return TCP_Client_Error_Connect;
    }

    return TCP_Client_Success;
}

void TCP_Client_Dispose(TCP_Client *client)
{
    if (client->socket < 0)
        return;

    close(client->socket);
    client->socket = -1; // we invalidate it.
}

TCP_Client_Error TCP_Client_Send(TCP_Client* client, const void* data, size_t length)
{
    if (client->socket < 0)
    {
        return TCP_Client_Error_Socket;
    }

    if (Internet_Get_State() != NETWORK_ONLINE)
    {
        ESP_LOGW("TCP", "No internet connection");
        return TCP_Client_Error_Send;
    }

    if (send(client->socket, data, length, 0) < 0)
    {
        return TCP_Client_Error_Send;
    }

    return TCP_Client_Success;
}