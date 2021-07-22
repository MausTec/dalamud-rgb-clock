#include <string.h>
#include <time.h>
#include "console.h"
#include "time_manager.h"

// typedef CommandParser<16, 4, 20, 64, 128> CMD;
// static CMD Parser;

static const int MAX_RESPONSE_SIZE = 128;

static void cmd_gettm(const char** argv, size_t argc, char* response) {
    struct tm time;
    if (!time_manager_get_local_time(&time)) {
        strncpy(response, "--:--:--", MAX_RESPONSE_SIZE);
        return;
    }

    snprintf(response, MAX_RESPONSE_SIZE, "%d:%d:%d", time.tm_hour, time.tm_min, time.tm_sec);
}

static void cmd_settm(const char** argv, size_t argc, char* response) {

}

esp_err_t console_init(void) {
    return ESP_OK;
}

static char line_buffer[128] = "";

esp_err_t console_tick(void) {
    size_t offset = strlen(line_buffer);

    while (false /*Serial.available()*/) {
        char c = 0;//Serial.read();
        if (c == '\r') {
            continue;
        } else if (c == '\n') {
            char response[MAX_RESPONSE_SIZE];
            //Serial.println(response);
            line_buffer[0] = '\0';
            offset = 0;
        } else {
            line_buffer[offset] = c;
            offset++;
            line_buffer[offset] = '\0';
        }
    }

    return ESP_OK;
}


