#include "kernel/printf.h"

#include "driver/uart.h"
#include "lib/printk.h"

namespace kernel {

void printf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	auto f = [](char c){
		driver::Uart::put_char(c);
	};
	simple_vsprintf(nullptr, fmt, va, f);
	va_end(va);
}

}  // namespace kernel