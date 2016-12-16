
CFLAGS=-D_TEST_ -O0 -Wall -pg -g -ftest-coverage -fprofile-arcs
INCLUDEROOT=-I../../



all: 
	gcc $(INCLUDEROOT) $(CFLAGS) ../malloc/*.c ../profiling/*.c ../../*.c -lpthread  -o coverage


test: clean all
	./coverage
	gcov ../profiling/*.c ../malloc/*.c ../../*.c
	vim ./arb.c.gcov



clean:
	rm -f ./*.o ./*.so ./coverage
	rm -f ./gmon.out
	rm -f ./*.gcov ./*.gcda ./*.gcno





