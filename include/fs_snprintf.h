#ifndef FREESTANDING_SNPRINTF_H
#define FREESTANDING_SNPRINTF_H


#include <stdarg.h>
#include "fs_int.h"

int fs_snprintf(char *buf, fs_size bufsz, const char *fmt, ...);
int fs_vsnprintf(char *buf, fs_size bufsz, const char *fmt, va_list ap);


#endif /* FREESTANDING_SNPRINTF_H */

