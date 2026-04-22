#include "tcp_server.h"

#include <stdlib.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

static const char* TAG = "TCP_Server";

bool TCP_Server_Bind(TCP_Server *server)
{
    int bind_result = bind(server->socket, (struct sockaddr*)&server->server_addr, sizeof(server->server_addr));
    if (bind_result < 0)
    {
        return false;
    }
    return true;
}

bool TCP_Server_Listen(TCP_Server *server)
{
    int listen_result = listen(server->socket, TCP_SERVER_BACKLOG);
    if (listen_result < 0)
    {
        return false;
    }
    return true;
}

void TCP_Server_Accept(TCP_Server *server)
{
    int client = accept(server->socket, NULL, NULL);

    if (client < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // ESP_LOGI(TAG, "No new client!");
            return;
        }
        else
        {
            // ESP_LOGE(TAG, "Failed to accept client");
            return;
        }
    }

    ESP_LOGI(TAG, "Got a new client!");

    int total_bytes = 0;

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int flags = fcntl(client, F_GETFL, 0);
    fcntl(client, F_SETFL, flags | O_NONBLOCK);

    while (true)
    {
        ESP_LOGI(TAG, "recving");
        int bytes = recv(client, &buffer[total_bytes], sizeof(buffer), 0);

        if (bytes > 0)
        {
            ESP_LOGI(TAG, "bytes increased");
            total_bytes += bytes;
        }

        if (bytes == 0)
        {
            ESP_LOGI(TAG, "broke");
            break;
        }

        if (bytes < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                break;
            }
            
            ESP_LOGI(TAG, "recv failed in TCP_Server_Accept");
            return;
        }

        ESP_LOGI(TAG, "Looppoop");
    }

    ESP_LOGI(TAG, "Entering out of here");

    buffer[total_bytes] = '\0';

    TCP_Server_Client tcp_client = {
        .data = buffer,
        .len = total_bytes,
        .socket = client
    };

    ESP_LOGI(TAG, "Entering callback");
    server->callback(&tcp_client);

    //ESP_LOGI(TAG, "Read buffer: [%s]\n", buffer);
}

TCP_Server_Error TCP_Server_Setup(TCP_Server *out_server, uint16_t port, TCP_Full_Data_Received callback)
{
    out_server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (out_server->socket < 0)
    {
        return TCP_Server_Error_Socket;
    }

    out_server->server_addr.sin_family = AF_INET;
    out_server->server_addr.sin_port = htons(port);
    out_server->server_addr.sin_addr.s_addr = INADDR_ANY;

    if (!TCP_Server_Bind(out_server))
    {
        return TCP_Server_Error_Bind;
    }

    if (!TCP_Server_Listen(out_server))
    {
        return TCP_Server_Error_Listen;
    }

    out_server->callback = callback;

    int flags = fcntl(out_server->socket, F_GETFL, 0);
    fcntl(out_server->socket, F_SETFL, flags | O_NONBLOCK);

    return TCP_Server_Success;
}

TCP_Server_Error TCP_Server_Send(TCP_Server_Client* client, const void* data, size_t length)
{
    if (client->socket < 0)
    {
        return TCP_Server_Error_Socket;
    }

    if (send(client->socket, data, length, 0) < 0)
    {
        return TCP_Server_Error_Send; 
    }

    return TCP_Server_Success;
}


void TCP_Server_Work(TCP_Server *server)
{
    TCP_Server_Accept(server);
}