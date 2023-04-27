#ifndef _CORE_ENGINES_LOGGER_H
#define _CORE_ENGINES_LOGGER_H

#include <ctime>

namespace core
{
    class Logger
    {
    public:
        Logger();
        ~Logger();

        void
        log_header(const char *s);

        void
        log(const char *format, ...);

    private:
        void   *file;
        int    error_count;
        time_t start_time;
        
        bool   logging_enabled;
    };
}

#endif
