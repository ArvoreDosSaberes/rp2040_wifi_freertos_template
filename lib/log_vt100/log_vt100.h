#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG = 1,
    LOG_LEVEL_INFO  = 2,
    LOG_LEVEL_WARN  = 3,
} log_level_t;

void log_set_level(log_level_t level);
void log_write(log_level_t level, const char *fmt, ...);

#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL LOG_LEVEL_INFO
#endif

#ifndef LOG_LEVEL
// -1: nenhum log; 0: apenas WARN; 1: INFO+WARN; 2: DEBUG+; 3: TRACE+
#define LOG_LEVEL 1
#endif

#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

#define LOG(level, fmt, ...) \
    log_write(LOG_LEVEL_##level, fmt, ##__VA_ARGS__)

// Mapeamento de verbosidade em tempo de compilação
#if LOG_LEVEL < 0

#define LOG_TRACE(fmt, ...) ((void)0)
#define LOG_DEBUG(fmt, ...) ((void)0)
#define LOG_INFO(fmt, ...)  ((void)0)
#define LOG_WARN(fmt, ...)  ((void)0)

#else

#if LOG_LEVEL >= 3
#define LOG_TRACE(fmt, ...) LOG(TRACE, fmt, ##__VA_ARGS__)
#else
#define LOG_TRACE(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= 2
#define LOG_DEBUG(fmt, ...) LOG(DEBUG, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= 1
#define LOG_INFO(fmt, ...)  LOG(INFO,  fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)  ((void)0)
#endif

// WARN fica sempre ativo quando LOG_LEVEL >= 0
#define LOG_WARN(fmt, ...)  LOG(WARN,  fmt, ##__VA_ARGS__)

#endif // LOG_LEVEL < 0

#ifdef __cplusplus
}
#endif

#endif // LOG_H

