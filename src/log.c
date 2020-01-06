#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "log.h"

static struct {
  void *udata;
  FILE *fp;
  int level;
  int quiet;
} Logger;

static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void log_init(const char* fname) {
    if (fname != NULL) {
        Logger.fp = fopen(fname, "a+");
        if (Logger.fp == NULL) {
            syslog(LOG_ERR, "Can not open log file: %s, error: %s",
                   fname, strerror(errno));
            Logger.fp = stdout;
        }
    } else {
        Logger.fp = stdout;
    }

    Logger.level = LOGGER_DEBUG;
}

void log_close() {
    if (Logger.fp != stdout) {
        fclose(Logger.fp);
    }
}
void log_set_level(int level) {
  Logger.level = level;
}

void log_log(int level, const char *file, int line, const char *fmt, ...) {
  if (level < Logger.level) {
    return;
  }

  /* Get current time */
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /* Log to file */
  if (Logger.fp) {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    fprintf(Logger.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
    va_start(args, fmt);
    vfprintf(Logger.fp, fmt, args);
    va_end(args);
    fprintf(Logger.fp, "\n");
    fflush(Logger.fp);
  }
}
