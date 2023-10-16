#ifndef FREESTANDING_SNPRINTF_H
#define FREESTANDING_SNPRINTF_H


#ifndef size_t 
#  define size_t unsigned long
#endif /* size_t */
#ifndef va_list
#  include <stdarg.h>
#endif /* va_list */

int fs_snprintf(char *buf, size_t bufsz, const char *fmt, ...);
int fs_vsnprintf(char *buf, size_t bufsz, const char *fmt, va_list ap);


#endif /* FREESTANDING_SNPRINTF_H */

