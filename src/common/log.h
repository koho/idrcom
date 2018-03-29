#ifndef LOG_H
#define LOG_H

#define LOG_DIR "/tmp/"
#define MAX_TEXT_LEN 200
#define MAX_PATH_LEN 150
#define MAX_LOG_LINE 200

#include <stdio.h>
#include <stdarg.h>

typedef struct {
    char time[22];
    char label[10];
    char text[MAX_TEXT_LEN];
} Message;

class Log {
    
public:
    static FILE *log_file;
    static unsigned int log_line;
    static int to(const char *file);
    static void close();
    static void e(const char *format, ...);
    static void i(const char *format, ...);
    static void w(const char *format, ...);
    static void write(const char *label, const char *format, va_list args);
};
#endif
