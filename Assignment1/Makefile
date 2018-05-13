
CC=gcc
CFLAGS=-Wall -c -o 
OBJECTS=test_assign1_1.o dberror.o storage_mgr.o

all: 	$(OBJECTS)
	$(CC) $(OBJECTS) -o test_assign1

test_assign1_1.o:
	$(CC) $(CFLAGS) test_assign1_1.o test_assign1_1.c 

dberror.o:
	$(CC) $(CFLAGS) dberror.o dberror.c 

storage_mgr.o:
	$(CC) $(CFLAGS) storage_mgr.o storage_mgr.c 

clean:
	rm -f $(OBJECTS) test_assign1
