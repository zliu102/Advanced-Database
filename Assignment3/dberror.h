/************************************************************
* File name: dberror.h
* Author: Group 5
*         Deman Yang(A20395988), Yifan Xu(A20385903)
*         Jiao Qu(A20386614), Ziyu Liu(A20377366)
* Version: 1.0
* Date: 02/03/2018
* Description: Database Error Code
 ************************************************************/
#ifndef DBERROR_H
#define DBERROR_H

#include "stdio.h"

/* module wide constants */
#define PAGE_SIZE 4096

/* return code definitions */
typedef int RC;

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4

//Start of Insertion
#define RC_GET_PAGE_SIZE_FAILED 5
#define RC_DELETED_FAILED 6
#define RC_NO_FILENAME 7
#define RC_FLUSHPOOL_FAILED 8
#define RC_CAPACITY_FAILED 9
#define RC_READ_FAILED 10
#define RC_PIN_FAILED 11
#define RC_UNPIN_FAILED 12
#define RC_CLOSE_FAILED 13
#define RC_OPEN_TABLE_FAILED 14
#define RC_CREATE_RECORD_FAILED 15
#define RC_SET_ATTR_FAILED 16
#define RC_RM_RECORD_NOT_EXIST 17
#define RC_ERROR 18
//End of Insertion

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303

/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError (RC error);
extern char *errorMessage (RC error);

#define THROW(rc,message) \
		do {			  \
			RC_message=message;	  \
			return rc;		  \
		} while (0)		  \

// check the return code and exit if it is an error
#define CHECK(code)							\
		do {									\
			int rc_internal = (code);						\
			if (rc_internal != RC_OK)						\
			{									\
				char *message = errorMessage(rc_internal);			\
				printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
				free(message);							\
				exit(1);							\
			}									\
		} while(0);


#endif
