#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kernel/tty.h>

enum FormatType {
    Char,
    FloatExp,
    FloatFixed,
    FloatOpt,
    HexFloat,
    Int16,
    Int10,
    Int8,
    Int2,
    UInt10,
    String,
    Percent,
    NChars,
    FormatTypeMax
};

enum Length {
    HalfHalf,
    Half,
    Long,
    LongLong,
    LongDouble,
    SizeT,
    IntMaxT,
    PtrDiffT,
    LengthMax
};

int printf(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    size_t fmt_len = strlen(format);
    int num_chars_written = 0;
    for (size_t index = 0; index < fmt_len; index++) {
        if (format[index] == '\\' && format[index + 1] == '%') {
            index++;
            putchar((int) format[index]);
            num_chars_written++;
        } else if (format[index] != '%') {
            putchar((int) format[index]);
            num_chars_written++;
        } else {
            index++;
            // @TODO: Set some of these up as flags?
            enum FormatType format_type = FormatTypeMax;
            enum Length format_length = LengthMax;
            size_t format_width = 0;
            size_t format_precision = 0;
            bool num_prefix = false;
            bool left_aligned = false;
            bool zero_fill = false;
            bool prepend_pos = false;
            bool prepend_space = false;
            bool capitals = false;
            bool thousands_separator = false;
            bool done = false;
            // Flags field
            while (done == false) {
                switch (format[index]) {
                case '-':
                    index++;
                    left_aligned = true;
                    break;
                case '+':
                    index++;
                    prepend_pos = true;
                    break;
                case ' ':
                    index++;
                    prepend_space = true;
                    break;
                case '0':
                    index++;
                    zero_fill = true;
                    break;
                case '\'':
                    index++;
                    thousands_separator = true;
                    break;
                case '#':
                    index++;
                    num_prefix = true;
                    break;
                default:
                    done = true;
                    break;
                }
            }
            done = false;
            if (format[index] == '*') {
                format_width = va_arg(args, unsigned int);
                index++;
            } else {
                while (done == false) {
                    if ('0' <= format[index] && format[index] <= '9') {
                        format_width *= 10;
                        format_width += format[index] - '0';
                        index++;
                    } else {
                        done = true;
                        break;
                    }
                }
            }
            done = false;
            if (format[index] == '.') {
                index++;
                while (done == false) {
                    if ('0' <= format[index] && format[index] <= '9') {
                        format_precision *= 10;
                        format_precision += format[index] - '0';
                        index++;
                    } else {
                        done = true;
                        break;
                    }
                }
            }
            switch (format[index]) {
            case 'h':
                index++;
                if (format[index] == 'h') {
                    format_length = HalfHalf;
                    index++;
                } else
                    format_length = Half;
                break;
            case 'l':
                index++;
                if (format[index] == 'l') {
                    format_length = LongLong;
                    index++;
                } else
                    format_length = Long;
                break;
            case 'L':
                format_length = LongDouble;
                index++;
                break;
            case 'z':
                format_length = SizeT;
                index++;
                break;
            case 'j':
                format_length = IntMaxT;
                index++;
                break;
            case 't':
                format_length = PtrDiffT;
                index++;
                break;
            }
            switch (format[index]) {
            case '%':
                format_type = Percent;
                break;
            case 'd':
            case 'i':
                format_type = Int10;
                break;
            case 'u':
                format_type = UInt10;
                break;
            case 'f':
                format_type = FloatFixed;
                break;
            case 'F':
                format_type = FloatFixed;
                capitals = true;
                break;
            case 'e':
                format_type = FloatExp;
                break;
            case 'E':
                format_type = FloatExp;
                capitals = true;
                break;
            case 'g':
                format_type = FloatOpt;
                break;
            case 'G':
                format_type = FloatOpt;
                capitals = true;
                break;
            case 'x':
                format_type = Int16;
                break;
            case 'X':
                format_type = Int16;
                capitals = true;
                break;
            case 'o':
                format_type = Int8;
                break;
            case 'b':
                format_type = Int2;
                break;
            case 's':
                format_type = String;
                break;
            case 'c':
                format_type = Char;
                break;
            case 'a':
                format_type = HexFloat;
                break;
            case 'A':
                format_type = HexFloat;
                capitals = true;
                break;
            case 'n':
                format_type = NChars;
                break;
            default:
                break;  // Error: Unrecognised format specifier.
            }
            switch (format_type) {
            case Char: {
                int c = va_arg(args, int);
                putchar(c);
                num_chars_written++;
            } break;
            case FloatExp:
            case FloatFixed:
            case FloatOpt:
            case HexFloat:
                break;
            case Int10: {
                unsigned long long num;
                bool negative = false;
                switch (format_length) {
                case HalfHalf: {
                    int num_in = va_arg(args, int);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                case Half: {
                    int num_in = va_arg(args, int);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                case Long: {
                    long num_in = va_arg(args, long);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                case LongLong: {
                    long long num_in = va_arg(args, long long);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                case SizeT: {
                    long long num_in = va_arg(args, size_t);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                case IntMaxT: {
                    long long num_in = va_arg(args, intmax_t);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                case PtrDiffT: {
                    long long num_in = va_arg(args, ptrdiff_t);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                default: {
                    long long num_in = va_arg(args, int);
                    if (num_in < 0) {
                        num = (unsigned long long) (-num_in);
                        negative = true;
                    } else {
                        num = (unsigned long long) num_in;
                    }
                } break;
                }
                if (negative) {
                    putchar('-');
                    num_chars_written++;
                    if (format_width > 0)
                        format_width -= 1;
                } else if (prepend_pos) {
                    putchar('+');
                    num_chars_written++;
                    if (format_width > 0)
                        format_width -= 1;
                } else if (prepend_space) {
                    putchar(' ');
                    num_chars_written++;
                    if (format_width > 0)
                        format_width -= 1;
                }
                size_t exp = 0;
                size_t val = 1;
                while (true) {
                    exp++;
                    val *= 10;
                    if (val > num) {
                        break;
                    }
                }
                bool first = true;
                char zfill_char = zero_fill ? '0' : ' ';
                if (!left_aligned)
                    while ((signed long long) (format_width - exp) > 0) {
                        if (!first && thousands_separator && exp == (exp / 3) * 3 &&
                            exp != 0) {
                            putchar(',');
                            num_chars_written++;
                        }
                        putchar(zfill_char);
                        num_chars_written++;
                        exp++;
                    }
                while (val > 1) {
                    val /= 10;
                    exp--;
                    putchar('0' + num / val);
                    num_chars_written++;
                    num %= val;
                    if (thousands_separator && exp == (exp / 3) * 3 && exp != 0) {
                        putchar(',');
                        num_chars_written++;
                    }
                }
                if (left_aligned)
                    while (exp > 0) {
                        putchar(' ');
                        num_chars_written++;
                    }
            } break;
            case UInt10: {
                unsigned long long num;
                switch (format_length) {
                case HalfHalf: {
                    num = va_arg(args, unsigned int);
                } break;
                case Half: {
                    num = va_arg(args, unsigned int);
                } break;
                case Long: {
                    num = va_arg(args, unsigned long);
                } break;
                case LongLong: {
                    num = va_arg(args, unsigned long long);
                } break;
                case SizeT: {
                    num = va_arg(args, size_t);
                } break;
                case IntMaxT: {
                    num = va_arg(args, intmax_t);
                } break;
                case PtrDiffT: {
                    num = va_arg(args, ptrdiff_t);
                } break;
                default: {
                    num = va_arg(args, unsigned int);
                } break;
                }
                size_t exp = 0;
                size_t val = 1;
                while (true) {
                    exp++;
                    val *= 10;
                    if (val > num) {
                        break;
                    }
                }
                bool first = true;
                char zfill_char = zero_fill ? '0' : ' ';
                if (!left_aligned)
                    while ((signed long long) (format_width - exp) > 0) {
                        if (!first && thousands_separator && exp == (exp / 3) * 3 &&
                            exp != 0) {
                            putchar(',');
                            num_chars_written++;
                        }
                        putchar(zfill_char);
                        num_chars_written++;
                        exp++;
                    }
                while (val > 1) {
                    val /= 10;
                    exp--;
                    putchar('0' + num / val);
                    num_chars_written++;
                    num %= val;
                    if (thousands_separator && exp == (exp / 3) * 3 && exp != 0) {
                        putchar(',');
                        num_chars_written++;
                    }
                }
                if (left_aligned)
                    while (exp > 0) {
                        putchar(' ');
                        num_chars_written++;
                    }
            } break;
            case Int16:
            case Int8:
            case Int2: {
                unsigned long long num;
                switch (format_length) {
                case HalfHalf: {
                    num = va_arg(args, int);
                } break;
                case Half: {
                    num = va_arg(args, int);
                } break;
                case Long: {
                    num = va_arg(args, long);
                } break;
                case LongLong: {
                    num = va_arg(args, long long);
                } break;
                case SizeT: {
                    num = va_arg(args, size_t);
                } break;
                case IntMaxT: {
                    num = va_arg(args, intmax_t);
                } break;
                case PtrDiffT: {
                    num = va_arg(args, ptrdiff_t);
                } break;
                default: {
                    num = va_arg(args, int);
                } break;
                }
                if (format_type == Int16) {
                    if (num_prefix) {
                        putchar('0');
                        num_chars_written++;
                        putchar(capitals ? 'X' : 'x');
                        num_chars_written++;
                        if (format_width == 1) {
                            format_width = 0;
                        } else if (format_width > 1) {
                            format_width -= 2;
                        }
                    }
                    const char* chars_upper = "0123456789ABCDEF";
                    const char* chars_lower = "0123456789abcdef";
                    size_t exp = 0;
                    size_t val = 1;
                    while (true) {
                        exp++;
                        val <<= 4;
                        if (val > num) {
                            break;
                        }
                    }
                    char zfill_char = zero_fill ? '0' : ' ';
                    if (!left_aligned)
                        while ((signed long long) (format_width - exp) > 0) {
                            putchar(zfill_char);
                            num_chars_written++;
                            exp++;
                        }
                    while (val > 1) {
                        val >>= 4;
                        putchar(
                            capitals ? chars_upper[num / val] : chars_lower[num / val]);
                        num_chars_written++;
                        num %= val;
                    }
                    if (left_aligned)
                        while (exp > 0) {
                            putchar(' ');
                            num_chars_written++;
                            exp++;
                        }
                } else if (format_type == Int8) {
                    if (num_prefix) {
                        putchar('0');
                        num_chars_written++;
                        if (format_width > 0) {
                            format_width -= 1;
                        }
                    }
                    const char* chars_upper = "01234567";
                    const char* chars_lower = "01234567";
                    size_t exp = 0;
                    size_t val = 1;
                    while (true) {
                        exp++;
                        val <<= 3;
                        if (val > num) {
                            break;
                        }
                    }
                    char zfill_char = zero_fill ? '0' : ' ';
                    if (!left_aligned)
                        while ((signed long long) (format_width - exp) > 0) {
                            putchar(zfill_char);
                            num_chars_written++;
                            exp++;
                        }
                    if (num == 0) {
                        putchar('0');
                        num_chars_written++;
                    } else
                        while (val > 1) {
                            val >>= 3;
                            putchar(
                                capitals ? chars_upper[num / val]
                                         : chars_lower[num / val]);
                            num_chars_written++;
                            num %= val;
                        }
                    if (left_aligned)
                        while (exp > 0) {
                            putchar(' ');
                            num_chars_written++;
                        }
                } else if (format_type == Int2) {
                    if (num_prefix) {
                        putchar('0');
                        num_chars_written++;
                        putchar('b');
                        num_chars_written++;
                        if (format_width > 1) {
                            format_width -= 1;
                        } else {
                            format_width = 0;
                        }
                    }
                    const char* chars_upper = "01";
                    const char* chars_lower = "01";
                    size_t exp = 0;
                    size_t val = 1;
                    while (true) {
                        exp++;
                        val <<= 1;
                        if (val > num) {
                            break;
                        }
                    }
                    char zfill_char = zero_fill ? '0' : ' ';
                    if (!left_aligned)
                        while ((signed long long) (format_width - exp) > 0) {
                            putchar(zfill_char);
                            num_chars_written++;
                            exp++;
                        }
                    if (num == 0) {
                        putchar('0');
                        num_chars_written++;
                    } else
                        while (val > 1) {
                            val >>= 1;
                            putchar(
                                capitals ? chars_upper[num / val]
                                         : chars_lower[num / val]);
                            num_chars_written++;
                            num %= val;
                        }
                    if (left_aligned)
                        while (exp > 0) {
                            putchar(' ');
                            num_chars_written++;
                        }
                }
            } break;
            case String: {
                const char* str = va_arg(args, char*);
                size_t len = strlen(str);
                for (size_t i = 0; i < len; i++) {
                    putchar(str[i]);
                    num_chars_written++;
                }
            } break;
            case Percent: {
                putchar('%');
                num_chars_written++;
            } break;
            case NChars: {
                int* n = va_arg(args, int*);
                *n = num_chars_written;
            } break;
            case FormatTypeMax:
                break;
            }
        }
    }
    va_end(args);
    return num_chars_written;
}
