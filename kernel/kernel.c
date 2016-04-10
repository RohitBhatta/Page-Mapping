
#include "ide.h"
#include "machine.h"
#include "mmu.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "console.h"
#include "vmm.h"

#define ANNOUNCE(code) do { \
    putStr(#code); putStr(" ... "); \
    code; \
    putStr("done\n"); \
} while (0)

int passed = 0;

void check(char* file, int line, char* expr, uint32_t found, uint32_t expected) {
    
    if (expected == found) {
        passed ++;
    } else {
        putStr(file);
        putStr(":");
        putDec(line);
        putStr(" -- (");
        putStr(expr);
        putStr("), expected: ");
        putHex(expected);
        putStr(", found ");
        putHex(found);
        putStr("\n");
        shutdown();
    }
}

#define CHECK(cond,expected) do { \
    check(__FILE__,__LINE__,#cond,(cond),(expected)); \
} while (0)

uint32_t peek(uint32_t ptr) {
    return *((volatile uint32_t*)ptr);
}

void poke(uint32_t ptr, uint32_t val) {
    *((volatile uint32_t*)ptr) = val;
}

void kernelMain(void) {
    putStr("\nstarting kernel\n");

    initGdt();
    initTss();
    initIdt();

    CHECK((getcr0() & 0x80000000), 0);

    putStr("about to enable paging\n");

    /* Explain this */
    for (uint32_t p = 0; p < 0x200000; p += 4096) {
        map(p,p);
    }

    /* enable paging */
    paging();

    putStr("we're back\n");

    CHECK(getcr0() & 0x80000000 , 0x80000000);

    uint32_t pa = frame();                   /* get a frame */
    map(0xf0000000,pa);                      /* map it at 0xf0000000 */
    CHECK(peek(0xf0000000),0);               /* should be zero-filled */
    CHECK(last(),0);                         /* no fault */
    poke(0xf0000000,0xdeadbeef);             /* put something in it */
    CHECK(peek(0xf0000000),0xdeadbeef);      /* should see the same value */
    CHECK(last(),0);                         /* no fault */
    map(0xe0000000,pa);                      /* map it at 0xe0000000 */
    CHECK(peek(0xe0000000),0xdeadbeef);      /* should see the same value */
    CHECK(last(),0);                         /* no fault */

    CHECK(forget(0xf0000000),1);             /* 0xf0000000 should be dirty */
    CHECK(forget(0xe0000000),0);             /* 0xe0000000 should be clean */

    CHECK(peek(0x00300000), 0x00000000);     /* zero-filled mapping */
    CHECK(last(),0x00300000);                /* fault */
    CHECK(forget(0x00300000),0);             /* forget it, should be clean */
    poke(0x00300000,0x1);                    /* a new zero-filled mapping */
    CHECK(last(),0x00300000);                /* fault */
    CHECK(peek(0x00300000), 0x00000001);     /* should be 1 now */
    CHECK(forget(0x00300000),1);             /* forget it, should be dirty */
    CHECK(peek(0x00300000), 0x00000000);     /* should be zero again */

    /* address-filled mapping */
    CHECK(peek(0xcccccccc),0xcccccccc);
    CHECK(last(),0xcccccccc);
    poke(0xcccccccc,0xc);
    CHECK(peek(0xcccccccc),0xc);

    /* from address-filled to zero-filled */
    CHECK(peek(0xaaaaaaa0),0xaaaaaaa0);
    CHECK(last(),0xaaaaaaa0);
    share(0xaaaaa000,0x0aaaa000);
    CHECK(peek(0x0aaaaaa0),0xaaaaaaa0);
    CHECK(last(),0xaaaaaaa0);

    /* from zero-filled to address-filled */
    CHECK(peek(0x0bbbbbb0),0x0);
    share(0x0bbbb000,0xbbbbb000);
    CHECK(peek(0xbbbbbbb0),0);

    poke(0x00300010,0x2);                    /* new mapping */
    CHECK(peek(0x00300010),2);               /* should be 2 */
    share(0x00300000,0x00400000);            /* share with 0x00400000 */
    CHECK(peek(0x00400010),2);               /* 0x00400010 should also be 2 */
    CHECK(forget(0x00300000),1);             /* forget it, should be dirty */
    CHECK(peek(0x00400010),2);               /* 0x00400010 should still be 2 */
    CHECK(forget(0x00400000),0);             /* forget it, should be clean */
    

    poke(0xf0000010,0xffffffff);
    CHECK(peek(0xf0000000), 0xf0000000);
    CHECK(peek(0xf000000c), 0xf000000c);
    CHECK(peek(0xf0000010), 0xffffffff);
    CHECK(peek(0xf0000014), 0xf0000014);

    poke(0x70000010,0xffffffff);
    CHECK(peek(0x70000000), 0x00000000);
    CHECK(peek(0x7000000c), 0x00000000);
    CHECK(peek(0x70000010), 0xffffffff);
    CHECK(peek(0x70000014), 0x00000000);

    poke(0x55555000,0x5);
    share(0x55555000,0x66666000);
    CHECK(peek(0x66666000),0x5);

    CHECK(forget(0x55555000),1);
    CHECK(peek(0x66666000),0x5);
    CHECK(forget(0x66666666),0);
    CHECK(peek(0x55555000),0x0);

    CHECK(peek(0x98888888), 0x98888888);
    poke(0x98888888,0x77777777);
    CHECK(peek(0x98888888), 0x77777777);

    CHECK(peek(0x08888888), 0x00000000);
    poke(0x08888888,0x66666666);
    CHECK(peek(0x08888888), 0x66666666);

    share(0x98888888,0x08888888);
    CHECK(peek(0x08888000), 0x98888000);
    CHECK(peek(0x08888004), 0x98888004);
    CHECK(peek(0x08888888), 0x77777777);
    CHECK(peek(0x08888ffc), 0x98888ffc);

    poke(0x98888888,0x1);
    CHECK(peek(0x98888888), 1);
    CHECK(peek(0x08888888), 1);

    poke(0x08888884, 0x2);
    CHECK(peek(0x98888884), 2);
    CHECK(peek(0x08888884), 2);

    poke(0xe1111000,0x3);
    CHECK(peek(0xe1111000), 3);
    CHECK(peek(0xe1111888), 0xe1111888);

    CHECK(forget(0xe1111000),1);
    CHECK(peek(0xe1111000), 0xe1111000);
    CHECK(peek(0xe1111888), 0xe1111888);

    CHECK(forget(0x98888000),1);
    CHECK(peek(0x98888884), 0x98888884);
    CHECK(peek(0x08888884), 2);

    putStr("passed ");
    putDec(passed);
    putStr(" tests\n");
    
    shutdown();
}
