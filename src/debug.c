
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define TIMEFMT "YYYY/MM/DD HH:MM:SS"

const char *level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "NONE",
};
const char *level_colours[] = {
    "",
    "\033[38;2;255;255;0;1m",
    "\033[38;2;255;127;0;1m",
    "\033[38;2;210;48;0;1m",
    "\033[38;2;160;0;0;1m",
    "\033[1m",
};

static enum loglvl_t minloglevel = INFO;

void set_min_loglevel(enum loglvl_t level) {
  minloglevel = level;
  logger(INFO, NO_ERROR, 0, "Log level set to %s%s\033[0m", level_colours[minloglevel],
         level_names[minloglevel]);
}

void logger(enum loglvl_t level, int id, int line, const char *format, ...) {
  if (level < minloglevel)
    return;

  FILE *out = stderr;

  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo = localtime(&rawtime);
  char time[sizeof(TIMEFMT)];
  strftime(time, sizeof(TIMEFMT), "%Y/%m/%d %X", timeinfo);
  fprintf(out, "%s - \033[38;2;220;220;220;1m0x%.02x\033[0m/%s%s\033[0m ", time,
          id, level_colours[level], level_names[level]);
  if (line)
    fprintf(out, "- %sL%d\033[0m ", level_colours[level], line);

  va_list format_params;
  va_start(format_params, format);
  vfprintf(out, format, format_params);
  va_end(format_params);

  putc('\n', out);
}
