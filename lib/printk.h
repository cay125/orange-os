#ifndef LIB_PRINTF_H_

#include <stdarg.h>

int simple_vsprintf(char **out, const char *format, va_list ap, void (*f)(char));

#endif  // LIB_PRINTF_H_