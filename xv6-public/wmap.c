#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "wmap.h"


int check_overlap(int addr, int length) {
    int currAddr;
    int tempAddr;
    int pages;

    for (int i = 0; i < MAX_WMMAP_INFO; i++) {
        if (myproc()->vld_map[i] == 1) {
            currAddr = myproc()->addr[i];
            // if (currAddr <= addr) {
              // cprintf("comparing %x with %x\n", currAddr, addr);
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
    int overlap = 0;
    while (count < 131072) {
        // if (count == 15) temp++;
        // int lower_bound = 0x60000000 + (PGSIZE * count);

        int upper_bound = lower_bound + (PGSIZE * new_PageNum);

        while (temp < MAX_WMMAP_INFO) {
          if (myproc()->vld_map[temp] == 1) {

              // Check if there's an existing mapping
              int temp_length = myproc()->length[temp];
              int temp_PageNum = (temp_length % PGSIZE == 0) ? (temp_length / PGSIZE) : ((temp_length / PGSIZE) + 1);
              int temp_lower_bound = myproc()->addr[temp];
              int temp_upper_bound = temp_lower_bound + (PGSIZE * temp_PageNum);
          //     cprintf("lower bound: %x, upper bound: %x, length:%d\n", lower_bound, upper_bound, length);
          // cprintf("comp with: lower bound: %x, upper bound: %x, length:%d\n", temp_lower_bound, temp_upper_bound, temp_length);
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
    // cprintf("Addr: %x, i here %d\n",  currAddr, index);
    if (index == FAILED) return FAILED;
    // currAddr = myproc()->addr[index];
    int fd = myproc()->fd[index];
    int flags = myproc()->flags[index];
    int length = myproc()->length[index];
    int pages = (length % 4096 == 0) ? (length / 4096) : ((length / 4096) + 1);
    int upage = 0;
    while (myproc()->vld_pge[upage] == 1) {
      upage++;
    }
    // cprintf("found length here %d, num pages is %d, user index is %d\n", length, pages, upage);
    // cprintf("found flags are %x, fd is %d\n", flags , fd);

    for (int i = 0; i < pages; i++) {
      pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
      if (*pte & PTE_P) return FAILED;
      // if (check_overlap(currAddr, length)) return FAILED;

      mem = kalloc();
      memset(mem, 0, PGSIZE);
      mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
      myproc()->va[upage+i] = currAddr;
      myproc()->pa[upage+i] = V2P(mem);
      myproc()->vld_pge[upage+i] = 1;
      

      if ((flags & MAP_ANONYMOUS) == 0) { 
        // Read the file into memory
        struct file *f = myproc()->ofile[fd];
        fileread(f, (void*)currAddr, 4096);
      } 
      currAddr = currAddr + 4096;
      myproc()->n_loaded_pages[index]++;
    }

    

  return SUCCESS;
}

int map(int addr, int length, int flags, int fd) {
    // pte_t *pte;
    // int found = 0;
    // int count = 0;

    int tot_maps = myproc()->total_mmaps;
    // int tot_upge = myproc()->n_upages;

    int iterations = (length / 4096);
    if (length % 4096 != 0) {
      iterations++;
    }
    int currAddr = addr;
    int temp_length = length;
    cprintf("fd in waaas: %x\n", fd);

    // UNFIXED mapping
    if ((tot_maps > 0) && ((flags & MAP_FIXED) != MAP_FIXED)) {
      // while (found == 0){
      //     if (check_overlap(0x60000000) == 1) count++;
      //     addr = 0x60000000 + (4096 * count);
      //     pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)addr), 1);
      //     if (*pte & PTE_P) count++;
      //     else found = 1;
      //   }
      cprintf("how about here\n");
      currAddr = find_addr(temp_length);
      // cprintf("here addr is:%x and then we found: currAddr %x \n", addr, currAddr);
    } else {
      if (check_overlap(addr, temp_length) == 1) return FAILED;
    }


    // int ret;



    // ret = fill_table(currAddr, iterations, length, flags, fd, tot_upge);
    // if (ret == -1) return FAILED;



    myproc()->total_mmaps++;
    int i = 0;
    while (myproc()->vld_map[i] == 1) {
      i++;
    }
    // cprintf("here addr is: %x and curr is: %x\n", addr, currAddr);
    myproc()->addr[i] = currAddr;
    myproc()->flags[i] = flags;
    myproc()->fd[i] = fd;
    myproc()->vld_map[i] = 1;
    myproc()->length[i] = length;
    cprintf("and we loaded this %x with length: %d\n", myproc()->addr[i], myproc()->length[i]);
    
    // myproc()->n_upages = myproc()->n_upages + iterations;

    // Return the mapped address

    return currAddr;
}
