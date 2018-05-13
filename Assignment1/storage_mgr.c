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
#include <stdbool.h>
#include "storage_mgr.h"
#include "dberror.h"

/************************************************************
* Global variables
 ************************************************************/
bool isOpen = false; //( false = not open; true = open
char* fileName = NULL;
FILE *pageFile = NULL;

SM_FileHandle fHandle;
SM_PageHandle memPage;// = malloc(sizeof(SM_PageHandle) * PAGE_SIZE);



/***********************************************************
 * Entrance of the program
 ***********************************************************/
/*
int main()
{
    int comm; // Command
    int ctrl; // Control the loop
    int result;
    fHandle.fileName = malloc(sizeof(char) * 20);
    memset(fHandle.fileName, 0 , sizeof(char) * 20);
    memPage = malloc(sizeof(SM_PageHandle) * PAGE_SIZE);
    memset(memPage, 0, sizeof(SM_PageHandle) * PAGE_SIZE);

    ctrl = 1;
    while (ctrl == 1)
    {
        printf("***********************************************************\n");
        printf("*          CS525 Storage Manager                          *\n");
        printf("*                                                         *\n");
        printf("* (1) Create file                                         *\n");
        printf("* (2) Open file                                           *\n");
        printf("* (3) Close file                                          *\n");
        printf("* (4) Destroy file                                        *\n");
        //printf("* (5) Best Router for Broadcast                           *\n");
        printf("* (5) Exit                                                *\n");
        printf("***********************************************************\n");
        printf("\nCommand:");
        scanf("%d", &comm);
        switch (comm)
        {
            case 1: // Create
                ctrl = 1;
                //getFileName();
                result = createPageFile(&fileName);
                break;
            case 2: // Open
                ctrl = 1;
                //getFileName();
                result = openPageFile(&fileName, &fHandle);
                break;
            case 3: // Close
                ctrl = 1;
                result = closePageFile(&fHandle);
                break;
            case 4: // Delete
                ctrl = 1;
                //getFileName();
                result = destroyPageFile(&fileName);
                break;
            case 5: // Exit
                ctrl = 0;
                initStorageManager();
                return 0;
            default:
                ctrl = 1;
                printf("Wrong Choice, choose again!\n");
        }
        printError(result);
    }
};*/

/***********************************************************
 * Get file name
 ***********************************************************/
/*void getFileName( ){
    printf("Enter the file name:\n");
    scanf("%s", &fileName);
}*/

/************************************************************
 * Initialization
 ************************************************************/
void initStorageManager(){

    //free(fHandle.fileName);
    fHandle.fileName = NULL;

    fHandle.curPagePos = 0;
    fHandle.totalNumPages = 0;

    //free(fHandle.mgmtInfo);
    fHandle.mgmtInfo = NULL;

    free(memPage);
    //TBD;
}

/************************************************************
* Create a new page file fileName. The initial file size should
* be one page. This method should fill this single page with
* '\0' bytes.
 ************************************************************/
RC createPageFile(char* fileName) {

    char emptyPage[PAGE_SIZE];
    bool flag = true;

    pageFile = fopen(fileName, "w+"); //Open file for read and write(overwrite existing one)
    if(pageFile == NULL) {
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
    }

    memset(emptyPage, '\0', PAGE_SIZE);   //fill the content with '\0'

    if( fwrite(emptyPage, 1, PAGE_SIZE, pageFile) != PAGE_SIZE){
        flag = false;
    }else{
        (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    }

    fclose(pageFile); //Close file

    if (flag){
        RC_message = "File created successfully";
        return RC_OK;
    }else{
        RC_message = "File created failed!";
        return RC_WRITE_FAILED;
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
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
    }

    fHandle->fileName = fileName;   // Record the filename
    if (fseek(pageFile, 0, SEEK_END) != 0){ //Move pointer to the end
        fclose(pageFile);
        return RC_GET_PAGE_SIZE_FAILED;
    };

    fHandle->totalNumPages = (int)(ftell(pageFile) / PAGE_SIZE); //Get total number of pages
    fseek(pageFile, 0, SEEK_SET); //Move pointer back to the beginning

    fHandle->curPagePos = 0;       // Set the current position to the beginning of file

    fHandle->mgmtInfo = pageFile;  // POSIX file descriptor

    RC_message = "File opened successfully";
    return RC_OK;
}

/************************************************************
* Close an open page file.
 ************************************************************/
RC closePageFile (SM_FileHandle *fHandle){
    if (fHandle->mgmtInfo == NULL){
        RC_message = "File is not open";
        return RC_FILE_HANDLE_NOT_INIT;
    }

    fclose(fHandle->mgmtInfo); // Close file descriptor

    initStorageManager();
    RC_message = "File closed successfully";
    return RC_OK;
}

/************************************************************
* Destroy(delete) an open page file.
 ************************************************************/
RC destroyPageFile (char *fileName){
    if (remove(fileName) != 0) {
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
    }

    initStorageManager();
    RC_message = "File deleted successfully";
    return RC_OK;
}

/************************************************************
* The method reads the pageNumth block from a file and stores
* its content in the memory pointed to by the memPage page
* handle. If the file has less than pageNum pages, the method
* should return RC_READ_NON_EXISTING_PAGE.
 ************************************************************/
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

    if ( fHandle->fileName == NULL) {
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
    }

    // If the file has less than pageNum pages or pageNum invalid
    if(pageNum > fHandle->totalNumPages || pageNum < 0){
        RC_message = "Page number does not exist";
        return RC_READ_NON_EXISTING_PAGE;
    }

    //FILE *fp = fHandle->mgmtInfo;
    fHandle->curPagePos = pageNum;  // Set current page position.
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE * sizeof(char), SEEK_SET); // Set the pointer to pageNum
    fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo); // Read the block to memPage
    return RC_OK;
}

/************************************************************
* Return the current page position in a file
 ************************************************************/
int getBlockPos (SM_FileHandle *fHandle){
    if ( fHandle->fileName == NULL) {
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
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
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
    }

    // If the file has less than pageNum pages or pageNum invalid
    if(pageNum > fHandle->totalNumPages || pageNum < 0){
        RC_message = "Page number does not exist";
        return RC_READ_NON_EXISTING_PAGE;
    }

    //FILE *fp = fHandle->mgmtInfo;
    fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE * sizeof(char), SEEK_SET); // Set the pointer to pageNum

    //fread(memPage, 1, PAGE_SIZE, fHandle->mgmtInfo); // Read the block to memPage
    int i = (fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo));
    if ( i != PAGE_SIZE){
        RC_message = "Write failed";
        return RC_WRITE_FAILED;
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
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
    }

    FILE *fp = fHandle->mgmtInfo;
    char* emptyPage = (char*)malloc(sizeof(char) * PAGE_SIZE); //prepare the page
    memset(emptyPage, '\0', PAGE_SIZE); //fill the page content with '\0'
    fseek(fp, 0, SEEK_END);  //Move pointer to the end of file
    if ((fwrite(emptyPage, 1, PAGE_SIZE, fp)) != PAGE_SIZE){
        RC_message = "Failed to append empty page";
        return RC_WRITE_FAILED;
    }
    fHandle->totalNumPages++;
    fHandle->curPagePos = fHandle->totalNumPages;
    return RC_OK;
}

/************************************************************
* If the file has less than numberOfPages pages then increase
* the size to numberOfPages.
 ************************************************************/
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){

    if ( fHandle->fileName == NULL) {
        RC_message = "File not found";
        return RC_FILE_NOT_FOUND;
    }

    for(int i = fHandle->totalNumPages; i < numberOfPages; i++) {
        appendEmptyBlock(fHandle);
    }

    return RC_OK;
}
