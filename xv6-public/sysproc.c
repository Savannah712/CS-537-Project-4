#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
// #include "uaccess.h"
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
        
    return map(addr, length, flags, fd);

  }


int
sys_wunmap(void)
{
    int addr;

    if (argint(0, &addr) < 0)
      return FAILED;


    return unmap(addr);

}




int remap_inplace (int addr, int oldsize, int length) {
    // pte_t *pte;
    // int found = 0;
    // int count = 0;

    // int tot_maps = myproc()->total_mmaps;
    // int tot_upge = myproc()->n_upages;

    // int new_iterations = (length / 4096);
    // if (length % 4096 != 0) {
    //   new_iterations++;
    // }
    // int old_iterations = (oldsize / 4096);
    // if (oldsize % 4096 != 0) {
    //   old_iterations++;
    // }

    // int currAddr = addr;
    // char *mem;

    // // GROW
    // if (length > oldsize) {
    //   for (int i = 0; i < new_iterations; i++) {
    //     currAddr = currAddr + (4096 * i + old_iterations);
    //     pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
    //     if (*pte & PTE_P) return FAILED;
    //     mem = kalloc();
    //     mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
    //     myproc()->va[tot_upge + i] = currAddr;
    //     myproc()->pa[tot_upge + i] = V2P(mem);
    //     myproc()->vld_pge[tot_upge + i] = 1;
        

    //   }



    // } else { // SHRINK

    // }
    
    
    // for (int i = 0; i < iterations; i++) {
    //   pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
    //   if (*pte & PTE_P) return FAILED;

    //   mem = kalloc();
    //   mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
    //   myproc()->va[tot_upge + i] = currAddr;
    //   myproc()->pa[tot_upge + i] = V2P(mem);
    //   myproc()->vld_pge[tot_upge + i] = 1;


    //   if ((flags & MAP_ANONYMOUS) == 0) { 
    //     // Read the file into memory
    //     struct file *f = myproc()->ofile[fd];
    //      fileread(f, (void*)currAddr, 4096);
    //     // if (bytes_read != length) {
    //     //     kfree(mem);
    //     //     return bytes_read; // Failed to read the file into memory
    //     // }
    //   }   

    //   currAddr = currAddr + 4096;


    // }



    // myproc()->total_mmaps++;
    // int i = 0;
    // while (myproc()->vld_map[i] == 1) {
    //   i++;
    // }
    // myproc()->addr[i] = addr;
    // myproc()->vld_map[i] = 1;
    // myproc()->length[i] = length;
    // myproc()->n_loaded_pages[i] = iterations;
    // myproc()->n_upages = myproc()->n_upages + iterations;

    // // Return the mapped address

    // return addr;
return 0;

}

int
sys_wremap(void)
{
  return 0;
    // int oldaddr;
    // int oldsize;
    // int newsize;
    // int flags;


    // if (argint(0, &oldaddr) < 0 || argint(1, &oldsize) < 0 || argint(2, &newsize) < 0 || argint(3, &flags) < 0)
    //   return FAILED;
    
    // // FAIL if is not page aligned 
    // if (((oldaddr % 4096) != 0) && (oldaddr < 0x60000000)) 
    //   return FAILED;
    
    // // FAIL if is not a starting address that we have & same length

    // int found = 0;
    // int i = 0;
    // // int length = 0;

    // while ((found == 0) && (i < MAX_WMMAP_INFO)) {
    //   if ((myproc()->vld_map[i] == 1) && (myproc()->addr[i] == oldaddr)) {
    //     found = 1;
    //     if (myproc()->length[i] != oldsize) {
    //       return FAILED;
    //     }
    //   } else {
    //     i++;
    //   }
    // }

    // // if (flags == MREMAP_MAYMOVE) {

    // // }

      

    // // map(oldaddr, length, flags, fd);
    // // Your implementation here
    // return oldaddr;

    
    // if (argint(0, &oldaddr) < 0 || argint(1, &oldsize) < 0 ||
    //     argint(2, &newsize) < 0 || argint(3, &flags) < 0)
    //     return FAILED;

    // // Check if oldaddr is page-aligned
    // if (oldaddr % PGSIZE != 0)
    //     return FAILED;

    // // Find the existing mapping based on oldaddr and oldsize
    // int found = 0;
    // int i = 0;
    // while (!found && i < MAX_WMMAP_INFO) {
    //     if (myproc()->vld_map[i] && myproc()->addr[i] == oldaddr && myproc()->length[i] == oldsize)
    //         found = 1;
    //     else
    //         i++;
    // }
    // if (!found)
    //     return FAILED;

    // // If the MREMAP_MAYMOVE flag is set, try moving the mapping
    // if (flags & MREMAP_MAYMOVE) {
    //     // Your logic for moving the mapping to a new address goes here
    //     // Ensure that you update the process's page directory and manage
    //     // virtual-to-physical mappings accordingly

    //     // Placeholder logic
    //     // Remove the old mapping
    //     if (unmap(oldaddr) == FAILED)
    //         return FAILED;

    //     // Create a new mapping at a new address
    //     int newaddr = map(0, newsize, 0, -1);
    //     if (newaddr == FAILED)
    //         return FAILED;

    //     return newaddr;
    // } else {
    //     // Resize the mapping in-place
    //     myproc()->length[i] = newsize;

    //     // Update process's page directory and manage virtual-to-physical mappings accordingly
    //     // Not implemented in this example

    //     return oldaddr; // Return the old address as success
    // }
}

int
sys_getpgdirinfo(void)
{


    struct pgdirinfo *pdinfo;
    if (argptr(0, (void *)&pdinfo, sizeof(*pdinfo)) < 0)
    return -1;

    struct proc *p = myproc();
    pde_t *pgdir;
    pte_t *pte;
    uint n_upages = 0;
    int i = 0;

    // Initialize pdinfo to zeros to ensure all fields are properly set
    memset(pdinfo, 0, sizeof(struct pgdirinfo));

    pgdir = p->pgdir;

    for (uint va = 0; va < KERNBASE; va += PGSIZE) {
        pte = walkpgdir(pgdir, (void *)va, 0);
        if (pte && (*pte & PTE_P) && (*pte & PTE_U)) {
            if (i < MAX_UPAGE_INFO) {
                pdinfo->va[i] = va;
                pdinfo->pa[i] = PTE_ADDR(*pte);
                i++;
            }
            n_upages++;
        }
    }

    pdinfo->n_upages = n_upages;
    return 0;
}

int unmap_all() {
  pte_t *pte;
  pde_t *pgdir = myproc()->pgdir;
      for (uint va = 0; va < KERNBASE; va += PGSIZE) {
        pte = walkpgdir(pgdir, (void *)va, 0);
        if (pte && (*pte & PTE_P) && (*pte & PTE_U)) {
            // if (i < MAX_UPAGE_INFO) {
            //     i++;
            // }
            unmap(va);
        }
    }

    return 0;

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

    // while ((currMap <= tot)) {
    for (int i = 0; i < MAX_WMMAP_INFO; i++) {
      if(myproc()->vld_map[i] == 1) {
        wminfo->addr[currMap] = myproc()->addr[i];
        wminfo->length[currMap] = myproc()->length[i];
        wminfo->n_loaded_pages[currMap] = myproc()->n_loaded_pages[i];
        currMap++;
      }
    }
    return 0;
}

