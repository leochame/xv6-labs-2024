// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct{
  struct spinlock allLock;
  struct spinlock lock[MAXPAGES/PGSIZE];
  int refcount[MAXPAGES/PGSIZE];
}refcount;

static int
pa2index(uint64 pa)
{
  if (pa < (uint64)end)
    return -1;
  uint64 idx = (pa - (uint64)end) / PGSIZE;
  if (idx >= MAXPAGES/PGSIZE)
    return -1;
  return idx;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initrefcount();
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}
void
initrefcount(){
  int count = 0;
  for(; count < MAXPAGES/PGSIZE; count++)
    set_ref(count,1);
  printf("freerange: initialized %d pages\n", count);
}
//增加引用计数
void
inc_ref(void *pa){
  int idx = pa2index((uint64)pa);
  acquire(&refcount.lock[idx]);
  if (idx >= 0)
  	refcount.refcount[idx]++;
  release(&refcount.lock[idx]);
}

// 获取引用计数
int get_ref(void *pa)
{
  int idx = pa2index((uint64)pa);
  int count = -1;
  acquire(&refcount.lock[idx]);
  if (idx >= 0)
  	count = refcount.refcount[idx];
  release(&refcount.lock[idx]);
  return count;
}
void
set_ref(int idx, int value)
{
  acquire(&refcount.lock[idx]);
  if (idx < 0)
  	printf("set_ref: index out of range,%d\n", idx);
  refcount.refcount[idx] = value;
  release(&refcount.lock[idx]);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");


  int idx = pa2index((uint64)pa);
  acquire(&refcount.lock[idx]);
  if (idx < 0 || refcount.refcount[idx] <= 0){
    release(&refcount.lock[idx]);
    printf("end is %lx,pa is %lx,idx is %d,ref count is %d\n", (uint64)end,(uint64)pa, idx, refcount.refcount[idx]);
    panic("kfree: invalid pa or refcount");
  }
  int count = --refcount.refcount[idx];
  release(&refcount.lock[idx]);
  if(count > 0 )
    return;// refence count is not zero, we cant free that pa
  // Fill with junk to catch dangling refs.


  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
  {
    memset((char*)r, 5, PGSIZE); // fill with junk
    uint64 idx = pa2index((uint64)r);
    acquire(&refcount.lock[idx]);
    refcount.refcount[idx] = 1; // 设置引用计数为1
    release(&refcount.lock[idx]);
  }
  return (void*)r;
}
