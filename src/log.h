#ifndef LOG_H_
#define LOG_H_

#define COLOR_RED    "\033[0;31m"
#define COLOR_CYAN   "\033[0;36m"
#define COLOR_WHITE  "\033[38;5;252m"
#define COLOR_YELLOW "\033[38;5;220m"
#define COLOR_ORANGE "\033[38;5;208m"
#define COLOR_RESET  "\033[0m"

#define log_(lvl, typ, col, fmt, ...)                                                                                                                                                             \
do {                                                                                                                                                                                              \
    SDL_Time ns__ = { 0 };                                                                                                                                                                        \
    if (SDL_GetCurrentTime(&ns__))                                                                                                                                                                \
    {                                                                                                                                                                                             \
        SDL_DateTime dt__ = { 0 };                                                                                                                                                                \
        SDL_TimeToDateTime(ns__, &dt__, true);                                                                                                                                                    \
        const int ms__ = SDL_NS_TO_MS(dt__.nanosecond);                                                                                                                                           \
                                                                                                                                                                                                  \
        const char time__[] = {                                                                                                                                                                   \
            (char)('0' +  dt__.month / 10),                                                                                                                                                       \
            (char)('0' +  dt__.month % 10),                                                                                                                                                       \
            '-',                                                                                                                                                                                  \
            (char)('0' +  dt__.day / 10),                                                                                                                                                         \
            (char)('0' +  dt__.day % 10),                                                                                                                                                         \
            '-',                                                                                                                                                                                  \
            (char)('0' + dt__.year / 1000),                                                                                                                                                       \
            (char)('0' + dt__.year % 1000 / 100),                                                                                                                                                 \
            (char)('0' + dt__.year %  100 / 10),                                                                                                                                                  \
            (char)('0' + dt__.year %   10),                                                                                                                                                       \
            ' ',                                                                                                                                                                                  \
            (char)('0' +  dt__.hour / 10),                                                                                                                                                        \
            (char)('0' +  dt__.hour % 10),                                                                                                                                                        \
            ':',                                                                                                                                                                                  \
            (char)('0' +  dt__.minute / 10),                                                                                                                                                      \
            (char)('0' +  dt__.minute % 10),                                                                                                                                                      \
            ':',                                                                                                                                                                                  \
            (char)('0' +  dt__.second / 10),                                                                                                                                                      \
            (char)('0' +  dt__.second % 10),                                                                                                                                                      \
            '.',                                                                                                                                                                                  \
            (char)('0' + ms__ / 100),                                                                                                                                                             \
            (char)('0' + ms__ % 100 / 10),                                                                                                                                                        \
            (char)('0' + ms__ % 10),                                                                                                                                                              \
            '\0',                                                                                                                                                                                 \
        };                                                                                                                                                                                        \
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, lvl, col "[%s] " typ " [%" PRIu64 "] %s:%d: " fmt COLOR_RESET, time__, SDL_GetCurrentThreadID(), __FILE__, __LINE__, ##__VA_ARGS__);              \
    }                                                                                                                                                                                             \
    else                                                                                                                                                                                          \
    {                                                                                                                                                                                             \
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, lvl, col "[-----INVALID--TIME-----] " typ " [%" PRIu64 "] %s:%d: " fmt COLOR_RESET, SDL_GetCurrentThreadID(), __FILE__, __LINE__, ##__VA_ARGS__); \
    }                                                                                                                                                                                             \
} while (0)

#define log_dbg(fmt, ...) log_(SDL_LOG_PRIORITY_DEBUG,    " DEBUG ", COLOR_CYAN,   fmt, ##__VA_ARGS__)
#define log_msg(fmt, ...) log_(SDL_LOG_PRIORITY_INFO,     "MESSAGE", COLOR_WHITE,  fmt, ##__VA_ARGS__)
#define log_wrn(fmt, ...) log_(SDL_LOG_PRIORITY_WARN,     "WARNING", COLOR_YELLOW, fmt, ##__VA_ARGS__)
#define log_err(fmt, ...) log_(SDL_LOG_PRIORITY_ERROR,    " ERROR ", COLOR_ORANGE, fmt, ##__VA_ARGS__)
#define log_fat(fmt, ...) log_(SDL_LOG_PRIORITY_CRITICAL, " FATAL ", COLOR_RED,    fmt, ##__VA_ARGS__)

#endif // LOG_H_
