#ifndef __DEBUG_LOG_H
#define __DEBUG_LOG_H

#include <stdio.h>

#ifndef APP_LOG_ENABLED
#define APP_LOG_ENABLED 1
#endif

#if APP_LOG_ENABLED
#define APP_LOG(fmt, ...) printf("[APP] " fmt "\r\n", ##__VA_ARGS__)
#else
#define APP_LOG(fmt, ...) ((void)0)
#endif

#endif /* __DEBUG_LOG_H */
