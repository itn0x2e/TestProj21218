
all: rt_ut

clean:
	-rm -f rt_ut

rt_ut: *.c
	gcc *.c unit-tests/rt_ut.c ../password/*.c ../common/*.c ../DEHT/*.c -lm -o rt_ut 
