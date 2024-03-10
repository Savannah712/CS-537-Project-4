#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "wmap.h"          // TODO: added
// #include "vm.c"           // TODO: added


int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int
sys_wmap(void)
{
    int addr;
    int length;
    int flags;
    int fd;

    if (argint(0, &addr) < 0 || argint(1, &length) < 0 || argint(2, &flags) < 0 || argint(3, &fd) < 0)
        return 1;
        
    pte_t *pte;
    int found = 0;
    int count = 0;

    int tot = myproc()->total_mmaps;

    if ((tot > 0) && ((flags & MAP_FIXED) != MAP_FIXED)) {

      while (found == 0){
        addr = 0x60000000 + (4096 * count);
        pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)addr), 1);
        if (*pte & PTE_P) count++;
        else found = 1;
      }
    }

    int iterations = (length / 4096);
    if (length % 4096 != 0) {
      iterations++;
    }
    int currAddr = addr;

    char *mem;

    // if (length == 8200) return iterations;
    
    for (int i = 0; i < iterations; i++) {
      pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
      if (*pte & PTE_P) return FAILED;
      // for (int j = 0; j < myproc()->total_mmaps ; j++) {
      //   for (int k = 0; k < myproc()->length[j]; k++){
      //     if ((myproc()->addr[j] + (4096 * j)) == currAddr) return FAILED;
      //     // if (length == 12296) return 
      //   }
      // }
      mem = kalloc();
      mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
      currAddr = currAddr + 4096;


    }

    // walkpgdir(myproc()->pgdir, (void *)(addr), 0);

    // Update process's memory mappings
    // myproc()->num_mappings++;
    myproc()->total_mmaps++;
    myproc()->addr[tot] = addr;
    myproc()->length[tot] = length;
    myproc()->n_loaded_pages[tot] = iterations;
    myproc()->n_upages = myproc()->n_upages + iterations;

    // Return the mapped address

    return addr;
    

    // Check if the requested flags are valid
    // if ((flags & (MAP_ANONYMOUS | MAP_FIXED | MAP_SHARED)) != (MAP_ANONYMOUS | MAP_FIXED | MAP_SHARED))
    //     return 2;  // Invalid flags combination


    // if(c == 0)
    //   break;
    // switch(c){
    // case 'd':
    //   printint(*argp++, 10, 1);
    //   break;
    // case 'x':
    // case 'p':
    //   printint(*argp++, 16, 0);
    //   break;
    // case 's':
    //   if((s = (char*)*argp++) == 0)
    //     s = "(null)";
    //   for(; *s; s++)
    //     consputc(*s);
    //   break;
    // case '%':
    //   consputc('%');
    //   break;
    // default:
    //   // Print unknown % sequence to draw attention.
    //   consputc('%');
    //   consputc(c);
    //   break;
    // }

    // // // Check if the requested region is within the user address space
    // if ((addr < 0x60000000) || (addr >= KERNBASE))
    //     return 3;  // Invalid address range

    // // Check if the requested region is available
    // for (int i = 0; i < length; i += PGSIZE) {
    //     pte_t *pte = walkpgdir(myproc()->pgdir, (void *)(addr + i), 0);
    //     if (pte && *pte)
    //         return 4;  // Address already mapped
    // }

    // // Map the pages in the requested region
    // for (int i = 0; i < length; i += PGSIZE) {
    //     char *mem = kalloc(); // Allocate physical memory
    //     if (mem == 0) {
    //         // Failed to allocate memory
    //         // Unmap previously mapped pages
    //         for (int j = 0; j < i; j += PGSIZE) {
    //             pte_t *pte = walkpgdir(myproc()->pgdir, (void *)(addr + j), 0);
    //             if (pte) {
    //                 *pte = 0;  // Clear the page table entry
    //                 // Invalidate TLB
    //                 lcr3(V2P(myproc()->pgdir));
    //             }
    //         }
    //         return 5;
    //     }
    //     // Map the physical memory to the requested virtual address
    //     if (mappages(myproc()->pgdir, (void *)(addr + i), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
    //         // Failed to map memory
    //         // Free the allocated physical memory
    //         kfree(mem);
    //         // Unmap previously mapped pages
    //         for (int j = 0; j < i; j += PGSIZE) {
    //             pte_t *pte = walkpgdir(myproc()->pgdir, (void *)(addr + j), 0);
    //             if (pte) {
    //                 *pte = 0;  // Clear the page table entry
    //                 // Invalidate TLB
    //                 lcr3(V2P(myproc()->pgdir));
    //             }
    //         }
    //         return -1;
    //     }
    // }
    
    // // Check if the number of mappings exceeds the maximum limit
    // struct proc *curproc = myproc();
    // if (curproc->num_mappings >= 16)
    //     return -1;

    // // Populate the new mapping
    // struct wmapinfo *new_mapping = &curproc->mappings[curproc->num_mappings++];
    // new_mapping->addr = addr;
    // new_mapping->length = length;
    // new_mapping->n_loaded_pages = 0; // Initialize the number of loaded pages to 0


    // return addr;
  }



int
sys_wunmap(void)
{
    // uint addr;
    // Your implementation here
    return 0;
}

int
sys_wremap(void)
{
    // uint oldaddr;
    // int oldsize;
    // int newsize;
    // int flags;
    // Your implementation here
    return 0;
}

int
sys_getpgdirinfo(void)
{
    struct pgdirinfo *pdinfo;


    if (argptr(0, (void *)&pdinfo, sizeof(struct pgdirinfo)) < 0)
        return FAILED; // Invalid pointer

    struct proc *curproc = myproc();
    if (curproc == 0)
        return FAILED; // No current process

    pdinfo->n_upages = 0; // Initialize the count of user pages

    // Iterate over the page directory and page tables
    int idx = 0; // Index for storing virtual and physical addresses
    for (int i = 0; i < NPDENTRIES && idx < MAX_UPAGE_INFO; i++) {
        pde_t *pgtab = (pde_t *)P2V(PTE_ADDR(curproc->pgdir[i]));
        for (int j = 0; j < NPTENTRIES && idx < MAX_UPAGE_INFO; j++) {
            pte_t pte = pgtab[j];
            if (pte & PTE_P && pte & PTE_U) { // If page is present and user accessible
                pdinfo->va[idx] = PGADDR(i, j, 0); // Store virtual address
                pdinfo->pa[idx] = PTE_ADDR(pte);   // Store physical address
                pdinfo->n_upages++;                // Increment count of user pages
                idx++;                             // Increment index
            }
        }
    }

    return SUCCESS;
}



int
sys_getwmapinfo(void)
{
    struct wmapinfo *wminfo;
    if (argptr(0, (void *)&wminfo, sizeof(*wminfo)) < 0)
    return -1;

    int tot = myproc()->total_mmaps;
    wminfo->total_mmaps = tot;
    
    for (int i = 0; i < tot; i++) {
    wminfo->addr[i] = myproc()->addr[i];
    wminfo->length[i] = myproc()->length[i];
    wminfo->n_loaded_pages[i] = myproc()->n_loaded_pages[i];
    }
  

    return 0;
}
