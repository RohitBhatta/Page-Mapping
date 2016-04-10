#ifndef _VMM_H_
#define _VMM_H_

#include "stdint.h"

/* returns the va that caused the last page fault then clears it */
extern uint32_t last(void);

/* allocate a new physical frame */
extern uint32_t frame(void);

/* turn paging on */
extern void paging(void);

/* Creata a mapping from va to pa, offset bits are ignored */
extern void map(uint32_t va, uint32_t pa);

/* unmap the page that contains the given VA, returns 1 if the page 
   was dirty */
extern int forget(uint32_t va);

/* to_va points to the same frame as from_va */
extern void share(uint32_t from_va, uint32_t to_va);

#endif
