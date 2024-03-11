#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
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
    int offset = currAddr - myproc()->addr[index];
    int fd = myproc()->fd[index];
    int flags = myproc()->flags[index];
    // int length = myproc()->length[index];
    // int pages = (length % 4096 == 0) ? (length / 4096) : ((length / 4096) + 1);
    int upage = 0;
    while ((myproc()->vld_pge[upage] == 1) && (upage < MAX_UPAGE_INFO)) {
      upage++;
    }

    // for (int i = 0; i < pages; i++) {
      pte = walkpgdir(myproc()->pgdir, (char*)PGROUNDDOWN((uint)currAddr), 1);
      if (*pte & PTE_P) return FAILED;
      // if (check_overlap(currAddr, length)) return FAILED;

      mem = kalloc();
      memset(mem, 0, PGSIZE);
      mappages(myproc()->pgdir, (void*)currAddr, 4096, V2P(mem), PTE_W | PTE_U);
      if(upage != MAX_UPAGE_INFO) {
      myproc()->va[upage] = currAddr;
      myproc()->pa[upage] = V2P(mem);
      myproc()->vld_pge[upage] = 1;
      }
      

      if ((flags & MAP_ANONYMOUS) == 0) { 
        // cprintf("curr offset %d\n",offset);
        // Read the file into memory
        struct file *f = myproc()->ofile[fd];
        changeOffset(f, offset );
        fileread(f, (void*)currAddr, 4096);
      } 
      currAddr = currAddr + 4096;
      myproc()->n_loaded_pages[index]++;
    // }

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
    
    // myproc()->n_upages = myproc()->n_upages + iterations;

    // Return the mapped address

    return currAddr;
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

      if ((myproc()->vld_pge[i] == 1) && myproc()->va[i] == addr) {
        found = 1;
        currAddr = myproc()->va[i];

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
            kfree(P2V(pa));

          currAddr = currAddr + 4096;
        }
      }
      i++;
    }


    return SUCCESS;
}
