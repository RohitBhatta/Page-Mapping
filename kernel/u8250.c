#include "u8250.h"
#include "machine.h"

/* 8250 */

void u8250_putChar(char c) {
    while (!(inb(0x3F8+5) & 0x20));
    outb(0x3F8,c);
}
