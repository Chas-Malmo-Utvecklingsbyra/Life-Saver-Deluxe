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
    int flags = fcntl(server->socket, F_GETFL, 0);
    fcntl(server->socket, F_SETFL, flags | O_NONBLOCK);
    char buffer[1024];


    while (1)
    {
        int client = accept(server->socket, NULL, NULL);

        if (client < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //ESP_LOGI(TAG, "No new client!");
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }
            else
            {
                //ESP_LOGE(TAG, "Failed to accept client");
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }
        }

        ESP_LOGI(TAG, "Got a new client!");

        int bytes_left = 0;

        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(client, buffer, sizeof(buffer), 0);

        if (bytes > 0)
        {
            bytes_left += bytes;

            buffer[bytes_left] = '\0';
            ESP_LOGI(TAG, "Read buffer: [%s]\n", buffer);
        }

    }

}

TCP_Server_Error TCP_Server_Setup(TCP_Server *out_server, uint16_t port)
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

    TCP_Server_Accept(out_server);

    return TCP_Server_Success;
}
