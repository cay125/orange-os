#include "lib/lib.h"

#include "lib/printk.h"

void printf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	auto f = [](char c){
	  syscall::write(1, &c, 1);
	};
	simple_vsprintf(nullptr, fmt, va, f);
	va_end(va);
}