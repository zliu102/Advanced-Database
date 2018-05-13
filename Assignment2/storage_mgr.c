/************************************************************
* File name: storage_mgr.c
* Author: Group 5
*         Deman Yang(A20395988), Yifan Xu(A20385903)
*         Jiao Qu(A20386614), Ziyu Liu(A20377366)
* Version: 1.0
* Date: 02/03/2018
* Description: Storage manager
 ************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "storage_mgr.h"
#include "dberror.h"

/************************************************************
* Global variables
 ************************************************************/
FILE *pageFile = NULL;

/************************************************************
 * Initialization
 ************************************************************/
void initStorageManager(){

}

/************************************************************
* Create a new page file fileName. The initial file size should
* be one page. This method should fill this single page with
* '\0' bytes.
 ************************************************************/
RC createPageFile(char* fileName) {

    pageFile = fopen(fileName, "w+"); //Open file for read and write(overwrite existing one)
    if(pageFile == NULL) {
        THROW(RC_FILE_NOT_FOUND, "File not found");
    }

    char* emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    memset(emptyPage, '\0', PAGE_SIZE);   //fill the content with '\0'
    size_t w = fwrite(emptyPage, 1, PAGE_SIZE, pageFile);
    free(emptyPage);
    if( w != PAGE_SIZE){
    	THROW(RC_WRITE_FAILED, "File created failed");
    }else{
        fclose(pageFile); //Close file
        THROW(RC_OK, "File created successfully");
    }
}

/************************************************************
* Opens an existing page file.
* Should return RC_FILE_NOT_FOUND if the file does not exist.
* The second parameter is an existing file handle.
 ************************************************************/
RC openPageFile (char *fileName, SM_FileHandle *fHandle){

    pageFile = fopen(fileName, "r+"); //Open file for write and read
    if(pageFile == NULL) {
        THROW(RC_FILE_NOT_FOUND, "File not found");
    }

    fHandle->fileName = fileName;   // Record the filename
    if (fseek(pageFile, 0, SEEK_END) != 0){ //Move pointer to the end
        fclose(pageFile);
        THROW(RC_GET_PAGE_SIZE_FAILED, "Failed to get page size");
    };

    fHandle->totalNumPages = (int)(ftell(pageFile) / PAGE_SIZE); //Get total number of pages
    fseek(pageFile, 0, SEEK_SET); //Move pointer back to the beginning
    fHandle->curPagePos = 0;       // Set the current position to the beginning of file
    fHandle->mgmtInfo = pageFile;  // POSIX file descriptor
    THROW(RC_OK, "File opened successfully");
}

/************************************************************
* Close an open page file.
 ************************************************************/
RC closePageFile (SM_FileHandle *fHandle){
    if (fHandle->mgmtInfo == NULL){
        THROW(RC_FILE_HANDLE_NOT_INIT, "File is not open");
    } else{
    	fclose(fHandle->mgmtInfo); // Close file descriptor
    	THROW(RC_OK, "File closed successfully");
    }
}

/************************************************************
* Destroy(delete) an open page file.
 ************************************************************/
RC destroyPageFile (char *fileName){
    if (remove(fileName) != 0) {
        THROW(RC_DELETED_FAILED, "File deleted failed");
    }else{
    	THROW(RC_OK, "File deleted successfully");
    }
}

/************************************************************
* The method reads the pageNumth block from a file and stores
* its content in the memory pointed to by the memPage page
* handle. If the file has less than pageNum pages, the method
* should return RC_READ_NON_EXISTING_PAGE.
 ************************************************************/
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

    if ( fHandle->fileName == NULL) {
    	THROW(RC_FILE_NOT_FOUND, "File not found");
    }

    // If the file has less than pageNum pages or pageNum invalid
    if(pageNum > fHandle->totalNumPages || pageNum < 0){
        THROW(RC_READ_NON_EXISTING_PAGE, "Page number does not exist");
    }

    //fHandle->curPagePos = pageNum;  // Set current page position.
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE * sizeof(char), SEEK_SET); // Set the pointer to pageNum
    fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo); // Read the block to memPage
    THROW(RC_OK, "Read successfully");
}

/************************************************************
* Return the current page position in a file
 ************************************************************/
int getBlockPos (SM_FileHandle *fHandle){
    if ( fHandle->fileName == NULL) {
    	THROW(RC_FILE_NOT_FOUND, "File not found");
    }
    return fHandle->curPagePos;
}

/************************************************************
* Read the first page in a file
 ************************************************************/
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(0, fHandle, memPage);
}

/************************************************************
* Read the last page in a file
 ************************************************************/
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->totalNumPages-1, fHandle, memPage);
}

/************************************************************
* Read the previous page relative to the curPagePos of the file.
* If the user tries to read a block before the first page of
* the file, the method should return RC_READ_NON_EXISTING_PAGE.
 ************************************************************/
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->curPagePos-1, fHandle, memPage);
}

/************************************************************
* Read the current page relative to the curPagePos of the file.
* The curPagePos should be moved to the page that was read.
 ************************************************************/
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

/************************************************************
* Read the previous page relative to the curPagePos of the file.
* If the user tries to read a block after the last page of the
* file, the method should return RC_READ_NON_EXISTING_PAGE.
 ************************************************************/
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->curPagePos+1, fHandle, memPage);
}

/************************************************************
* Write a page to disk using an absolute position
 ************************************************************/
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

    if ( fHandle->fileName == NULL) {
    	THROW(RC_FILE_NOT_FOUND, "File not found");
    }

    // If pageNum invalid
    if( pageNum < 0){
        THROW(RC_READ_NON_EXISTING_PAGE, "Page number does not exist");
    }

    while(pageNum > fHandle->totalNumPages){
        appendEmptyBlock(fHandle);
    }

    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE * sizeof(char), SEEK_SET); // Set the pointer to beginning of pageNum

    size_t w = (fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo));
    if ( w != PAGE_SIZE){
        THROW(RC_WRITE_FAILED, "Write failed");
    }
    return RC_OK;
}

/************************************************************
* Write a page to disk using the current position
 ************************************************************/
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

/************************************************************
* Increase the number of pages in the file by one.
* The new last page should be filled with zero bytes.
 ************************************************************/
RC appendEmptyBlock (SM_FileHandle *fHandle){

    if ( fHandle->fileName == NULL) {
    	THROW(RC_FILE_NOT_FOUND, "File not found");
    }

    pageFile = fHandle->mgmtInfo;
    char* emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    memset(emptyPage, '\0', PAGE_SIZE);   //fill the content with '\0'

    fseek(pageFile, 0, SEEK_END);  //Move pointer to the end of file
    size_t w = fwrite(emptyPage, 1, PAGE_SIZE, pageFile);
    free(emptyPage);

    if (w != PAGE_SIZE){
    	THROW(RC_WRITE_FAILED, "Failed to append empty page");
    }else{
        fHandle->curPagePos = ++fHandle->totalNumPages;
        return RC_OK;
    }
}

/************************************************************
* If the file has less than numberOfPages pages then increase
* the size to numberOfPages.
 ************************************************************/
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){

    if ( fHandle->fileName == NULL) {
    	THROW(RC_FILE_NOT_FOUND, "File not found");
    }

    for(int i = fHandle->totalNumPages; i < numberOfPages; i++) {
        appendEmptyBlock(fHandle);
    }

    return RC_OK;
}
