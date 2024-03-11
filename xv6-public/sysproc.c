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


//////////// HELLO//
// int check_overlap(int addr, int length) {
//     int currAddr;
//     int tempAddr;
//     int pages;

//     for (int i = 0; i < MAX_WMMAP_INFO; i++) {
//         if (myproc()->vld_map[i] == 1) {
//             currAddr = myproc()->addr[i];
//             // if (currAddr <= addr) {
//               // cprintf("comparing %x with %x\n", currAddr, addr);
//                 int mapLength = myproc()->length[i];
//                 pages = (mapLength % PGSIZE == 0) ? (mapLength / PGSIZE) : ((mapLength / PGSIZE) + 1);
//                 for (int j = 0; j < pages; j++) {
//                     tempAddr = currAddr + (PGSIZE * j);
//                     if (tempAddr == addr)
//                         return 1; // Overlapping
//                 }
//             // }
//         }
//     }
//     return 0; // Not overlapping
// }

// int map(int addr, int length, int flags, int fd) {
//     pte_t *pte;
//     int found = 0;
//     int count = 0;

//     int tot_maps = myproc()->total_mmaps;
//     int tot_upge = myproc()->n_upages;


//     int iterations = (length / 4096);
//     if (length % 4096 != 0) {
//       iterations++;
//     }
//     // UNFIXED mapping
//     if ((tot_maps > 0) && ((flags & MAP_FIXED) != MAP_FIXED)) {
//       while (found == 0) {
//           addr = 0x60000000 + (4096 * count);
//           // pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)addr), 1);
//           if (check_va((uint)addr)) {
//             count++;
//           }
//           else {
//             found = 1;
//           }
//         }
//         cprintf("curr addr: %x, count: %x length: %d, iterations: %d\n", addr, count, length, iterations);
//     }


//     int currAddr = addr;
//     // char *mem;
    
//     for (int i = 0; i < iterations; i++) {
//       if (check_va((uint) currAddr)) return FAILED;
//       pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
//       if (*pte & PTE_P) return FAILED;

//       // mem = kalloc();
//       // mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
//       myproc()->va[tot_upge + i] = currAddr;
//       // myproc()->pa[tot_upge + i] = V2P(mem);
//       myproc()->vld_pge[tot_upge + i] = 1;


//       // if ((flags & MAP_ANONYMOUS) == 0) { 
//       //   struct file *f = myproc()->ofile[fd];
//       //    fileread(f, (void*)currAddr, 4096);
//       // }   

//       currAddr = currAddr + 4096;
//     }



//     myproc()->total_mmaps++;
//     int i = 0;
//     while (myproc()->vld_map[i] == 1) {
//       i++;
//     }
//     myproc()->addr[i] = addr;
//     myproc()->vld_map[i] = 1;
//     myproc()->length[i] = length;
//     myproc()->n_loaded_pages[i] = iterations;
//     myproc()->n_upages = myproc()->n_upages + iterations;

//     // Return the mapped address

//     return addr;
// }



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

int unmap (int addr) {
  
    if ((addr % 4096 != 0) || (addr < 0x60000000))
      return FAILED; 



    myproc()->total_mmaps--;


    int currAddr = addr;
    cprintf("this is what was passed: %x\n", addr);
    int found = 0;
    int i = 0;

    int pages = 0;
    while ((found == 0) && (i < MAX_WMMAP_INFO)) {
      if ((myproc()->vld_map[i] == 1) && myproc()->addr[i] == addr) {
        found = 1;
        myproc()->vld_map[i] = 0;
        int length = myproc()->length[i];
        pages = (length % 4096 != 0) ? (length / 4096 + 1) : (length / 4096);
      }
      i++;
    }
    found = 0;
    i = 0;

    while ((found == 0) && (i < MAX_UPAGE_INFO)) {
      cprintf("curr i: %d VA: %x\n", i, (myproc()->va[i]));
      if ((myproc()->vld_pge[i] == 1) && myproc()->va[i] == addr) {
        found = 1;
        currAddr = myproc()->va[i];

        for (int j = 0; j < pages; j++) {
            cprintf("curr VA: %x\n", currAddr);
            myproc()->vld_pge[i] = 0;
            myproc()->n_upages--;
            pte_t *pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 0);
            uint pa = PTE_ADDR(*pte);
            *pte = 0;
            kfree(P2V(pa));

          currAddr = currAddr + 4096;

        }

      }
      i++;
    }


    return SUCCESS;
}


int
sys_wunmap(void)
{
    int addr;

    if (argint(0, &addr) < 0)
      return FAILED;


    return unmap(addr);

}



/*
int map(int addr, int length, int flags, int fd) {
    pte_t *pte;
    int found = 0;
    int count = 0;

    int tot_maps = myproc()->total_mmaps;
    int tot_upge = myproc()->n_upages;

    int iterations = (length / 4096);
    if (length % 4096 != 0) {
      iterations++;
    }
    int exists = 0;
    // UNFIXED mapping
    if ((tot_maps > 0) && ((flags & MAP_FIXED) != MAP_FIXED)) {
      while (found == 0){
          exists = 0;
          addr = 0x60000000 + (4096 * count);
          // pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)addr), 1);
          // if (*pte & PTE_P) count++;
          // for (int i = 0; i < iterations; i++) {
            for (int k = 0; k < MAX_UPAGE_INFO; k++) {
              if ((myproc()->va[k] == addr)) {
                exists = 1;
              }
            }
            if (exists) count++;
            else found = 1;

        }
    }


    int currAddr = addr;
    // char *mem;

    // int tot = myproc()->n_upages;
    // int currPge = 0;
    // while ((currPge <= tot)){

  
    for (int i = 0; i < iterations; i++) {
      for (int k = 0; k < MAX_UPAGE_INFO; k++) {
        if ((myproc()->vld_pge[k] == 1) && (myproc()->va[k] == currAddr)) {
            return FAILED;
          }
      } 
      pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
      if (*pte & PTE_P) return FAILED;

      // mem = kalloc();
      // mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
      myproc()->va[tot_upge + i] = currAddr;
      // myproc()->pa[tot_upge + i] = V2P(mem);
      myproc()->vld_pge[tot_upge + i] = 1;


      // if ((flags & MAP_ANONYMOUS) == 0) { 
      //   // Read the file into memory
      //   struct file *f = myproc()->ofile[fd];
      //    fileread(f, (void*)currAddr, 4096);
      //   // if (bytes_read != length) {
      //   //     kfree(mem);
      //   //     return bytes_read; // Failed to read the file into memory
      //   // }
      // }   

      currAddr = currAddr + 4096;
    }



    int r = 0;
    while ((myproc()->vld_map[r] == 1) && (r < MAX_WMMAP_INFO)) {
      r++;
    }
    if (r == MAX_WMMAP_INFO) return FAILED;
    myproc()->total_mmaps++;
    myproc()->addr[r] = addr;
    myproc()->vld_map[r] = 1;
    myproc()->flags[r] = flags;
    myproc()->length[r] = length;
    myproc()->n_loaded_pages[r] = iterations;
    myproc()->n_upages = myproc()->n_upages + iterations;

    // Return the mapped address

    return addr;
}


*/
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