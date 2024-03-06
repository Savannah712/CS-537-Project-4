# CS-537-Project-4

Need to Implement 5 calls 
- wmap
- wunmap
- getpgdirinfo (DEBUG)
- getwmapinfo (DEBUG)
- wremap

# Suggested Steps 
1. Make sure you understand how xv6 does memory management. Refer to the xv6 manual. The second chapter called 'page tables' gives a good insight of the memory layout in xv6. 
2. Try to implement a basic **wmap**. It should just check if that particular region asked by the user is available or not. If it is, you should map the pages in that range.
3. Implement **wunmap**. For now, just remove the mappings.
4. Implement **getwmapinfo** and **getpgdirinfo**. As mentioned earlier, most of the tests depend on these two syscalls to work.
5. Modify your **wmap** such that it's able to search for an available region in the process address space. This should make your wmap work without MAP_FIXED.
6. Support file-backed mapping. You'll need to change the wmap so that it's able to use the provided fd to find the file. You'll also need to revisit wunmap to write the changes made to the mapped memory back to disk when you're removing the mapping. You can assume that the offset is always 0.
7. Go for fork() and exit(). Copy mappings from parent to child across the fork() system call. Also, make sure you remove all mappings in exit().
8. Implement MAP_PRIVATE. You'll need to change 'fork()' to behave differently if the mapping is private. Also, you'll need to revisit 'wunmap' and make sure changes are NOT reflected in the underlying file with MAP_PRIVATE set.
9. Implement **wremap**.
