
CFLAGS=-D_TEST_ -O0 -Wall -pg -g
CFLAGS= -O0 -Wall -pg -g
INCLUDEROOT=-I../../



all: 
	gcc $(INCLUDEROOT) $(CFLAGS) *.c ../malloc/*.c ../../*.c -lpthread  -o profiling_arb
	gcc $(INCLUDEROOT) $(CFLAGS) *.c -lpthread  -o profiling_std


test: all
	cd ../malloc && make clean && make 
	bash -c "time ./profiling_std"
	bash -c "time ./profiling_arb"
	./profiling_arb && gprof ./profiling_arb ./gmon.out | less


clean:
	rm -f ./*.o ./profiling_arb ./profiling_std
	rm -f ./gmon.out
	rm -f ./*.so





