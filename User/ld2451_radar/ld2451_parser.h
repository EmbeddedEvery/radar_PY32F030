#ifndef __LD2451_PARSER_H__
#define __LD2451_PARSER_H__

#include <stdint.h>
#include <stdbool.h>

#ifndef LD2451_MAX_TARGETS
#define LD2451_MAX_TARGETS 5
#endif

typedef enum {
    LD2451_PARSE_OK = 0,
    LD2451_PARSE_ERROR_INVALID_LENGTH,
    LD2451_PARSE_ERROR_INVALID_HEADER,
    LD2451_PARSE_ERROR_INVALID_FOOTER,
    LD2451_PARSE_ERROR_DATA_MISMATCH
} ld2451_parse_status_t;

typedef struct {
    int8_t angle;          // Angle in degrees (-127 to 127)
    uint8_t distance;      // Distance in meters
    bool is_approaching;   // true = approaching, false = moving away
    uint8_t speed;         // Speed in km/h
    uint8_t snr;           // Signal-to-Noise Ratio
} ld2451_target_t;

typedef struct {
    uint8_t target_count;                       // Number of targets detected
    bool alert;                                 // Alert status (true = active, false = no alert)
    ld2451_target_t targets[LD2451_MAX_TARGETS]; // Targets list
} ld2451_data_t;

/**
 * @brief Parse the raw data frame from the LD2451 radar.
 * 
 * @param data Pointer to the raw data buffer.
 * @param len Length of the data buffer.
 * @param result Pointer to the structure where the parsed result will be stored.
 * @return ld2451_parse_status_t Status of the parsing operation.
 */
ld2451_parse_status_t parse_ld2451_data(const uint8_t *data, uint16_t len, ld2451_data_t *result);

/**
 * @brief Print the parsed radar data to the console using log system.
 * 
 * @param data Pointer to the parsed radar data.
 */
void print_ld2451_data(const ld2451_data_t *data);

/**
 * @brief Create a mock test frame for a target approaching on the left lane.
 * 
 * @param result Pointer to the structure where the test frame will be created.
 */
void ld2451_create_test_frame_left(ld2451_data_t *result);

/**
 * @brief Create a mock test frame for a target approaching on the right lane.
 * 
 * @param result Pointer to the structure where the test frame will be created.
 */
void ld2451_create_test_frame_right(ld2451_data_t *result);

#endif // __LD2451_PARSER_H__
