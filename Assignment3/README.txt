Group 5

Main functionality.
   1. in file rm_serializer.c, fixed a bug of calloc in line 21.
	 2. implement TID and Tombstone concept.( Extra Credit )
	 3. In the beginning of the first page, we store the schema and number of tuples. For the rest of the space, we store the records.


1. Task Assignment:

  Deman Yang(A20395988): initRecordManager, shutdownRecordManager, createTable, openTable, closeTable, deleteTable, getNumTuples, 

  Jiao Qu(A20386614): insertRecord, deleteRecord, updateRecord, getRecord, testing, documentation

  Yifan Xu(A20385903): startScan, next, closeScan, getRecordSize, createSchema, freeSchema, deSerializeSchema.

  Ziyu Liu(A20377366): createRecord, freeRecord, getAttr, setAttr, Makefile
  
2. Test steps(in Linux system, suppose gcc is installed)

  1. Download all the source code in the same folder.
  2. Open terminal, use command "cd" to access this folder.
  3. Input command "make", press Enter.
  4. Input command "./test_assign3_1”, press Enter.
  5. Input command "./test_assign3_2”, press Enter.
  6. Input command "./test_expr”, press Enter.
  7. After test, use clean to delete files except source code.
  

3. File list 

  - Makefile
  - buffer_mgr.c
	- buffer_mgr.h
	-	buffer_mgr_stat.c
	-	buffer_mgr_stat.h
	-	dberror.c
	-	dberror.h
	-	expr.c
	-	expr.h
	-	record_mgr.c
	-	record_mgr.h
	-	rm_serializer.c
	-	storage_mgr.c
	-	storage_mgr.h
	-	tables.h
	-	test_assign3_1.c
	- test_assign3_2.c
	-	test_expr.c
	-	test_helper.h


4. Function descriptions: of all additional functions Functionalities of Record Manager contains 5 parts: 

  Table and Manager
    initRecordManager: Initial record manager, setup TID and Tombstone
    shutdownRecordManager: Shut down record manager, reset TID and Tombstone
    createTable: Creating a table and store information about the schema, number of tuples
    openTable: Open a table, enable buffer manager, read schema from buffer to table data
    closeTable: Close a table
    deleteTable: Delete a tables
    getNumTuples: Get the number of record from table data
    
  Handling records in a table
    insertRecord: Insert a record into corresponding page and update RID in Record with TID concept
    deleteRecord: Delete a record by id
    updateRecord: Update a record by its id
    getRecord: Get a record by id from rel
    
  Scans
    startScan: Initialize a scan with rel and cond
    next: Search the table and store result in record with condition in scan
    closeScan: Free the scan, do nothing
    
  Dealing with schemas  
    getRecordSize: Get the size of record described by schema
    createSchema: Create a new schema described by the parameters
    freeSchema: Free schema
    deSerializeSchema: Deserialize Schema from table to Schema struct
    
  Dealing with records and attribute values  
    createRecord: Create a record by the schema
    freeRecord: Free the space of a record
    getAttr: Get the value of a record
    setAttr: Set Record *record, Schema *schema, int attrNum, Value *value
    
    
5. Data structure Enhancement: main data structure used

  typedef struct RM_ScanHandle
  {
    RM_TableData *rel;
    int currentPage; //Enhancement, start from 0
    int currentSlot; //Enhancement, start from 0
    int totalSlot;   //Enhancement, start from 1, same as tuples
    Expr *expr; //Enhancement
    void *mgmtData;
  } RM_ScanHandle;

  typedef struct RM_TableData
  {
  	char *name;
  	Schema *schema;
  	BM_BufferPool *bm;//Enhancement
  //	void *mgmtData;
  } RM_TableData;



