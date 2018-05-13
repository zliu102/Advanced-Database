Group 5

1. Task Assignment:

  Deman Yang(A20395988): pinPage, forceFlushPool, unpinPage, Testing.

  Jiao Qu(A20386614): initBufferPool, shutdownBufferPool, documentation.

  Yifan Xu(A20385903): forcePage, getFrameContents, getFixCounts,makefile.

  Ziyu Liu(A20377366): markDirty,getDirtyFlags,getNumReadIO, getNumWriteIO.
  

2. Test steps(in Linux system, suppose gcc is installed)

  1. Download all the source code in the same folder.
  2. Open terminal, use command "cd" to access this folder.
  3. Input command "make", press Enter.
  4. Input command "./test_assign2”, press Enter.
  5. After test, use clean to delete files except source code.
  

3. File list 

  - buffer_mgr.c
  - buffer_mgr.h
  - buffer_mgr_stat.c
  - buffer_mgr_stat.h
  - dberror.c
  - dberror.h
  - dt.h
  - Makefile
  - README
  - storage_mgr.c
  - storage_mgr.h
  - test_assign2_1.c
  - test_helper.h


4. Function descriptions: of all additional functions Functionalities of Buffer Manager contains 3 parts: 
	 Buffer Manager Interface Pool Handling, Buffer Manager Interface Access Pages, Statistics Interface.


Buffer Manager Interface Pool Handling:

  initBufferPool: creates a new buffer pool with parameters which is used to cache pages from the page file with name. Initially the pool is empty and 
                  the page file should already exist, i.e., this method should not generate a new page file. stratData can be used to pass parameters 
                  for the page replacement strategy. 
 
  shutdownBufferPool:destroy buffer pool and free up all resources associated with buffer pool. This method should free up all resources associated with buffer pool. 
										For example, it should free the memory allocated for page frames. If the buffer pool contains any dirty pages, then these pages should be written 										back to disk before destroying the pool. It is an error to shutdown a buffer pool that has pinned pages.
 	
 	pinPage:pins the page with page number pageNum. The buffer manager is responsible to set the pageNum field of the page handle passed to the method. Similarly, the 
 					data field should point to the page frame the page is stored in (the area in memory storing the content of the page).The replacement strategy included in 	
 					this function, such as LRU and FIFO.

  forceFlushPool: cause all dirty pages with fix count 0 from the buffer pool to be written to disk.

Buffer Manager Interface Access Pages:

  markDirty: mark a page as dirty.
  
  unpinPage: unpin the page page. The pageNum field of page should be used to figure out which page to unpin.
  
  forcePage: write the current content of the page back to the page file on disk.

Statistics Interface:

  getFrameContents: returns an array of PageNumbers (of size numPages) where the ith element is the number of the page stored in the ith page frame. An empty page 
  									frame is represented using the constant NO_PAGE.

  getDirtyFlags: returns an array of bools (of size numPages) where the ith element is TRUE if the page stored in the ith page frame is dirty. Empty page frames are 
  							considered as clean.
  
  getFixCounts:returns an array of ints (of size numPages) where the ith element is the fix count of the page stored in the ith page frame. Return 0 for empty page 
  						 frames.
  						 
  getNumReadIO:returns the number of pages that have been read from disk since a buffer pool has been initialized. You code is responsible to initializing this 
  						 statistic at pool creating time and update whenever a page is read from the page file into a page frame.
 
	getNumWriteIO:returns the number of pages written to the page file since the buffer pool has been initialized.
	
	
5. Data structure Enhancement: main data structure used

typedef struct BM_Frame {
	BM_PageHandle *ph; 
	bool empty;      
	bool dirty;      
	int fixCount;    
	double lastHitTime; 
	double enterTime;   
} BM_Frame;


typedef struct BM_mgmtData{
	BM_Frame *framePool;
	SM_FileHandle *fHandle;
	int writeIO;
	int readIO;
	int bufferCapacity; 
	double count;
} BM_mgmtData;


typedef struct BM_BufferPool {
	char *pageFile;
	int numPages;
	ReplacementStrategy strategy;
	BM_mgmtData *mgmtData; 	// Updated
} BM_BufferPool;




