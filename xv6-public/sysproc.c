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
        return FAILED;
        
    pte_t *pte;
    int found = 0;
    int count = 0;

    int tot_maps = myproc()->total_mmaps;
    int tot_upge = myproc()->n_upages;

    // UNFIXED mapping
    if ((tot_maps > 0) && ((flags & MAP_FIXED) != MAP_FIXED)) {
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
    
    for (int i = 0; i < iterations; i++) {
      pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
      if (*pte & PTE_P) return FAILED;

      mem = kalloc();
      mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
      myproc()->va[tot_upge + i] = currAddr;
      myproc()->pa[tot_upge + i] = V2P(mem);
      myproc()->vld_pge[tot_upge + i] = 1;


      if ((flags & MAP_ANONYMOUS) == 0) { 
        // Read the file into memory
        struct file *f = myproc()->ofile[fd];
         fileread(f, (void*)currAddr, 4096);
        // if (bytes_read != length) {
        //     kfree(mem);
        //     return bytes_read; // Failed to read the file into memory
        // }
      }   

      currAddr = currAddr + 4096;


    }



    myproc()->total_mmaps++;
    int i = 0;
    while (myproc()->vld_map[i] == 1) {
      i++;
    }
    myproc()->addr[i] = addr;
    myproc()->vld_map[i] = 1;
    myproc()->length[i] = length;
    myproc()->n_loaded_pages[i] = iterations;
    myproc()->n_upages = myproc()->n_upages + iterations;

    // Return the mapped address

    return addr;
  }



int
sys_wunmap(void)
{
    int addr;

    if (argint(0, &addr) < 0)
      return FAILED;

    if ((addr % 4096 != 0) || (addr < 0x60000000))
      return FAILED; 

    // int iterations = myproc()->total_mmaps;

      // currAddr = addr + (4096 * j);
      // pte_t *pte = walkpgdir(myproc()->pgdir, addr, 0);
      pte_t *pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)addr), 1);

      uint pa = PTE_ADDR(*pte);
      kfree(P2V(pa));


      myproc()->total_mmaps--;
      // myproc()->addr[tot_maps] = addr;
      // myproc()->length[tot_maps] = length;
      // myproc()->n_loaded_pages[tot_maps] = iterations;
      // myproc()->n_upages = myproc()->n_upages + iterations;

    // int currAddr = addr;
    // int found = 0;
    // int i = 0;

    // while ((found == 0) && (i < iterations)) {
    //   if (myproc()->addr[i] == addr) {
    //     found = 1;
    //   } else {
    //     i++;
    //   }
    // }

    // int map_length = myproc()->length[i];

    // for (int j = 0; j < map_length; j++) {
    //   currAddr = addr + (4096 * j);
    //   // pte_t *pte = walkpgdir(myproc()->pgdir, currAddr, 0);
    //   pte_t *pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);

    //   uint pa = PTE_ADDR(*pte);
    //   kfree(P2V(pa));
    // }


    // pte_t *pte = walkpgdir(myproc()->pgdir, addr, 0);

    // char * pa = PTE_ADDR(*pte);
    // kfree(P2V(pa));

    // Your implementation here
    return SUCCESS;
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

    int tot = myproc()->n_upages;
    pdinfo->n_upages = tot;
    int currPge = 0;
    int i = 0;
    while ((currPge <= tot)){
    // for (int i = 0; i < MAX_UPAGE_INFO; i++) {
      if (myproc()->vld_pge[i] == 1) {
        pdinfo->va[currPge] = myproc()->va[i];
        pdinfo->pa[currPge] = myproc()->pa[i];
        currPge++;
        }
        i++;
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
    int currMap = 0;
    int i = 0;

    while ((currMap <= tot)) {
    // for (int i = 0; i < MAX_WMMAP_INFO; i++) {
      if(myproc()->vld_map[i] == 1) {
        wminfo->addr[currMap] = myproc()->addr[i];
        wminfo->length[currMap] = myproc()->length[i];
        wminfo->n_loaded_pages[currMap] = myproc()->n_loaded_pages[i];
        currMap++;
      }
      i++;
    }
    return 0;
}



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