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





/*
 *-looks through table
	-if segment id doesnt exists -> allocate page, map it -> store info in shm table
		-grab lock
	-if exists -> 

  -
 *
 */
int shm_open(int id, char **pointer) {

//you write this
  struct proc *curr = myproc();
  int i;
  char *va;
  
  
  va = (char*)PGROUNDUP(curr->sz);
  
  //acquire lock so no race conditions of ref cnt
  acquire(&(shm_table.lock));


  //segment id exists
  for(i = 0; i<64; i++)
  {     
	if(shm_table.shm_pages[i].id == id)
	{ 
          //increment ref count
	  shm_table.shm_pages[i].refcnt += 1;
	  //map it to va -> a free virtual address
	  mappages(curr->pgdir, va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
	  
	  //update sz in curr proc
	  curr->sz = (uint) va;

	  
          //initialize pointer to shared page
	  *pointer = (char*) va;

	  //return ref count
	  int cnt = shm_table.shm_pages[i].refcnt;

	  //release lock
	  release(&(shm_table.lock));
	  return cnt;
	}
  }


  //if doesnt exist
  //allocate page, map to shm table to store info
  for( i = 0; i<64; ++i)
  {
	//this is default if there is no page allocated
	//find empty entry
	if(shm_table.shm_pages[i].id == 0)
	{
		//initialize info
		struct shm_page *page = shm_table.shm_pages + i;
		page->id = id; //page id
		page->frame = kalloc();//no malloc, allocate mem for passed pointer
		memset(page->frame, 0, PGSIZE); //must be done after allocating page
		page->refcnt += 1; //increment ref count

		//map page in a free virtual address
		mappages(curr->pgdir, va, PGSIZE, V2P(page->frame), PTE_W|PTE_U);
		//initialize pointer to shared page
		*pointer = (char*) (va);
		curr->sz = (uint) va;

		//increment refcbt
		page->refcnt += 1;
		int cnt = page-> refcnt;
		//release lock
		release(&(shm_table.lock));
		return cnt;	
	} 
  }
	




return 0; //added to remove compiler warning -- you should decide what to return
}



/*
 *-looks for the shared memory segment id already exists.
 *-finds it -> decrements -> if 0 -> clears shm_table
  -no need to free up page (already within the system)
 */
int shm_close(int id) {
//you write this too!

  int i;
  //acquire lock to avoid race condition of changing info like ref count
  acquire(&(shm_table.lock));
  for(i =0; i<64; ++i)
  {     
	//find the id
	if(shm_table.shm_pages[i].id == id)
	{
		//decrement refcnt
		shm_table.shm_pages[i].refcnt -=1;

		//check if 0
		if(!shm_table.shm_pages[i].refcnt)
		{	
			//clear the page, refcnt, 
			shm_table.shm_pages[i].id = 0;
			//shm_table.shm_pages[i].refcnt = 0;
			shm_table.shm_pages[i].frame = 0;
		}
	}	
  }
  release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}
