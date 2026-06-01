#include "ld2451_parser.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

static const char *TAG = "LD2451_PARSER";

// Define the frame header and footer
static const uint8_t FRAME_HEADER[] = {0xF4, 0xF3, 0xF2, 0xF1};
static const uint8_t FRAME_FOOTER[] = {0xF8, 0xF7, 0xF6, 0xF5};

ld2451_parse_status_t parse_ld2451_data(const uint8_t *data, uint16_t len, ld2451_data_t *result) {
    // 1. Check for minimum frame length
    if (len < 10) {
        return LD2451_PARSE_ERROR_INVALID_LENGTH;
    }

    // 2. Check if the frame header matches
    if (memcmp(data, FRAME_HEADER, sizeof(FRAME_HEADER)) != 0) {
        return LD2451_PARSE_ERROR_INVALID_HEADER;
    }

    // 3. Check if the frame footer matches
    if (memcmp(data + len - sizeof(FRAME_FOOTER), FRAME_FOOTER, sizeof(FRAME_FOOTER)) != 0) {
        return LD2451_PARSE_ERROR_INVALID_FOOTER;
    }
    
    // 4. Get the payload data length (little-endian)
    uint16_t data_length = data[4] | (data[5] << 8);

    // If there is no payload, return success
    if (data_length == 0) {
        result->target_count = 0;
        result->alert = false;
        return LD2451_PARSE_OK;
    }

    // 5. Validate the total frame length
    // Expected length = Header(4) + Length Field(2) + Data(data_length) + Footer(4)
    if (len != (sizeof(FRAME_HEADER) + 2 + data_length + sizeof(FRAME_FOOTER))) {
        return LD2451_PARSE_ERROR_DATA_MISMATCH;
    }

    // Pointer to the payload section of the data frame
    const uint8_t *payload = data + 6;

    // 6. Extract the number of targets and the alert information
    result->target_count = payload[0];
    result->alert = (payload[1] == 0x01);
    
    // Validate if the payload length matches the number of targets
    if (data_length != (2 + result->target_count * 5)) {
        return LD2451_PARSE_ERROR_DATA_MISMATCH;
    }
    
    // Limit the number of targets to parse to prevent overflowing our array
    uint8_t targets_to_parse = result->target_count;
    if (targets_to_parse > LD2451_MAX_TARGETS) {
        ESP_LOGW(TAG, "Detected %d targets, but will only parse %d", targets_to_parse, LD2451_MAX_TARGETS);
        targets_to_parse = LD2451_MAX_TARGETS;
    }

    // 7. Loop through and parse each target's information
    for (int i = 0; i < targets_to_parse; i++) {
        // Each target's information is 5 bytes long
        const uint8_t *target_data = payload + 2 + (i * 5);
        
        uint8_t angle_raw = target_data[0];
        
        // Actual angle = reported value - 0x80
        result->targets[i].angle = (int8_t)(angle_raw - 0x80);
        result->targets[i].distance = target_data[1];
        result->targets[i].is_approaching = (target_data[2] == 0x01);
        result->targets[i].speed = target_data[3];
        result->targets[i].snr = target_data[4];
    }
    
    return LD2451_PARSE_OK;
}

void print_ld2451_data(const ld2451_data_t *data) {
    ESP_LOGI(TAG, "--- Radar Detection Result ---");
    ESP_LOGI(TAG, "Alert Status: %s", data->alert ? "Target approaching" : "No alert");
    ESP_LOGI(TAG, "Targets Detected: %d", data->target_count);
    
    uint8_t targets_to_print = data->target_count;
    if (targets_to_print > LD2451_MAX_TARGETS) {
        targets_to_print = LD2451_MAX_TARGETS;
    }

    for (int i = 0; i < targets_to_print; i++) {
        const ld2451_target_t *t = &data->targets[i];
        ESP_LOGI(TAG, "  Target %d: Angle=%d°, Distance=%dm, Speed=%dkm/h (%s), SNR=%d",
                 i + 1,
                 t->angle,
                 t->distance,
                 t->speed,
                 t->is_approaching ? "approaching" : "moving away",
                 t->snr);
    }
}

void ld2451_create_test_frame_left(ld2451_data_t *result)
{
    if (!result) return;
    result->alert = true;
    result->target_count = 1;
    result->targets[0].angle = -45; // Left lane
    result->targets[0].distance = 10;
    result->targets[0].is_approaching = true;
    result->targets[0].speed = 50;
    result->targets[0].snr = 80;
}

void ld2451_create_test_frame_right(ld2451_data_t *result)
{
    if (!result) return;
    result->alert = true;
    result->target_count = 1;
    result->targets[0].angle = 45; // Right lane
    result->targets[0].distance = 10;
    result->targets[0].is_approaching = true;
    result->targets[0].speed = 50;
    result->targets[0].snr = 80;
}
