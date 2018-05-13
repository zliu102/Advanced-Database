
all: test_assign2

test_assign2: test_assign2_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o 
	gcc -g test_assign2_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o -o test_assign2

test_assign2_1.o: test_assign2_1.c test_helper.h
	gcc -g -c test_assign2_1.c

storage_mgr.o: storage_mgr.c storage_mgr.h
	gcc -g -c storage_mgr.c


buffer_mgr.o: buffer_mgr.c buffer_mgr.h
	gcc -g -c buffer_mgr.c

storage_mgr_stat.o: buffer_mgr_stat.c buffer_mgr_stat.h
	gcc -g -c buffer_mgr_stat.c

dberror.o: dberror.c dberror.h
	gcc -g -c dberror.c

clean:
	rm buffer_mgr.o dberror.o test_assign2_1.o 
	storage_mgr.o buffer_mgr_stat.o test_assign2
