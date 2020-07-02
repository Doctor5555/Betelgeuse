#include "keyboard.h"

#define SHIFT 0b00000001
//#define RSHIFT 0b00000010
#define CTRL   0b00000100
#define ALT    0b00001000
#define CAPS   0b10000000

u8 inportb(u16 _port) {
    u8 rv;
    __asm__ __volatile__("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void kb_read(kb_result *result) {
    char c;
    u8 reading = 1;
    u8 modifiers = result->modifiers; // caps_active 0 0 0 0 0 ctrl shift
    while (reading) {
        if (inportb(0x64) & 0x1) {
            u8 scancode = inportb(0x60);
            result->scancode = scancode;
            reading = 0;
            /*
            {
                //kprintch('\r');
                u8 num = kprintden(scancode);
                num = 3 - num;
                for (num; num > 0; num--) {
                    //kprintch(' ');
                }
                set_cursor_x(11 + i);
            }
            */
            switch (scancode) {
            case 1: //ESC key
                break;
            case 2:
                c = (modifiers & SHIFT) ? '!' : '1';
                break;
            case 3:
                c = (modifiers & SHIFT) ? '@' : '2';
                break;
            case 4:
                c = (modifiers & SHIFT) ? '#' : '3';
                break;
            case 5:
                c = (modifiers & SHIFT) ? '$' : '4';
                break;
            case 6:
                c = (modifiers & SHIFT) ? '%' : '5';
                break;
            case 7:
                c = (modifiers & SHIFT) ? '^' : '6';
                break;
            case 8:
                c = (modifiers & SHIFT) ? '&' : '7';
                break;
            case 9:
                c = (modifiers & SHIFT) ? '*' : '8';
                break;
            case 10:
                c = (modifiers & SHIFT) ? '(' : '9';
                break;
            case 11:
                c = (modifiers & SHIFT) ? ')' : '0';
                break;
            case 12:
                c = (modifiers & SHIFT) ? '_' : '-';
                break;
            case 13:
                c = (modifiers & SHIFT) ? '+' : '=';
                break;
            case 14:
                c = 0x08;
                //if (i > 0) {
                    //kprintch(0x08);
                    //i--;
                //}
                //buffer_string[i] = 0;
                break;
            case 15:
                c = '\t';
                //kprintch('\t');
                //buffer_string[i] = ' ';

                break;
            case 16:
                c = (modifiers & SHIFT) ? 'Q' : 'q';
                break;
            case 17:
                c = (modifiers & SHIFT) ? 'W' : 'w';
                break;
            case 18:
                c = (modifiers & SHIFT) ? 'E' : 'e';
                break;
            case 19:
                c = (modifiers & SHIFT) ? 'R' : 'r';
                break;
            case 20:
                c = (modifiers & SHIFT) ? 'T' : 't';
                break;
            case 21:
                c = (modifiers & SHIFT) ? 'Y' : 'y';
                break;
            case 22:
                c = (modifiers & SHIFT) ? 'U' : 'u';
                break;
            case 23:
                c = (modifiers & SHIFT) ? 'I' : 'i';
                break;
            case 24:
                c = (modifiers & SHIFT) ? 'O' : 'o';
                break;
            case 25:
                c = (modifiers & SHIFT) ? 'P' : 'p';
                break;
            case 26:
                c = (modifiers & SHIFT) ? '{' : '[';
                break;
            case 27:
                c = (modifiers & SHIFT) ? '}' : ']';
                break;
            case 28: // ENTER key
                c = '\n';
                break;
            case 29:
                result->do_not_print = 1;
                modifiers |= CTRL;
                break;
            case 30:
                c = (modifiers & SHIFT) ? 'A' : 'a';
                break;
            case 31:
                c = (modifiers & SHIFT) ? 'S' : 's';
                break;
            case 32:
                c = (modifiers & SHIFT) ? 'D' : 'd';
                break;
            case 33:
                c = (modifiers & SHIFT) ? 'F' : 'f';
                break;
            case 34:
                c = (modifiers & SHIFT) ? 'G' : 'g';
                break;
            case 35:
                c = (modifiers & SHIFT) ? 'H' : 'h';
                break;
            case 36:
                c = (modifiers & SHIFT) ? 'J' : 'j';
                break;
            case 37:
                c = (modifiers & SHIFT) ? 'K' : 'k';
                break;
            case 38:
                c = (modifiers & SHIFT) ? 'L' : 'l';
                break;
            case 39:
                c = (modifiers & SHIFT) ? ':' : ';';
                break;
            case 40:
                c = (modifiers & SHIFT) ? '"' : '\'';
                break;
            case 41:
                c = (modifiers & SHIFT) ? '~' : '`';
                break;
            case 42:
                result->do_not_print = 1;
                modifiers |= SHIFT;
                break;
            case 43:
                c = (modifiers & SHIFT) ? '|' : '\\';
                break;
            case 44:
                c = (modifiers & SHIFT) ? 'Z' : 'z';
                break;
            case 45:
                c = (modifiers & SHIFT) ? 'X' : 'x';
                break;
            case 46:
                c = (modifiers & SHIFT) ? 'C' : 'c';
                break;
            case 47:
                c = (modifiers & SHIFT) ? 'V' : 'v';
                break;
            case 48:
                c = (modifiers & SHIFT) ? 'B' : 'b';
                break;
            case 49:
                c = (modifiers & SHIFT) ? 'N' : 'n';
                break;
            case 50:
                c = (modifiers & SHIFT) ? 'M' : 'm';
                break;
            case 51:
                c = (modifiers & SHIFT) ? '<' : ',';
                break;
            case 52:
                c = (modifiers & SHIFT) ? '>' : '.';
                break;
            case 53:
                c = (modifiers & SHIFT) ? '?' : '/';
                break;
            case 54:
                result->do_not_print = 1;
                modifiers |= SHIFT;
                break;
            case 55: // Numpad *
                c = '*';
                break;
            case 56:
                result->do_not_print = 1;
                modifiers |= ALT;
                break;
            case 57:
                c = ' ';
                break;
            case 58:
                result->do_not_print = 1;
                modifiers |= CAPS;
                break;
            /* 59 - 68 F1 - F10 */

            /* Numpad keys: */
            case 71:
                c = '7';
                break;
            case 72:
                c = '8';
                break;
            case 73:
                c = '9';
                break;
            case 74:
                c = '-';
                break;
            case 75:
                c = '4';
                break;
            case 76:
                c = '5';
                break;
            case 77:
                c = '6';
                break;
            case 78:
                c = '+';
                break;
            case 79:
                c = '1';
                break;
            case 80:
                c = '2';
                break;
            case 81:
                c = '3';
                break;
            case 82:
                c = '0';
                break;
            case 83:
                c = '.';
                break;

            case 157:
                result->do_not_print = 1;
                modifiers &= ~CTRL;
                break;
            case 170:
                result->do_not_print = 1;
                modifiers &= ~SHIFT;
                break;
            case 182:
                result->do_not_print = 1;
                modifiers &= ~SHIFT;
                break;
            default:
                result->do_not_print = 1;
                break;
            }
        }
    }

    result->c = c;
    result->modifiers = modifiers;
}