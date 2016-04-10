#include "vmm.h"
#include "console.h"
#include "stdint.h"
#include "machine.h"

#define MISSING() do { \
    putStr(__FILE__); \
    putStr(":"); \
    putDec(__LINE__); \
    putStr(" is missing\n"); \
    shutdown(); \
} while (0)

/* Each frame is 4K */
#define FRAME_SIZE (1 << 12)

/* A table contains 4K/4 = 1K page table entries */
#define TABLE_ENTRIES (FRAME_SIZE / sizeof(uint32_t))

/* A table, either a PD, or a PT */
typedef struct {
    uint32_t entries[TABLE_ENTRIES];
} table_s;


/* Address of first avaialble frame */
uint32_t avail = 0x100000;

/* pointer to page directory */
table_s *pd = 0;

/* The world's simplest frame allocator */
uint32_t frame(void) {
    uint32_t ptr = avail;
    avail += FRAME_SIZE;
    char* p = (char*) ptr;
    for (int i=0; i<FRAME_SIZE; i++) {
        p[i] = 0;
    }
    return ptr;
}

extern void vmm_on(uint32_t);

void paging(void) {
    MISSING();
}

uint32_t last(void) {
    MISSING();
    return 0;
}

/* handle a page fault */
void pageFault(uint32_t addr) {
   putStr("fault at ");
   putHex(addr);
   putStr("\n");
   MISSING();
}

/* Create a new mapping from va to pa */
void map(uint32_t va, uint32_t pa) {
    MISSING();
}

/* unmap the given va */
int forget(uint32_t va) {
    MISSING();
    return 0;
}

void share(uint32_t from, uint32_t to) {
    MISSING();
}

/* print the contents of the page table */
#if 0
void vmm_dump() {
    table_s *pd = getPD();
    sayHex("PD @ ",(uint32_t) pd);
    for (int i=0; i<TABLE_ENTRIES; i++) {
        uint32_t e = pd->entries[i];
        if (e != 0) {
            putStr("    ");
            putHex(i);
            sayHex(") PDE = ",e);
            table_s * pt = (table_s*) (e & 0xfffff000);
            for (int j=0; j<TABLE_ENTRIES; j++) {
                uint32_t e = pt->entries[j];
                if (e != 0) {
                    putStr("        ");
                    putHex(j);
                    sayHex(") PTE = ",e);
                }
            }
        }
    }
}
#endif

