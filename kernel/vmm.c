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

/* Basic variables */
//First pd
int first = 0;

//Last va that caused pagefault
uint32_t lastVA = 0;

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
    //MISSING();
    vmm_on((uint32_t) pd);
}

uint32_t last(void) {
    //MISSING();
    return lastVA;
}

/* handle a page fault */
void pageFault(uint32_t addr) {
    putStr("fault at ");
    putHex(addr);
    putStr("\n");
    //MISSING();
    lastVA = addr;
    uint32_t pa = frame();
    table_s *table = (table_s *) pa;
    if (addr >= 0x80000000) {
        for (int i = 0; i < FRAME_SIZE; i++) {
            table->entries[i] = (addr & 0xFFFFF000) + 4 * i;
        }
    }
    map(addr, pa); 
}

/* Create a new mapping from va to pa */
void map(uint32_t va, uint32_t pa) {
    //MISSING();
    if (first == 0) {
        first++;
        pd = (table_s *) frame();
    }

    uint32_t pdIndex = (uint32_t) va >> 22 & 0x3FF;
    uint32_t ptIndex = (uint32_t) va >> 12 & 0x3FF;

    uint32_t pdEntry = (uint32_t) (pd->entries[pdIndex]);
    uint32_t pdPresent = ((uint32_t) pdEntry) & 0x1;
    if (!pdPresent) {
        pd->entries[pdIndex] = (uint32_t) ((frame() & 0xFFFFF000) | 0x1);
    }

    table_s *pt = (table_s *) (pd->entries[pdIndex] & 0xFFFFF000);
    pt->entries[ptIndex] = ((uint32_t) pa) | 0x1;
}

/* unmap the given va */
int forget(uint32_t va) {
    //MISSING()
    uint32_t pdIndex = (uint32_t) va >> 22 & 0x3FF;
    uint32_t ptIndex = (uint32_t) va >> 12 & 0x3FF;

    uint32_t pdEntry = (uint32_t) (pd->entries[pdIndex]);
    uint32_t pdPresent = ((uint32_t) pdEntry) & 0x1;
    if (!pdPresent) {
        pd->entries[pdIndex] = (uint32_t) ((frame() & 0xFFFFF000) | 0x1);
    }

    table_s *pt = (table_s *) (pd->entries[pdIndex] & 0xFFFFF000);

    int forgetBit = (pt->entries[ptIndex] >> 6) & 0x1;
    pt->entries[ptIndex] = 0x00000000;
    invlpg(va);
    return forgetBit;
}

void share(uint32_t from, uint32_t to) {
    //MISSING();
    //Forget to
    forget(to);

    //From
    uint32_t pdFromIndex = (uint32_t) from >> 22 & 0x3FF;
    uint32_t ptFromIndex = (uint32_t) from >> 12 & 0x3FF;

    uint32_t pdFromEntry = (uint32_t) (pd->entries[pdFromIndex]);
    uint32_t pdFromPresent = ((uint32_t) pdFromEntry) & 0x1;
    if (!pdFromPresent) {
        pd->entries[pdFromIndex] = (uint32_t) ((frame() & 0xFFFFF000) | 0x1);
    }

    table_s *ptFrom = (table_s *) (pd->entries[pdFromIndex] & 0xFFFFF000);

    uint32_t pa = (uint32_t) (ptFrom->entries[ptFromIndex] & 0xFFFFF000);
    
    //To
    uint32_t pdToIndex = (uint32_t) to >> 22 & 0x3FF;
    uint32_t ptToIndex = (uint32_t) to >> 12 & 0x3FF;

    uint32_t pdToEntry = (uint32_t) (pd->entries[pdToIndex]);
    uint32_t pdToPresent = (uint32_t) pdToEntry & 0x1;
    if (!pdToPresent) {
        pd->entries[pdToIndex] = (uint32_t) ((frame() & 0xFFFFF000) | 0x1);
    }

    table_s *ptTo = (table_s *) (pd->entries[pdToIndex] & 0xFFFFF000);

    ptTo->entries[ptToIndex] = pa | 0x1;
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

