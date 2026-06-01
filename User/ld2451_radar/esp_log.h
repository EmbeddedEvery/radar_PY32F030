#ifndef __ESP_LOG_H__
#define __ESP_LOG_H__

#include "../debug_log.h"

// Map ESP-IDF logging macros to the local APP_LOG system
#define ESP_LOGI(tag, fmt, ...) APP_LOG("[%s] " fmt, tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) APP_LOG("[%s] WARNING: " fmt, tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, fmt, ...) APP_LOG("[%s] ERROR: " fmt, tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, fmt, ...) APP_LOG("[%s] DEBUG: " fmt, tag, ##__VA_ARGS__)
#define ESP_LOGV(tag, fmt, ...) APP_LOG("[%s] VERBOSE: " fmt, tag, ##__VA_ARGS__)

#endif // __ESP_LOG_H__
