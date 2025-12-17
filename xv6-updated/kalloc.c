// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct {
  struct spinlock lock;
  int count[PHYSTOP / PGSIZE];  // Reference count for each physical page
} pageref;

//Helper to get safe index from physical address
static int
pa2idx(uint pa)
{
  if(pa < (uint)V2P(end) || pa >= PHYSTOP)
    return -1;
  return pa / PGSIZE;
}

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.

void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  initlock(&pageref.lock, "pageref");  // ADD THIS
  
  // Initialize all reference counts to 0
  memset(pageref.count, 0, sizeof(pageref.count));  // ADD THIS
  
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Only free if reference count reaches 0
  if(kderefpage(v) > 0)  // ADD THIS
    return;               // ADD THIS

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);
  r = (struct run*)v;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE);  // Fill with junk
    krefpage((char*)r);  // To initialize refcount to 1
  }
  
  return (char*)r;
}

// To increment reference count for a physical page
void
krefpage(void *pa)
{
  int idx;
  
  if((uint)pa % PGSIZE || (uint)pa < KERNBASE || (uint)pa >= PHYSTOP)
    return;
  
  idx = pa2idx(V2P(pa));
  if(idx < 0 || idx >= PHYSTOP / PGSIZE)
    return;
    
  acquire(&pageref.lock);
  pageref.count[idx]++;
  release(&pageref.lock);
}

// To decrement reference count and return new count
int
kderefpage(void *pa)
{
  int idx;
  int count;
  
  if((uint)pa % PGSIZE || (uint)pa < KERNBASE || (uint)pa >= PHYSTOP)
    return 0;
  
  idx = pa2idx(V2P(pa));
  if(idx < 0 || idx >= PHYSTOP / PGSIZE)
    return 0;
    
  acquire(&pageref.lock);
  if(pageref.count[idx] > 0)
    pageref.count[idx]--;
  count = pageref.count[idx];
  release(&pageref.lock);
  
  return count;
}

// Getting current reference count
int
kgetrefcount(void *pa)
{
  int idx;
  int count;
  
  if((uint)pa % PGSIZE || (uint)pa < KERNBASE || (uint)pa >= PHYSTOP)
    return 0;
  
  idx = pa2idx(V2P(pa));
  if(idx < 0 || idx >= PHYSTOP / PGSIZE)
    return 0;
    
  acquire(&pageref.lock);
  count = pageref.count[idx];
  release(&pageref.lock);
  
  return count;
}