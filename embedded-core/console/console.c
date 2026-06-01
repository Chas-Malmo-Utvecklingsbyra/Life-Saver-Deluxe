#include "console.h"

#include <string.h>

#include "esp_console.h"
#include "esp_log.h"

static const char* TAG = "Console";
static Console console = {0};

bool Console_Command_Add(Console_Command command)
{
    if (console.commands == NULL)
    {
        console.commands = (Console_Command*)malloc(sizeof(Console_Command));
        if (console.commands == NULL)
        {
            ESP_LOGE(TAG, "Failed to malloc Console_Command_Add");
            return false;
        }

        console.commands[0] = command;
        console.commands_length = 1;
        console.commands_capacity = 1;
    }
    else if (console.commands_length < console.commands_capacity)
    {
        console.commands[console.commands_length++] = command;
    }
    else
    {
        console.commands_capacity *= 2;

        console.commands = (Console_Command*)realloc(console.commands, sizeof(Console_Command) * console.commands_capacity);
        if (console.commands == NULL)
        {
            ESP_LOGE(TAG, "Failed to realloc Console_Command_Add");
            return false;
        }

        console.commands_length += 1;
        console.commands[console.commands_length - 1] = command;
    }

    return true;
}

void Console_Initialize()
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "cmd> ";

    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    if (esp_console_new_repl_uart(&uart_config, &repl_config, &repl) != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to establish UART!");
        return;
    }

    for (size_t i = 0; i < console.commands_length; i++)
    {
        const esp_console_cmd_t command = {
            .command = console.commands[i].command,
            .help = console.commands[i].help,
            .hint = console.commands[i].hint,
            .func = console.commands[i].callback,
        };

        ESP_LOGE(TAG, "Registered command hehe!");
        ESP_ERROR_CHECK(esp_console_cmd_register(&command));
    }


    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}

void Console_Dispose()
{
    if (console.commands != NULL)
    {
        free(console.commands);
        console.commands = NULL;
    }
}