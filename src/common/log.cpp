#include "log.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

FILE *Log::log_file = stdout;
unsigned int Log::log_line = 0;

int Log::to(const char *file) {
    const char *file_name_start = NULL;
    (file_name_start = strrchr(file, '/')) ? ++file_name_start : (file_name_start = file);
    if (strlen(file_name_start) > MAX_PATH_LEN - 5) {
        Log::e("log: file name is too long");
        return 1;
    }
    const char *file_name_end = file + strlen(file) - 5;
    size_t name_len = (strcmp(file_name_end, ".conf") == 0 &&
                       file_name_end >= file_name_start) ?
                       (file_name_end - file_name_start) : strlen(file_name_start);
    char log_path[strlen(LOG_DIR) + MAX_PATH_LEN];
    memset(log_path, 0, sizeof(log_path));
    strncpy(log_path, LOG_DIR, strlen(LOG_DIR));
    strncpy(log_path + strlen(log_path), file_name_start, name_len);
    strcat(log_path, ".log");
    FILE *fp = fopen(log_path, "w");
    if (fp == NULL) {
        Log::e("io: fail to open %s for logging", log_path);
        return 1;
    }
    Log::i("log: opening %s for logging", log_path);
    log_file = fp;
    return 0;
}

void Log::close() {
    fclose(log_file);
}

void Log::e(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    write("ERROR", format, ap);
    va_end(ap);
}

void Log::i(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    write("INFO", format, ap);
    va_end(ap);
}

void Log::w(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    write("WARNING", format, ap);
    va_end(ap);
}

void Log::write(const char *label, const char *format, va_list args) {
    Message message;
    memset(&message, 0, sizeof(Message));
    time_t now;
    time(&now);
    strftime(message.time, sizeof(message.time), "%Y-%m-%d %H:%M:%S", localtime(&now));
    strncpy(message.label, label, sizeof(message.label) - 1);
    vsnprintf(message.text, sizeof(message.text), format, args);
    if (log_file == stdout)
        fprintf(log_file, "%s\n", message.text);
    else {
        if (log_line >= MAX_LOG_LINE) {
            ftruncate(fileno(log_file), 0);
            lseek(fileno(log_file), 0, SEEK_SET);
            fflush(log_file);
            log_line = 0;
        }
        fprintf(log_file, "%s [%s] %s\n", message.time, message.label, message.text);
        if (++log_line % 10 == 0)
            fflush(log_file);
    }
}
