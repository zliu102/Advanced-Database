Group 5

Task Assignment:
  Deman Yang(A20395988): initStorageManager,createPageFile,openPageFile, Documentation.

  Jiao Qu(A20386614): appendEmptyBlock, ensureCapacity, Testing, makefile.
  
  Yifan Xu(A20385903): writeBlock(writeCurrentBlock), closePageFile, destroyPageFile.

  Ziyu Liu(A20377366): readBlock(readFirstBlock, readLastBlock, readPreviousBlock, readCurrentBlock, readNextBlock), getBlockPos.
  

Test steps(in Linux system, suppose gcc is installed)
  1. Download all the source code in the same folder.
  2. Open terminal, use command "cd" to access this folder.
  3. Input command "make", press Enter.
  4. Input command "./test_assign1", press Enter.


Functionalities of Storage Manager contains 3 parts: Manipulating page files; Reading blocks from disc; Writing blocks to a page file.
  Manipulating page files 
    initStorageManager: Initialize page handler and file handler
    createPageFile: Create a new page file. The initial file size should be one page. This method should fill this single page with'\0' bytes.
    openPageFile:Opens an existing page file. Should return RC_FILE_NOT_FOUND if the file does not exist.
		closePageFile: Close an open page file.
    destroyPageFile: Delete an open page file.
    
  Reading blocks from disc
    readBlock:Reads the pageNumth block from a file and stores its content in the memory pointed to by the memPage page handle. If the file has less than pageNum pages, the method should return     
    					RC_READ_NON_EXISTING_PAGE.
    getBlockPos: Return the current page position in a file.
    readFirstBlock: Read the first page in a file.
    readPreviousBlock: Read previous page in a file.
    readCurrentBlock: Read current page in a file.
    readNextBlock: Read next page in a file.
    readLastBlock: Read the last page in a file.
    
  Writing blocks to a page file
    writeBlock: Write a page to disk using an absolute position.
    writeCurrentBlock: Write a page to disk using the current position.
    appendEmptyBlock: Increase the number of pages in the file by one. The new last page should be filled with zero bytes.
    ensureCapacity: If the file has less than numberOfPages pages then increase the size to numberOfPages.
    
