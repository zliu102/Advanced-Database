all: test_assign3_1 test_assign3_2 test_expr

CC=gcc
CFLAGS=-Wall -c -g
OBJECTS= test_assign3_1.o test_assign3_2.o test_expr.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr.o buffer_mgr_stat.o

test_assign3_1: test_assign3_1.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	$(CC) test_assign3_1.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o -o test_assign3_1

test_assign3_2: test_assign3_2.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	$(CC) test_assign3_2.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o -o test_assign3_2
	
test_expr: test_expr.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	$(CC) test_expr.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o -o test_expr
	
test_assign3_1.o: test_assign3_1.c
	$(CC) $(CFLAGS) test_assign3_1.c

test_assign3_2.o: test_assign3_2.c
	$(CC) $(CFLAGS) test_assign3_2.c
	
test_expr.o: test_expr.c
	$(CC) $(CFLAGS) test_expr.c

record_mgr.o: record_mgr.c
	$(CC) $(CFLAGS) record_mgr.c

rm_serializer.o: rm_serializer.c
	$(CC) $(CFLAGS) rm_serializer.c

expr.o: expr.c
	$(CC) $(CFLAGS) expr.c

storage_mgr.o: storage_mgr.c
	$(CC) $(CFLAGS) storage_mgr.c

dberror.o: dberror.c
	$(CC) $(CFLAGS) dberror.c

buffer_mgr.o: buffer_mgr.c
	$(CC) $(CFLAGS) buffer_mgr.c

buffer_mgr_stat.o: buffer_mgr_stat.c
	$(CC) $(CFLAGS) buffer_mgr_stat.c

clean:
	rm -f $(OBJECTS) test_assign3_1 test_assign3_2 test_expr
	
