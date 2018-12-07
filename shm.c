#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

//you write this
  struct proc *curr = myproc();
  int i;
  char *va;
  va = (char*)PGROUNDUP(curr->sz);

  acquire(&(shm_table.lock));
  for(i = 0; i<64; i++)
  {
	if(shm_table.shm_pages[i].id == id)
	{
	  shm_table.shm_pages[i].refcnt += 1;
	  mappages(curr->pgdir, va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
	  curr->sz = (uint) va;
	  *pointer = (char*) va;
	  int cnt = shm_table.shm_pages[i].refcnt;
	  release(&(shm_table.lock));
	  return cnt;
	}
  }

  for( i = 0; i<64; ++i)
  {

	if(shm_table.shm_pages[i].id == 0)
	{
		struct shm_page *page = shm_table.shm_pages + i;
		page->id = id;
		page->frame = kalloc();
		memset(page->frame, 0, PGSIZE);
		page->refcnt += 1;
		mappages(curr->pgdir, va, PGSIZE, V2P(page->frame), PTE_W|PTE_U);
		*pointer = (char*) (va);
		curr->sz = (uint) va;
		page->refcnt += 1;
		int cnt = page-> refcnt;
		release(&(shm_table.lock));
		return cnt;	
	} 
  }
	




return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!

  int i;
  acquire(&(shm_table.lock));
  for(i =0; i<64; ++i)
  {
	if(shm_table.shm_pages[i].id == id)
	{
		shm_table.shm_pages[i].refcnt -=1;
		if(!shm_table.shm_pages[i].refcnt)
		{
			shm_table.shm_pages[i].id = 0;
			shm_table.shm_pages[i].refcnt = 0;
			shm_table.shm_pages[i].frame = 0;
		}
	}	
  }
  release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}
