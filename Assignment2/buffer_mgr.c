/************************************************************
* File name: buffer_mgr.c
* Author: Group 5
*         Deman Yang(A20395988), Yifan Xu(A20385903)
*         Jiao Qu(A20386614), Ziyu Liu(A20377366)
* Version: 1.0
* Date: 15/03/2018
* Description: Buffer manager
 ************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"

SM_FileHandle fHandle;
//**********************************Buffer Manager Interface Pool Handling**********************************

/************************************************************
 * creates a new buffer pool with parameters which is used to
 * cache pages from the page file with name. Initially the
 * pool is empty and the page file should already exist, i.e.,
 * this method should not generate a new page file. stratData
 * can be used to pass parameters for the page replacement strategy.
 ************************************************************/
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
		const int numPages, ReplacementStrategy strategy,void *stratData){

    if(pageFileName == NULL){
    	THROW(RC_NO_FILENAME, "File not found");
    };

    //check if pageFile exists already
    if(openPageFile((char*)pageFileName, &fHandle) != RC_OK) {
    	THROW(RC_FILE_NOT_FOUND, "File not found");
    }

    // initialize the buffer pool
    bm->pageFile = (char*)pageFileName;
    bm->numPages = numPages;
    bm->strategy = strategy;

    BM_mgmtData *mgmtData = calloc(1, sizeof(BM_mgmtData));
    mgmtData->fHandle = &fHandle;
    mgmtData->readIO = 0;
    mgmtData->writeIO = 0;
    mgmtData->bufferCapacity = 0;
    mgmtData->count = 0;
    mgmtData->framePool = calloc(numPages, sizeof(BM_Frame));

    for(int i = 0; i < numPages ; i++){
        mgmtData->framePool[i].ph = calloc(1, sizeof(BM_PageHandle));
        mgmtData->framePool[i].ph->pageNum = -1;
        mgmtData->framePool[i].ph->data = calloc(PAGE_SIZE, sizeof(char));
        mgmtData->framePool[i].empty = 1;
        mgmtData->framePool[i].dirty = 0;
        mgmtData->framePool[i].fixCount = 0;
        mgmtData->framePool[i].lastHitTime = 0;
        mgmtData->framePool[i].enterTime = 0;
    }

    bm->mgmtData = mgmtData;

    return RC_OK;
}

/************************************************************
 * pins the page with page number pageNum. The buffer manager
 * is responsible to set the pageNum field of the page handle
 * passed to the method. Similarly, the data field should point
 * to the page frame the page is stored in (the area in memory
 * storing the content of the page).
 ************************************************************/
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
            const PageNumber pageNum){
	if(bm->pageFile == NULL) {
		THROW(RC_NO_FILENAME, "File not found");
	}

	SM_FileHandle fHandle;

	if (openPageFile((char *)bm->pageFile, &fHandle) != RC_OK) {
		THROW(RC_NO_FILENAME, "File open failed");
	}

	if(ensureCapacity(pageNum, &fHandle) != RC_OK) {
		THROW(RC_CAPACITY_FAILED, "Capacity not enough");
	}

	int emptyFrame = -1; // First empty frame number
	int FIFOFrame = -1;
	int LRUFrame = -1;
	bool updateFlag = 0;
	double tempHitTime = INFINITY;
	double tempEnterTime = INFINITY;
    for(int i=0;i<bm->numPages;i++){
    	// Update existing page
       	if (bm->mgmtData->framePool[i].ph != NULL &&
			bm->mgmtData->framePool[i].ph->pageNum == pageNum){ // Update existing
       		readBlock(pageNum,&fHandle, bm->mgmtData->framePool[i].ph->data);
       		page->pageNum = pageNum;
       		page->data = bm->mgmtData->framePool[i].ph->data;
			bm->mgmtData->framePool[i].fixCount++;

       		bm->mgmtData->framePool[i].lastHitTime = ++bm->mgmtData->count;
			updateFlag = 1;
       		break;
       	}

    	if(emptyFrame == -1 && bm->mgmtData->framePool[i].empty == 1){ // Record first empty frame number
    		emptyFrame = i;
    	}

    	if(bm->mgmtData->framePool[i].fixCount == 0){
			if (tempHitTime > bm->mgmtData->framePool[i].lastHitTime){
				tempHitTime = bm->mgmtData->framePool[i].lastHitTime;
				LRUFrame = i;
			}

			if(tempEnterTime > bm->mgmtData->framePool[i].enterTime){
				tempEnterTime = bm->mgmtData->framePool[i].enterTime;
				FIFOFrame = i;
			}
    	}
    }

    int targetFrame = -1; // Target Frame
    if (updateFlag == 0){
    	if( bm->mgmtData->bufferCapacity == bm->numPages){// Buffer full, replacement
            switch(bm->strategy){
            case RS_FIFO:
            	targetFrame = FIFOFrame;
            	break;
            case RS_LRU:
            	targetFrame = LRUFrame;
            	break;
            case RS_LRU_K:
            	//TBD
            	break;
            case RS_CLOCK:
            	//TBD
            	break;
            default:
            	break;
            }
    	}else{// Update to empty page
    		targetFrame = emptyFrame;
    	}

		if( emptyFrame != -1){ // Buffer is not full, need to increase
			bm->mgmtData->bufferCapacity++;
		}else{ // Buffer is full, need write frame back to page
			if (bm->mgmtData->framePool[targetFrame].dirty == 1){
				RC ret = forcePage(bm, bm->mgmtData->framePool[targetFrame].ph);
				if(ret != RC_OK){
					THROW(RC_WRITE_FAILED, "File write failed");
				}
			}
		}

		bm->mgmtData->framePool[targetFrame].ph->pageNum = pageNum;

		readBlock(pageNum, &fHandle, bm->mgmtData->framePool[targetFrame].ph->data);
//		printf("%d -> %s, frame: %d\n", pageNum, bm->mgmtData->framePool[targetFrame].ph->data, targetFrame); //TEST

		bm->mgmtData->framePool[targetFrame].empty = 0;
		bm->mgmtData->framePool[targetFrame].dirty = 0;
		bm->mgmtData->framePool[targetFrame].fixCount++;

		bm->mgmtData->count++;
		bm->mgmtData->framePool[targetFrame].lastHitTime = bm->mgmtData->count;
		bm->mgmtData->framePool[targetFrame].enterTime = bm->mgmtData->count;
		bm->mgmtData->readIO++;

		page->pageNum = pageNum;
		page->data = bm->mgmtData->framePool[targetFrame].ph->data;
    }
    return RC_OK;
}

/************************************************************
 * destroy buffer pool and free up all resources associated
 * with buffer pool. This method should free up all resources
 * associated with buffer pool. For example, it should free
 * the memory allocated for page frames. If the buffer pool
 * contains any dirty pages, then these pages should be written
 * back to disk before destroying the pool. It is an error to
 * shutdown a buffer pool that has pinned pages.
 ************************************************************/
RC shutdownBufferPool(BM_BufferPool *const bm){

    if(bm->pageFile == NULL) {
    	THROW(RC_NO_FILENAME, "File not found");
    }

    // make sure all the pages are written into disk
    if(forceFlushPool(bm) != RC_OK) {
    	THROW(RC_FLUSHPOOL_FAILED, "File flush failed");
    }

    free(bm->mgmtData->fHandle->mgmtInfo);
    bm->mgmtData->fHandle->mgmtInfo = NULL;
    for(int i = 0; i < bm->numPages; i++){
        free(bm->mgmtData->framePool[i].ph->data);
        bm->mgmtData->framePool[i].ph->data = NULL;
        free(bm->mgmtData->framePool[i].ph);
        bm->mgmtData->framePool[i].ph = NULL;
    }
    free(bm->mgmtData->framePool);
    bm->mgmtData->framePool = NULL;
    free(bm->mgmtData);
    bm->mgmtData = NULL;
    return RC_OK;
}

/************************************************************
 * cause all dirty pages with fix count 0 from the buffer pool
 * to be written to disk
 ************************************************************/
RC forceFlushPool(BM_BufferPool *const bm){
    if(bm->pageFile == NULL) {
    	THROW(RC_NO_FILENAME, "File not found");
    }

    //force every dirty page with fix count 0 to write into disk
    for(int i=0;i<bm->numPages;i++){
       	if (bm->mgmtData->framePool[i].dirty == 1 && bm->mgmtData->framePool[i].fixCount == 0){
       		RC ret = forcePage(bm, bm->mgmtData->framePool[i].ph);
       		if( ret != RC_OK ){
    			THROW(RC_FLUSHPOOL_FAILED, "File flush failed");
    		}
    	}
    }

    return RC_OK;
}

//**********************************Buffer Manager Interface Access Pages**********************************
/************************************************************
 * marks a page as dirty
 ************************************************************/
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
    if(bm->pageFile == NULL) {
    	THROW(RC_NO_FILENAME, "File not found");
    }
    // mark frame dirty
    for(int i=0;i<bm->numPages;i++){
       	if (bm->mgmtData->framePool[i].ph->pageNum == page->pageNum){
			bm->mgmtData->framePool[i].dirty = 1;
			break;
    	}
    }
    return RC_OK;
}

/************************************************************
 * Unpin the page page. The pageNum field of page should be
 * used to figure out which page to unpin.
 ************************************************************/
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    if(bm->pageFile == NULL) {
    	THROW(RC_NO_FILENAME, "File not found");
    }

    for(int i=0;i<bm->numPages;i++){
       	if (bm->mgmtData->framePool[i].ph->pageNum == page->pageNum){
       		if( bm->mgmtData->framePool[i].fixCount <= 0 ){
       			THROW(RC_UNPIN_FAILED, "Frame unpin failed");
       		}else{
       			bm->mgmtData->framePool[i].fixCount--;
       		}
    		break;
    	}
    }
    return RC_OK;
}

/************************************************************
 * write the current content of the page back to the page
 * file on disk.
 ************************************************************/
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    if(bm->pageFile == NULL) {
    	THROW(RC_NO_FILENAME, "File not found");
    }

    for(int i=0;i<bm->numPages;i++){
       	if (bm->mgmtData->framePool[i].ph->pageNum == page->pageNum){
       		RC ret = writeBlock(bm->mgmtData->framePool[i].ph->pageNum, bm->mgmtData->fHandle, bm->mgmtData->framePool[i].ph->data);
    		if( ret == RC_OK ){
    			bm->mgmtData->framePool[i].dirty = 0;
    			bm->mgmtData->writeIO++;
    		}else{
    			THROW(RC_WRITE_FAILED, "File write failed");
    		}
    		break;
    	}
    }

    return RC_OK;
}

//**********************************Statistics Interface**********************************

/************************************************************
 * returns an array of PageNumbers (of size numPages) where
 * the ith element is the number of the page stored in the
 * ith page frame. An empty page frame is represented using
 * the constant NO_PAGE.
 ************************************************************/
PageNumber *getFrameContents (BM_BufferPool *const bm){
    PageNumber *result = calloc(bm->numPages, sizeof(int));

    for(int i=0;i<bm->numPages;i++){
       	if (bm->mgmtData->framePool[i].empty == 1){
       		result[i] = NO_PAGE;
    	}else{
    		result[i] = bm->mgmtData->framePool[i].ph->pageNum;
    	}
    }
    return result;
}

/************************************************************
 * returns an array of bools (of size numPages) where the ith
 * element is TRUE if the page stored in the ith page frame is
 * dirty. Empty page frames are considered as clean.
 ************************************************************/
bool *getDirtyFlags (BM_BufferPool *const bm){
    bool *result = calloc(bm->numPages, sizeof(int));

    for(int i=0;i<bm->numPages;i++){
       	if (bm->mgmtData->framePool[i].dirty == 1){
       		result[i] = 1;
    	}else{
    		result[i] = 0;
    	}
    }
    return result;
}

/************************************************************
 * returns an array of ints (of size numPages) where the ith
 * element is the fix count of the page stored in the ith page
 * frame. Return 0 for empty page frames.
 ************************************************************/
int *getFixCounts (BM_BufferPool *const bm){
    int *result = calloc(bm->numPages, sizeof(int));

    for(int i=0;i<bm->numPages;i++){
       	if (bm->mgmtData->framePool[i].empty == 1){
       		result[i] = 0;
    	}else{
    		result[i] = bm->mgmtData->framePool[i].fixCount;
    	}
    }
    return result;
}

/************************************************************
 * returns the number of pages that have been read from disk
 * since a buffer pool has been initialized. You code is
 * responsible to initializing this statistic at pool creating
 * time and update whenever a page is read from the page file
 * into a page frame.
 ************************************************************/
int getNumReadIO (BM_BufferPool *const bm){
    return bm->mgmtData->readIO;
}

/************************************************************
 * returns the number of pages written to the page file since
 * the buffer pool has been initialized.
 ************************************************************/
int getNumWriteIO (BM_BufferPool *const bm){
	return bm->mgmtData->writeIO;
}
