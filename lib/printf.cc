#include "lib/lib.h"

#include "lib/printk.h"
#include "lib/string.h"

void printf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char out_str[64] = {0};
	char* p = out_str;
	simple_vsprintf(&p, fmt, va, nullptr);
	va_end(va);
	syscall::write(1, out_str, strlen(out_str));
}

void printf(PrintLevel level, const char *fmt, ...) {
  const char* escape_sequences_start[] = {
    [0] = "\033[33m",
    [1] = "\033[31m",
  };
  const char* escape_sequences_end = "\033[0m";
  printf("%s", escape_sequences_start[(int)level]);
  va_list va;
	va_start(va, fmt);
	char out_str[64] = {0};
	char* p = out_str;
	simple_vsprintf(&p, fmt, va, nullptr);
	va_end(va);
	syscall::write(1, out_str, strlen(out_str));
  printf("%s", escape_sequences_end);
}