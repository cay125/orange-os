#include "lib/printk.h"

#define PRINT_BUF_LEN 64

static void simple_outputchar(char **str, char c, void (*f)(char))
{
	if (str) {
		**str = c;
		++(*str);
	} else {
		f(c);
	}
}

enum flags {
	PAD_ZERO = 1,
	PAD_RIGHT = 2,
	PREFIX = 4
};

static int prints(char **out, const char *string, int width, int flags, void (*f)(char))
{
	int pc = 0, padchar = ' ';

	if (width > 0) {
		int len = 0;
		const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if ((flags & PAD_ZERO) && !(flags & PAD_RIGHT))
			padchar = '0';
	}
	if (!(flags & PAD_RIGHT)) {
		for (; width > 0; --width) {
			simple_outputchar(out, padchar, f);
			++pc;
		}
	}
	for (; *string; ++string) {
		simple_outputchar(out, *string, f);
		++pc;
	}
	for (; width > 0; --width) {
		simple_outputchar(out, padchar, f);
		++pc;
	}

	return pc;
}

// this function print number `i` in the base of `base` (base > 1)
// `sign` is the flag of print signed number or unsigned number
// `width` and `flags` mean the length of printed number at least `width`,
// if the length of number is less than `width`, choose PAD_ZERO or PAD_RIGHT
// `letbase` means uppercase('A') or lowercase('a') when using hex
// you may need to call `prints`
// you do not need to print prefix like "0x", "0"...
// Remember the most significant digit is printed first.
static int printk_write_num(char **out, long long i, int base, int sign,
			    int width, int flags, int letbase, void (*f)(char))
{
	char print_buf[PRINT_BUF_LEN];
	char *s;
	int t, neg = 0, pc = 0;
	unsigned long long u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(out, print_buf, width, flags, f);
	}

	if (sign && base == 10 && i < 0) {
		neg = 1;
		u = -i;
	}
	// TODO: fill your code here
	// store the digitals in the buffer `print_buf`:
	// 1. the last postion of this buffer must be '\0'
	// 2. the format is only decided by `base` and `letbase` here

	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';
	while(u){
		t = u % base;
		if(t < 10){ // base less than 10
			*--s = '0' + t;
		}else{ // e.g. hex 
			*--s = letbase + (t - 10);
		}
		u /= base;
	}
	// finish todo

	bool pad_zero = width && (flags & PAD_ZERO) && !(flags & PAD_RIGHT);
	if (neg) {
		if (pad_zero) {
			simple_outputchar(out, '-', f);
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}
	if (flags & PREFIX) {
		if (base == 8) {
			if (pad_zero) {
				simple_outputchar(out, '0', f);
				++pc;
			} else {
			  *--s = '0';
			}
		} else if (base == 16) {
			char sub_c = (letbase == 'a') ? 'x' : 'X';
			if (pad_zero) {
				simple_outputchar(out, '0', f);
				simple_outputchar(out, sub_c, f);
				pc += 2;
			} else {
			  *--s = sub_c;
			  *--s = '0';
			}
		}
	}

	return pc + prints(out, s, width, flags, f);
}

int simple_vsprintf(char **out, const char *format, va_list ap, void (*f)(char))
{
	int width, flags;
	int pc = 0;
	char scr[2];
	union {
		char c;
		char *s;
		int i;
		unsigned int u;
		long li;
		unsigned long lu;
		long long lli;
		unsigned long long llu;
		short hi;
		unsigned short hu;
		signed char hhi;
		unsigned char hhu;
		void *p;
	} u;

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = flags = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				flags |= PAD_RIGHT;
			}
			if (*format == '#') {
				++format;
				flags |= PREFIX;
			}
			while (*format == '0') {
				++format;
				flags |= PAD_ZERO;
			}
			if (*format == '*') {
				width = va_arg(ap, int);
				format++;
			} else {
				for (; *format >= '0' && *format <= '9';
				     ++format) {
					width *= 10;
					width += *format - '0';
				}
			}
			switch (*format) {
			case ('d'):
				u.i = va_arg(ap, int);
				pc +=
				    printk_write_num(out, u.i, 10, 1, width,
						     flags, 'a', f);
				break;

			case ('u'):
				u.u = va_arg(ap, unsigned int);
				pc +=
				    printk_write_num(out, u.u, 10, 0, width,
						     flags, 'a', f);
				break;

			case ('o'):
				u.u = va_arg(ap, unsigned int);
				pc +=
				    printk_write_num(out, u.u, 8, 0, width,
						     flags, 'a', f);
				break;

			case ('x'):
				u.u = va_arg(ap, unsigned int);
				pc +=
				    printk_write_num(out, u.u, 16, 0, width,
						     flags, 'a', f);
				break;

			case ('X'):
				u.u = va_arg(ap, unsigned int);
				pc +=
				    printk_write_num(out, u.u, 16, 0, width,
						     flags, 'A', f);
				break;

			case ('p'):
				u.lu = va_arg(ap, unsigned long);
				pc +=
				    printk_write_num(out, u.lu, 16, 0, width,
						     flags, 'a', f);
				break;

			case ('c'):
				u.c = va_arg(ap, int);
				scr[0] = u.c;
				scr[1] = '\0';
				pc += prints(out, scr, width, flags, f);
				break;

			case ('s'):
				u.s = va_arg(ap, char *);
				pc +=
				    prints(out, u.s ? u.s : "(null)", width,
					   flags, f);
				break;
			case ('l'):
				++format;
				switch (*format) {
				case ('d'):
					u.li = va_arg(ap, long);
					pc +=
					    printk_write_num(out, u.li, 10, 1,
							     width, flags, 'a', f);
					break;

				case ('u'):
					u.lu = va_arg(ap, unsigned long);
					pc +=
					    printk_write_num(out, u.lu, 10, 0,
							     width, flags, 'a', f);
					break;

				case ('o'):
					u.lu = va_arg(ap, unsigned long);
					pc +=
					    printk_write_num(out, u.lu, 8, 0,
							     width, flags, 'a', f);
					break;

				case ('x'):
					u.lu = va_arg(ap, unsigned long);
					pc +=
					    printk_write_num(out, u.lu, 16, 0,
							     width, flags, 'a', f);
					break;

				case ('X'):
					u.lu = va_arg(ap, unsigned long);
					pc +=
					    printk_write_num(out, u.lu, 16, 0,
							     width, flags, 'A', f);
					break;

				case ('l'):
					++format;
					switch (*format) {
					case ('d'):
						u.lli = va_arg(ap, long long);
						pc +=
						    printk_write_num(out, u.lli,
								     10, 1,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('u'):
						u.llu =
						    va_arg(ap,
							   unsigned long long);
						pc +=
						    printk_write_num(out, u.llu,
								     10, 0,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('o'):
						u.llu =
						    va_arg(ap,
							   unsigned long long);
						pc +=
						    printk_write_num(out, u.llu,
								     8, 0,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('x'):
						u.llu =
						    va_arg(ap,
							   unsigned long long);
						pc +=
						    printk_write_num(out, u.llu,
								     16, 0,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('X'):
						u.llu =
						    va_arg(ap,
							   unsigned long long);
						pc +=
						    printk_write_num(out, u.llu,
								     16, 0,
								     width,
								     flags,
								     'A',
										 f);
						break;

					default:
						break;
					}
					break;
				default:
					break;
				}
				break;
			case ('h'):
				++format;
				switch (*format) {
				case ('d'):
					u.hi = va_arg(ap, int);
					pc +=
					    printk_write_num(out, u.hi, 10, 1,
							     width, flags, 'a', f);
					break;

				case ('u'):
					u.hu = va_arg(ap, unsigned int);
					pc +=
					    printk_write_num(out, u.lli, 10, 0,
							     width, flags, 'a', f);
					break;

				case ('o'):
					u.hu = va_arg(ap, unsigned int);
					pc +=
					    printk_write_num(out, u.lli, 8, 0,
							     width, flags, 'a', f);
					break;

				case ('x'):
					u.hu = va_arg(ap, unsigned int);
					pc +=
					    printk_write_num(out, u.lli, 16, 0,
							     width, flags, 'a', f);
					break;

				case ('X'):
					u.hu = va_arg(ap, unsigned int);
					pc +=
					    printk_write_num(out, u.lli, 16, 0,
							     width, flags, 'A', f);
					break;

				case ('h'):
					++format;
					switch (*format) {
					case ('d'):
						u.hhi = va_arg(ap, int);
						pc +=
						    printk_write_num(out, u.hhi,
								     10, 1,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('u'):
						u.hhu =
						    va_arg(ap, unsigned int);
						pc +=
						    printk_write_num(out, u.lli,
								     10, 0,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('o'):
						u.hhu =
						    va_arg(ap, unsigned int);
						pc +=
						    printk_write_num(out, u.lli,
								     8, 0,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('x'):
						u.hhu =
						    va_arg(ap, unsigned int);
						pc +=
						    printk_write_num(out, u.lli,
								     16, 0,
								     width,
								     flags,
								     'a',
										 f);
						break;

					case ('X'):
						u.hhu =
						    va_arg(ap, unsigned int);
						pc +=
						    printk_write_num(out, u.lli,
								     16, 0,
								     width,
								     flags,
								     'A',
										 f);
						break;

					default:
						break;
					}
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		} else {
 out:
			simple_outputchar(out, *format, f);
			++pc;
		}
	}
	if (out)
		**out = '\0';
	return pc;
}
