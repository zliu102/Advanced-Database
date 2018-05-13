///************************************************************
//* File name: record_mgr.c
//* Author: Group 5
//*         Deman Yang(A20395988), Yifan Xu(A20385903)
//*         Jiao Qu(A20386614), Ziyu Liu(A20377366)
//* Version: 1.0
//* Date: 04/06/2018
//* Description: Record manager
// ************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
//#include "storage_mgr.h"
//#include "dberror.h"
#include "record_mgr.h"
//#include "tables.h"
//#include "expr.h"

//SM_FileHandle fHandle;
//SM_PageHandle sm_pHandle;
//BM_PageHandle *bm_pHandle;
//BM_BufferPool *bm;
int metaDataSize;// Size of Schema + number of tuples, which will be stored in the beginning of each pagefile
int schemaSize;  // Size of Schema
int recordSize;	// Size of Each Record
int maxSlotNum; //Total number of slots for each Page
//int numofTuples; // Total number of tuples
//char *charSchema; //Schema content
int TombstoneID[10000];

/***************************************************************
* Deserialize Schema from table to Schema struct
***************************************************************/
void deSerializeSchema(char *string, Schema **schema){
    char *temp1 = ") with keys: ";//13
    char *temp2 = "> attributes (";//14
//    char *temp3 = "Schema with <";//13
    char *temp4;
    char *temp5;
    int length1 = 0;
    int length2 = 0;
    int length3 = 0;
    char *key;
    char *attr;
    char *num;
//    printf(" open phase 0\n");
    temp4 = strstr(string, temp1);
    length1 = strlen(temp4);

    key = calloc(length1 - 14, sizeof(char));
    memcpy(key, temp4+13, length1 - 14);
//    printf("%s\n", key);//(a)

    temp5 = strstr(string, temp2);
    length2 = strlen(temp5);
    attr = calloc(length2 - length1 - 14, sizeof(char));
    memcpy(attr, temp5+14, length2 - length1 - 14);
//    printf("%s\n", attr);//a: INT, b: STRING[4], c: INT

    length3 = strlen(string);
    num = calloc(length3 - length2, sizeof(char));
    memcpy(num, string+13, length3 - length2 - 13);
//    printf( "%s\n", num);//3

    int numAttr;
    int *typeLength, *keyAttrs;
    int keySize;
    DataType * dataType;
    char **attrNames;
    numAttr = atoi(num);
	attrNames = (char **)calloc(numAttr, sizeof(char *));
	dataType = (DataType *)calloc(numAttr, sizeof(DataType));
	typeLength = (int *)calloc(numAttr, sizeof(int));
	int j = 0;
    for(int i = 0; i < length2 - length1 - 12; i++){
    	if(attr[i] == ':'){
    		attrNames[j] = (char *)calloc(1, sizeof(char));
    		memcpy(attrNames[j], &attr[i - 1], 1);
    		switch(attr[i+2]){
				case 'I':
					dataType[j] = DT_INT;
					typeLength[j] = 0;
					break;
				case 'S':
					dataType[j] = DT_STRING;
					typeLength[j] = atoi(attr+i+9);
					break;
				case 'F':
					dataType[j] = DT_FLOAT;
					typeLength[j] = 0;
					break;
				case 'B':
					dataType[j] = DT_BOOL;
					typeLength[j] = 0;
					break;
				default :
					break;
    		}
//    		printf("Attr:%s, Type is %d, Length is %d\n", attrNames[j], dataType[j], typeLength[j]);
    		j++;
    	}
    }
//    printf("Find keys 1\n");
    keySize = 0; // assign keySize
    keyAttrs = (int *)calloc(numAttr, sizeof(int));
//    printf("Find keys 2\n");
    char *tempkey = calloc(1, sizeof(char));
//    printf("Find keys 3\n");
//    printf("Find keys %c\n", *(key+1));
//    printf("Find keys %s\n", key+1);
    for(int i = 0; i < strlen(key) - 1; i++){
    	if( key[i] == '('){
    		memcpy(tempkey, key+i+1, 1);
    		for(int j = 0; j < numAttr; j++){
    			if ( strcmp(attrNames[j], tempkey) == 0){
    				keyAttrs[i] = j;
    			}
    		}
    		keySize++;
    	}
    }

    *schema = createSchema(numAttr, attrNames, dataType, typeLength, keySize, keyAttrs);

    free(key);
    free(attr);
    free(num);
    free(tempkey);
//    return *schema;
}

/***************************************************************
* Initial record manager
***************************************************************/
RC initRecordManager (void *mgmtData){

	for(int i=0;i<10000;i++)
	{
		TombstoneID[i] = -1; // -1 means empty slot
	}
    return RC_OK;
}

/***************************************************************
* Shut down record manager
***************************************************************/
RC shutdownRecordManager (){
	for(int i=0;i<10000;i++)
	{
		TombstoneID[i] = -1; // -1 means empty slot
	}
    return RC_OK;
}

/***************************************************************
* Creating a table and store information about the schema, number of tuples
***************************************************************/
RC createTable (char *name, Schema *schema){
	SM_PageHandle sm_pHandle;
	SM_FileHandle fHandle;
    if(createPageFile(name) != RC_OK){
    	return -1;
    }else if(openPageFile(name, &fHandle) != RC_OK){
    	return -1;
    }else{
    	//Do nothing
    }
	recordSize = getRecordSize(schema); // Get record size when open table, will be cleared in deleteTable

    // get metadata size
	//Schema with <3> attributes (a: INT, b: STRING[4], c: INT) with keys: (a)\n
	char *charSchema = serializeSchema(schema);
    schemaSize = strlen(charSchema);
	metaDataSize = schemaSize + sizeof(int);
    maxSlotNum = (PAGE_SIZE - metaDataSize) / recordSize;

    sm_pHandle = (char *)calloc(PAGE_SIZE, sizeof(char));
    memcpy(sm_pHandle, charSchema, metaDataSize); // Copy schema to the beginning of pagefile
    writeBlock(0, &fHandle, sm_pHandle);

    if(closePageFile(&fHandle) == RC_OK){
        free(sm_pHandle);
        return RC_OK;
    }
    return RC_ERROR;
}

/***************************************************************
* Open a table, enable buffer manager, read schema from buffer to table data
***************************************************************/
RC openTable (RM_TableData *rel, char *name){
	BM_BufferPool *bm = MAKE_POOL();
	BM_PageHandle *bm_pHandle = MAKE_PAGE_HANDLE();
	SM_FileHandle fHandle;
    char * charSchema;

    // open file and initial buffer pool
    if(openPageFile(name, &fHandle) != RC_OK){
    	return -1;
    }else if(initBufferPool(bm, name, 10, RS_LRU, NULL) != RC_OK){
    	return -1;
    }else{
    	//Do nothing
    }

    //Get the first page to extract Schema data
	if (pinPage(bm, bm_pHandle, 0) != RC_OK){
		return -1;
	}
	if( unpinPage(bm, bm_pHandle) != RC_OK){
		return -1;
	}
	charSchema = calloc(metaDataSize, sizeof(char));
    memcpy(charSchema, bm_pHandle->data, metaDataSize); // Copy schema from beginning of pagefile

    rel->schema = calloc(1, sizeof(Schema));
    deSerializeSchema(charSchema, &rel->schema);
    rel->name = name;
    rel->bm = bm;
    return RC_OK;
}

/***************************************************************
* Close a table
***************************************************************/
RC closeTable (RM_TableData *rel){
    freeSchema(rel->schema);
    shutdownBufferPool(rel->bm);
    free(rel->bm);
    return RC_OK;
}

/***************************************************************
 * Delete a table
***************************************************************/
RC deleteTable (char *name){
//	free(fHandle.fileName);
//    fHandle.fileName = NULL;
//    free(fHandle.mgmtInfo);
	//sm_pHandle = NULL;
	metaDataSize = 0;
	schemaSize = 0;
	recordSize = 0;
	maxSlotNum = 0;

    return destroyPageFile(name);
}

/***************************************************************
* Get the number of record from table data
***************************************************************/
int getNumTuples (RM_TableData *rel){
	BM_PageHandle *bm_pHandle = MAKE_PAGE_HANDLE();
    int numTuples;
    pinPage(rel->bm, bm_pHandle, 0);
    memcpy(&numTuples, bm_pHandle->data + schemaSize, sizeof(int));
    unpinPage(rel->bm, bm_pHandle);
    free(bm_pHandle);
    return numTuples;
//    return rel->numTuples;
}

/***************************************************************
* Insert a record into corresponding page and update RID in Record
***************************************************************/
RC insertRecord (RM_TableData *rel, Record *record){
	BM_PageHandle *bm = MAKE_PAGE_HANDLE();
//	bm_pHandle = MAKE_PAGE_HANDLE();
    // Check if there's empty Slot, slot number starts from 0
	int emptySlot = -1;
    int numofTuples = getNumTuples(rel);

    for(int i = 0; i < numofTuples; i++){
    	if( TombstoneID[i] == -1 ){
    		emptySlot = i;
    		break;
    	}
    }
    if( emptySlot == -1){
    	emptySlot = numofTuples;
    }

    record->id.page = emptySlot / maxSlotNum;//Page starts from 0

    int pagePos = 0;
    if ( emptySlot < maxSlotNum ){// Check if it's in the first page
    	record->id.slot = emptySlot;
    }else{
    	record->id.slot = emptySlot % maxSlotNum;
    }

	pagePos = metaDataSize + record->id.slot * recordSize;

    pinPage(rel->bm, bm, record->id.page);
    memcpy(bm->data+pagePos, record->data, recordSize);

    markDirty(rel->bm, bm);
    unpinPage(rel->bm, bm);

    numofTuples++;
    pinPage(rel->bm, bm, 0);
	memcpy(bm->data+schemaSize, &numofTuples, sizeof(int));
	markDirty(rel->bm, bm);
	unpinPage(rel->bm, bm);


//    printf("Inert Record, current tuple is %d\n", rel->numTuples);
    TombstoneID[emptySlot] = 1; //Update TombstoneID indicator
    free(bm);

//    //Start of test
//    pinPage(rel->bm, bm_pHandle, record->id.page);
//    memcpy(testR.data, bm_pHandle->data+pagePos, recordSize);
//    printf("New data after free in page %d, slot %d\n", record->id.page, record->id.slot);
//	for(int i = 0; i < rel->schema->numAttr; i++){
//		char *test;
//		Value *testvalue;
//		getAttr(&testR, rel->schema, i, &testvalue);
//		test = serializeValue(testvalue);
//		printf("the %d element is %s\n", i , test);
//		free(test);
//		free(testvalue);
//	}
//	//End of testing
    return RC_OK;
}

/***************************************************************
* Delete a record by id
***************************************************************/
RC deleteRecord (RM_TableData *rel, RID id){
	BM_PageHandle *bm = MAKE_PAGE_HANDLE();
//	bm_pHandle = MAKE_PAGE_HANDLE();

    // Delete record and mark record status as false(calloc 0).
    pinPage(rel->bm, bm, id.page);
    memset(bm->data + metaDataSize + id.slot*recordSize, '\0',recordSize);
    markDirty(rel->bm, bm);
    unpinPage(rel->bm, bm);

    int numofTuples = getNumTuples(rel);
    numofTuples--;
    pinPage(rel->bm, bm, 0);
	memcpy(bm->data+schemaSize, &numofTuples, sizeof(int));
	markDirty(rel->bm, bm);
	unpinPage(rel->bm, bm);
	TombstoneID[id.page*maxSlotNum + id.slot] = -1;
    free(bm);
    return RC_OK;
}

/***************************************************************
* Update a record by its id
***************************************************************/
RC updateRecord (RM_TableData *rel, Record *record){
	BM_PageHandle *bm = MAKE_PAGE_HANDLE();
//	bm_pHandle = MAKE_PAGE_HANDLE();

    pinPage(rel->bm, bm, record->id.page);

    memcpy(bm->data + metaDataSize + record->id.slot*recordSize, record->data, recordSize);

    markDirty(rel->bm, bm);
    unpinPage(rel->bm, bm);

    free(bm);
    return RC_OK;
}

/***************************************************************
* Get a record by id from rel
***************************************************************/
RC getRecord (RM_TableData *rel, RID id, Record *record){
	BM_PageHandle *bm = MAKE_PAGE_HANDLE();
//	bm_pHandle = MAKE_PAGE_HANDLE();

	record->id = id;
    pinPage(rel->bm, bm, id.page);
    record->data = (char*)calloc(recordSize, sizeof(char));
	memcpy(record->data, bm->data + metaDataSize + record->id.slot*recordSize, recordSize);
	unpinPage(rel->bm, bm);
	free(bm);
	return RC_OK;
}

/***************************************************************
* Initialize a scan with rel and cond
***************************************************************/
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
    scan->rel=rel;
//	printf("Start Scan, total tuple is %d\n", rel->numTuples);

    scan->currentPage = 0;
    scan->currentSlot = 0;
    scan->totalSlot = 0;
    scan->expr=cond;
    return RC_OK;
}

/***************************************************************
* Search the table and store result in record with condition in scan
***************************************************************/
RC next (RM_ScanHandle *scan, Record *record){
    BM_PageHandle *bm = MAKE_PAGE_HANDLE(); //why must i declare it in local?Issue: free(bm) will lead record lose data
//    bm_pHandle = MAKE_PAGE_HANDLE();
    Record *temp=(Record *)calloc(1,sizeof(Record));
    Value *result=(Value *)calloc(1,sizeof(Value));
    RID rid;

    int numofTuples = getNumTuples(scan->rel);
    // If totalSlot equal number of tuples, it means all records are scanned
    while(scan->totalSlot != numofTuples){//scan->rel->numTuples){
        pinPage(scan->rel->bm, bm, scan->currentPage);
        for(int i = scan->currentSlot; i < maxSlotNum; i++){
        	rid.page = scan->currentPage;
        	rid.slot = i;
        	scan->totalSlot++;
        	if(getRecord(scan->rel,rid,temp) == RC_OK){
                evalExpr(temp, scan->rel->schema, scan->expr,&result);
                if(result->v.boolV){
                    record->id.page= temp->id.page;
                    record->id.slot= temp->id.slot;
                    record->data= temp->data;

                    if(i == maxSlotNum - 1)//Last record of certain page
                    {
                        scan->currentPage++;
                        scan->currentSlot=0;
                        break;
                    }
                    else{
                        scan->currentSlot=i+1;
                    }
                    free(result);
                    //free(temp);
                    unpinPage (scan->rel->bm, bm);
                    free(bm);
                    return RC_OK;
                }
        	}

        	if(scan->totalSlot == numofTuples){//scan->rel->numTuples){
        		unpinPage(scan->rel->bm, bm);
        		free(bm);
        		return RC_RM_NO_MORE_TUPLES;
        	}

            if(i == maxSlotNum - 1)//Last record of certain page
            {
                scan->currentPage++;
                scan->currentSlot=0;
                break;
            }
            else{
                scan->currentSlot=i+1;
            }
        }
    }

	if(scan->totalSlot == numofTuples){
		unpinPage(scan->rel->bm, bm);
		free(bm);
		return RC_RM_NO_MORE_TUPLES;
	}
	return RC_ERROR;
}

/***************************************************************
* Free the scan
***************************************************************/
RC closeScan (RM_ScanHandle *scan){
//    free(scan->rel);
//    free(scan->expr);
//    free(scan);

    return RC_OK;
}

/***************************************************************
* Get the size of record described by schema
***************************************************************/
int getRecordSize (Schema *schema){
    int result = 0;
    for (int i = 0; i < schema->numAttr; i++)
    {
        switch (schema->dataTypes[i])
        {
        case DT_INT:
            result += sizeof(int);
            break;
        case DT_FLOAT:
            result += sizeof(float);
            break;
        case DT_BOOL:
            result += sizeof(bool);
            break;
        case DT_STRING:
            result += schema->typeLength[i];
            break;
        }
    }
    return result;
}

/***************************************************************
* Create a new schema described by the parameters
***************************************************************/
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){
    Schema *newschema = (Schema*)malloc(sizeof(Schema));
    newschema->numAttr = numAttr;
    newschema->attrNames = attrNames;
    newschema->dataTypes = dataTypes;
    newschema->typeLength = typeLength;
    newschema->keySize = keySize;
    newschema->keyAttrs = keys;
    return newschema;
}

/***************************************************************
* Free schema
***************************************************************/
RC freeSchema (Schema *schema){
	schema->keySize = 0;
	schema->numAttr = 0;
    free(schema->keyAttrs);
    free(schema->typeLength);
    free(schema->dataTypes);
    for (int i = 0; i < schema->numAttr; i++)
        free(schema->attrNames[i]);
    free(schema->attrNames);
    free(schema);
    return RC_OK;
}

/***************************************************************
* Create a record by the schema
***************************************************************/
RC createRecord (Record **record, Schema *schema){
    *record = (struct Record *)calloc(1, sizeof(struct Record));
    (*record)->data = (char*)calloc(recordSize, sizeof(char));
    return RC_OK;
}

/***************************************************************
* Free the space of a record
***************************************************************/
RC freeRecord (Record *record){
    free(record->data);
    free(record);
    return RC_OK;
}

/***************************************************************
* Get the value of a record
***************************************************************/
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
    int offset = 0;
    char end = '\0';

    // Calculate offset
    for (int i = 0; i < attrNum; i++) {
        switch (schema->dataTypes[i]) {
        case DT_INT:
            offset += sizeof(int);
            break;
        case DT_FLOAT:
            offset += sizeof(float);
            break;
        case DT_BOOL:
            offset += sizeof(bool);
            break;
        case DT_STRING:
            offset += schema->typeLength[i];
            break;
        }
    }

    // Get value from record
    *value = (Value *)malloc(sizeof(Value));
    (*value)->dt = schema->dataTypes[attrNum];
    switch (schema->dataTypes[attrNum])
    {
    case DT_INT:
        memcpy(&((*value)->v.intV), record->data + offset, sizeof(int));
        break;
    case DT_FLOAT:
        memcpy(&((*value)->v.floatV), record->data + offset, sizeof(float));
        break;
    case DT_BOOL:
        memcpy(&((*value)->v.boolV), record->data + offset, sizeof(int));
        break;
    case DT_STRING:
        // Append end:\0 in the end of string
        (*value)->v.stringV = (char *)malloc(schema->typeLength[attrNum] + 1);
        memcpy((*value)->v.stringV, record->data + offset, schema->typeLength[attrNum]);
        memcpy((*value)->v.stringV + schema->typeLength[attrNum], &end, 1);
        break;
    }
    return RC_OK;
}

/***************************************************************
* Set Record *record, Schema *schema, int attrNum, Value *value
***************************************************************/
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
    int offset = 0;
    // Calculate offset
    for (int i = 0; i < attrNum; i++) {
        switch (schema->dataTypes[i]) {
        case DT_INT:
            offset += sizeof(int);
            break;
        case DT_FLOAT:
            offset += sizeof(float);
            break;
        case DT_BOOL:
            offset += sizeof(bool);
            break;
        case DT_STRING:
            offset += schema->typeLength[i];
            break;
        }
    }
    // Set value into record
    switch (schema->dataTypes[attrNum])
    {
    case DT_INT:
        memcpy(record->data + offset, &(value->v.intV), sizeof(int));
        break;
    case DT_FLOAT:
        memcpy(record->data + offset, &(value->v.floatV), sizeof(float));
        break;
    case DT_BOOL:
        memcpy(record->data + offset, &(value->v.boolV), sizeof(int));
        break;
    case DT_STRING:
        // Calculate the strlen of the input string.
        if (strlen(value->v.stringV) >= schema->typeLength[attrNum]) {
            memcpy(record->data + offset, value->v.stringV, schema->typeLength[attrNum]);
        } else {
            strcpy(record->data + offset, value->v.stringV);
        }
        break;
    }
    return RC_OK;
}
