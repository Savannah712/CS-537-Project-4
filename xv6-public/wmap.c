#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "wmap.h"

// #include "file.h"

int check_overlap(int addr, int length) {
    int currAddr;
    int tempAddr;
    int pages;

    for (int i = 0; i < MAX_WMMAP_INFO; i++) {
        if (myproc()->vld_map[i] == 1) {
            currAddr = myproc()->addr[i];
            // if (currAddr <= addr) {
                int mapLength = myproc()->length[i];
                pages = (mapLength % PGSIZE == 0) ? (mapLength / PGSIZE) : ((mapLength / PGSIZE) + 1);
                for (int j = 0; j < pages; j++) {
                    tempAddr = currAddr + (PGSIZE * j);
                    if (tempAddr == addr)
                        return 1; // Overlapping
                }
            // }
        }
    }
    return 0; // Not overlapping
}

int find_addr(int length) {
    int newAddr = -1; // Initialize newAddr to an invalid value
    int count = 0;
    int new_PageNum = (length % PGSIZE == 0) ? (length / PGSIZE) : ((length / PGSIZE) + 1);
    int temp = 0;
    int lower_bound = 0x60000000;
    int upper_bound = 0;
    int overlap = 0;
    while ((count < 131072) && (upper_bound <= KERNBASE)) {
        // if (count == 15) temp++;
        // int lower_bound = 0x60000000 + (PGSIZE * count);

        upper_bound = lower_bound + (PGSIZE * new_PageNum);
        // new_num_pages

        while (temp < MAX_WMMAP_INFO) {
          if (myproc()->vld_map[temp] == 1) {
              // Check if there's an existing mapping
              int temp_length = myproc()->length[temp];
              int temp_PageNum = (temp_length % PGSIZE == 0) ? (temp_length / PGSIZE) : ((temp_length / PGSIZE) + 1);
              int temp_lower_bound = myproc()->addr[temp];
              int temp_upper_bound = temp_lower_bound + (PGSIZE * temp_PageNum);
              if (new_PageNum >= temp_PageNum) {
                if (((temp_upper_bound > lower_bound) && (temp_upper_bound <= upper_bound)) ||
                    ((temp_lower_bound >= lower_bound) && (temp_lower_bound < upper_bound))) {
                      overlap = 1;
                      lower_bound = temp_upper_bound;
                      break;
                    }
              }
              else {
                if (((temp_upper_bound < lower_bound) && (temp_upper_bound >= upper_bound)) ||
                    ((temp_lower_bound <= lower_bound) && (temp_lower_bound > upper_bound))) {
                      overlap = 1;
                      lower_bound = temp_upper_bound;
                      break;
                    }
              }

          }
          temp++;
        }
        if (!overlap) {
          newAddr = lower_bound;
          break;
        }
        overlap = 0;
        count++;
        // if (i == 15) break;
    }

    return newAddr;
}


int find_index(int addr) {
  int i = 0;
  int length;
  int num_pages;
  int temp_addr;

  while (i < MAX_WMMAP_INFO) {
    if (myproc()->vld_map[i] == 1) {
        temp_addr = myproc()->addr[i];
        if (addr == temp_addr) return i;
        else if (temp_addr < addr) {
            length =  myproc()->length[i];
            num_pages = (length % 4096 == 0) ? (length / 4096) : ((length / 4096) + 1);
            int temp_upper_bound = temp_addr + (PGSIZE * num_pages);
            if (temp_upper_bound > addr) return i;
        }
    }
    i++;
  }


  return FAILED;
}

int fill_table(int currAddr) {
    pte_t *pte;
    char * mem;

    int index;
    index = find_index(currAddr);
    if (index == FAILED) return FAILED;
    int offset = currAddr - myproc()->addr[index];
    int fd = myproc()->fd[index];
    int flags = myproc()->flags[index];
    // int length = myproc()->length[index];
    // int pages = (length % 4096 == 0) ? (length / 4096) : ((length / 4096) + 1);
    int upage = 0;
    while ((myproc()->vld_pge[upage] == 1) && (upage < MAX_UPAGE_INFO)) {
      upage++;
    }

      pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
      if (*pte & PTE_P) return FAILED;


      mem = kalloc();
      memset(mem, 0, PGSIZE);
      mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
        // cprintf("remapped PTE: %x vs PTE %x\n", P2V(pa), va);
      // cprintf("PTE ADDR is= %d, PTE is %d\n", PTE_ADDR(*pte), *pte);

      if(upage != MAX_UPAGE_INFO) {
      myproc()->va[upage] = currAddr;
      myproc()->pa[upage] = V2P(mem);
      myproc()->vld_pge[upage] = 1;
      }
      

      if ((flags & MAP_ANONYMOUS) == 0) { 
        // Read the file into memory
        struct file *f = myproc()->ofile[fd];
        changeOffset(f, offset );
        fileread(f, (void*)currAddr, 4096);
      } 
      currAddr = currAddr + 4096;
      myproc()->n_loaded_pages[index]++;

  return SUCCESS;
}

int map(int addr, int length, int flags, int fd) {

    int tot_maps = myproc()->total_mmaps;

    int iterations = (length / 4096);
    if (length % 4096 != 0) {
      iterations++;
    }
    int currAddr = addr;
    int temp_length = length;

    // UNFIXED mapping
    if ((tot_maps > 0) && ((flags & MAP_FIXED) != MAP_FIXED)) {

      currAddr = find_addr(temp_length);
    } else {
      if (check_overlap(addr, temp_length) == 1) return FAILED;
    }
    myproc()->total_mmaps++;
    int i = 0;
    while (myproc()->vld_map[i] == 1) {
      i++;
    }
    myproc()->addr[i] = currAddr;
    myproc()->flags[i] = flags;
    myproc()->fd[i] = fd;
    myproc()->vld_map[i] = 1;
    myproc()->length[i] = length;

    return currAddr;
}

int shrink(int addr) {

    pte_t *pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)addr), 0);
    uint pa = PTE_ADDR(*pte);

    if ((*pte & PTE_P)) kfree(P2V(pa));
    *pte = 0;
    myproc()->n_loaded_pages[find_index(addr)]--;

    return SUCCESS;

}

void remap_virtual_address(uint va, uint new_va) {
  pde_t *pgdir = myproc()->pgdir;

  pte_t *pte = walkpgdir(pgdir, (void *)va, 0); // Don't allocate new pages if not found
  // uint pa = PTE_ADDR(*pte);
  if (pte == 0)
      panic("remap_virtual_address: no existing mapping");

    // cprintf("REMAP: PTE ADDR is= %d, PTE is %d\n", PTE_ADDR(*pte), *pte);
    // Map the new virtual address to the same physical address with the desired permissions
    mappages(pgdir, (void *)new_va, PGSIZE, PTE_ADDR(*pte), PTE_W | PTE_U);
    *pte = 0;


    // Invalidate TLB for old virtual address
    // tlbflush(); // You may need to flush TLB depending on your system
}

int is_allocated_virtual_address(uint va) {
    pde_t *pgdir = myproc()->pgdir;
    pte_t *pte = walkpgdir(pgdir, (void *)va, 0); // Don't allocate new pages if not found
    return (pte != 0);
}

int grow_unmap(int addr, int new_size, int index) {
  
  // int new_addr;
  // int old_length = myproc()->length[index];
  // int currAddr = find_addr(new_size);
  int new_addr = find_addr(new_size);
  int currVA = new_addr;
  // int oldVA;



    if ((addr % 4096 != 0) || (addr < 0x60000000))
      return FAILED; 

    int found = 0;
    // int i = 0;
    // int fd = 0;
    int length;
    // int flags = 0;
    // struct file *f;

    int pages = 0;
    
    length = myproc()->length[index];
    // flags = myproc()->flags[i];
    // fd = myproc()->fd[i];
    pages = (length % 4096 != 0) ? (length / 4096 + 1) : (length / 4096);
    found = 0;
    int i = 0;

    while ((found == 0) && (i < MAX_UPAGE_INFO)) {
      // if ((myproc()->vld_pge[i] == 1) && (myproc()->va[i] == addr)) {
      if (is_allocated_virtual_address(addr) != 0) {

        found = 1;
        int oldVA = addr;

        for (int j = 0; j < pages; j++) {
            // myproc()->vld_pge[i] = 0;
            // myproc()->n_upages--;
            // pte_t *pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 0);
            // uint pa = PTE_ADDR(*pte);
            // *pte = 0;
            // kfree(P2V(pa));
            remap_virtual_address(oldVA,currVA);

            currVA += PGSIZE;
            oldVA += PGSIZE;
        }
      }
      i++;
    }

    myproc()->length[index] = new_size;
    myproc()->addr[index] = new_addr;

    return new_addr;
}

int unmap (int addr) {
  
    if ((addr % 4096 != 0) || (addr < 0x60000000))
      return FAILED; 



    myproc()->total_mmaps--;

    int currAddr = addr;

    int found = 0;
    int i = 0;
    int fd = 0;
    int length;
    int flags = 0;
    struct file *f;

      int pages = 0;
    while ((found == 0) && (i < MAX_WMMAP_INFO)) {
      if ((myproc()->vld_map[i] == 1) && myproc()->addr[i] == addr) {
        // cprintf("here!!\n");
        found = 1;
        myproc()->vld_map[i] = 0;
        length = myproc()->length[i];
        flags = myproc()->flags[i];
        fd = myproc()->fd[i];
        pages = (length % 4096 != 0) ? (length / 4096 + 1) : (length / 4096);
      }
      i++;
    }
    found = 0;
    i = 0;

    while ((found == 0) && (i < MAX_UPAGE_INFO)) {

      if (is_allocated_virtual_address(addr) == 1) {
        found = 1;
        currAddr = addr;
              if ((flags & MAP_SHARED) != 0) {
                // Write the modified contents back to the file
                
                f = myproc()->ofile[fd];
                changeOffset(f, 0);
                
              }

        for (int j = 0; j < pages; j++) {


            if ((flags & MAP_SHARED) != 0) {
                // Write the modified contents back to the file
                // f = myproc()->ofile[fd];
                filewrite(f, (void*)currAddr, 4096);
                
              }

            myproc()->vld_pge[i] = 0;
            myproc()->n_upages--;
            pte_t *pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 0);
            uint pa = PTE_ADDR(*pte);
            *pte = 0;
            if ((*pte & PTE_P)) kfree(P2V(pa));

          currAddr = currAddr + 4096;
        }
      }
      i++;
    }



    return SUCCESS;
}


int unmap_all(void) {
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

int check_grow_in_place(int index, int newsize) {
  int addr = myproc()->addr[index];
  // int oldsize = myproc()->length[index];
  int new_PageNum = (newsize % PGSIZE == 0) ? (newsize / PGSIZE) : ((newsize / PGSIZE) + 1);
  int new_upperbound = addr + (PGSIZE * new_PageNum); 
  int temp_lower_bound;
  int temp_upper_bound;
  int temp_length;
  int temp_PageNum;
  int can_grow = 1;

  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    if((i != index) && (myproc()->vld_map[i] == 1)) {
      temp_length = myproc()->length[i];
      temp_PageNum = (temp_length % PGSIZE == 0) ? (temp_length / PGSIZE) : ((temp_length / PGSIZE) + 1);
      temp_lower_bound = myproc()->addr[i];
      temp_upper_bound = temp_lower_bound + (PGSIZE * temp_PageNum);


      if (((new_upperbound > temp_lower_bound) && (new_upperbound <= temp_upper_bound)) ||
          ((new_upperbound > temp_upper_bound) && (addr < temp_upper_bound))) return 0;
    }

  }

  return can_grow;
}


int remap(int oldaddr, int oldsize, int newsize, int flags) {

  if (((oldaddr % 4096) != 0) && (oldaddr < 0x60000000)) 
    return FAILED;
  
  int index = find_index(oldaddr);
  if (index == FAILED) return FAILED;
  int temp_addr = myproc()->addr[index];
  int temp_length = myproc()->length[index];
  if ((temp_addr != oldaddr) || (temp_length != oldsize)) return FAILED;



// REMAP IN PLACE!
  int old_num_pages = (oldsize % PGSIZE == 0) ? (oldsize / PGSIZE) : ((oldsize / PGSIZE) + 1);
  int new_num_pages = (newsize % PGSIZE == 0) ? (newsize / PGSIZE) : ((newsize / PGSIZE) + 1);

  // cprintf("old addr: %x, new addr %x\n", oldaddr, temp_addr);
  // cprintf("old size: %x, new size %x\n", oldsize, newsize);
  // cprintf("old page: %x, new page %x\n", old_num_pages, new_num_pages);


  // same: SAME NUMBER OF PAGES!
  if (new_num_pages == old_num_pages) {
    myproc()->length[index] = newsize;

    return oldaddr;
  }
  // shrink: LESS PAGES
  else if (new_num_pages < old_num_pages) {

    for (int i = new_num_pages; i < old_num_pages ; i++) {
      int rm_addr = oldaddr + (PGSIZE * i);

      shrink(rm_addr);
    }
    myproc()->length[index] = newsize;
    return oldaddr;
  } 
  // grow: MORE PAGES
  else {
    // CHECK IF IT CAN GROW IN PLACE
    if (check_grow_in_place(index, newsize) == 1){

        myproc()->length[index] = newsize;
    // CANT? CHECK IF IT CAN MOVE
    } else if (flags == MREMAP_MAYMOVE) {
      // unmap(oldaddr);
      // return map(oldaddr,newsize,(myproc()->flags[index] & ~MAP_FIXED), myproc()->fd[index]);
      return grow_unmap(oldaddr, newsize, index);
    } else return FAILED;
  }
  
  return oldaddr;


}


// #include "types.h"
// #include "defs.h"
// #include "param.h"
// #include "memlayout.h"
// #include "mmu.h"
// #include "proc.h"

// // Helper function to copy memory mappings from parent to child
// int copy_mmaps(struct proc *parent, struct proc *child) {
//     // Iterate over the memory mappings in the parent process
//     for (int i = 0; i < MAX_WMMAP_INFO; i++) {
//         // Check if the parent process has a memory mapping at this index
//         if (parent->addr[i] == 0)
//             continue; // No memory mapping at this index, skip to next
        
//         // Allocate new pages for the child process
//         char *new_mem = kalloc();
//         if (new_mem == 0)
//             return -1; // Failed to allocate memory
        
//         // Copy data from parent's memory mapping to the new pages
//         memmove(new_mem, (char *)parent->mmaps[i].addr, parent->mmaps[i].len);
        
//         // Map the new pages to the child process's address space
//         if (mappages(child->pgdir, (void *)parent->mmaps[i].addr, parent->mmaps[i].len, V2P(new_mem), PTE_W|PTE_U) < 0) {
//             kfree(new_mem);
//             return -1; // Failed to map pages to child process
//         }
//     }
    
//     return 0; // Success
// }
