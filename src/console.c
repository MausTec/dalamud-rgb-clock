#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "console.h"
#include "time_manager.h"

#define MAX_RESPONSE_SIZE 128
#define MAX_ARG_COUNT 9

void _get_int(int *out, char **argv, size_t argc, size_t *idx) {
    if (*idx >= argc) {
        return;
    }

    *out = atoi(argv[*idx]);
    (*idx)++;
}

static void cmd_gettm(char** argv, size_t argc, char* response) {
    struct tm time;
    if (ESP_OK != time_manager_get_local_time(&time)) {
        strncpy(response, "--:--:--", MAX_RESPONSE_SIZE);
        return;
    }

    snprintf(response, MAX_RESPONSE_SIZE, "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
}

static void cmd_settm(char** argv, size_t argc, char* response) {
    struct tm time = {};
    size_t idx = 0;
    
    if (argc == 3) {
        _get_int(&time.tm_hour, argv, argc, &idx);
        _get_int(&time.tm_min, argv, argc, &idx);
        _get_int(&time.tm_sec, argv, argc, &idx);
    } else if (argc == 1) {
        sscanf(argv[0], "%d:%d:%d", &time.tm_hour, &time.tm_min, &time.tm_sec);
    } else {
        return;
    }
    
    mktime(&time);

    snprintf(response, MAX_RESPONSE_SIZE, "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
}

typedef struct {
    const char *cmd;
    void (*func)(char**, size_t, char*);
} cmd_list_t;

static cmd_list_t cmd_list[] = {
    { "gettm", &cmd_gettm },
    { "settm", &cmd_settm },
};

esp_err_t console_init(void) {
    return ESP_OK;
}

void console_parse(char *buf, char *response) {
    size_t argc = 0;
    char *argv[MAX_ARG_COUNT];
    char *saveptr = NULL;
    char *token = strtok_r(buf, " \t", &saveptr);

    for (int i = 0; i < MAX_ARG_COUNT; i++)
        argv[i] = NULL;

    while (token != NULL && argc <= MAX_ARG_COUNT) {
        argv[argc] = token;
        argc++;
        token = strtok_r(NULL, " \t", &saveptr);
    }

    if (argc <= 0) {
        return;
    }

    char *cmd = argv[0];
    for (char *p = cmd; *p; ++p) *p = tolower(*p);

    for(size_t i = 0; i < sizeof(cmd_list) / sizeof(cmd_list[0]); i++) {
        cmd_list_t command = cmd_list[i];
        if (!strcmp(cmd, command.cmd)) {
            return command.func(argv + 1, argc - 1, response);
        }
    }

    snprintf(response, MAX_RESPONSE_SIZE, "Unknown Command: %s", cmd);
    return;
}

static char line_buffer[128] = "";

esp_err_t console_tick(void) {
    size_t offset = strlen(line_buffer);
    char c = getchar();

    while (c != 0xFF) {
        if (c == '\r') {
            continue;
        } else if (c == '\n') {
            char response[MAX_RESPONSE_SIZE + 1];
            response[0] = '\0';
            
            console_parse(line_buffer, response);
            printf("%s\n", response);
            line_buffer[0] = '\0';
            offset = 0;
        } else if (c == '\b') {
            if (offset > 0) offset--;
            line_buffer[offset] = '\0';
        } else {
            line_buffer[offset] = c;
            offset++;
            line_buffer[offset] = '\0';
        }

        c = getchar();
        break;
    }

    return ESP_OK;
}


