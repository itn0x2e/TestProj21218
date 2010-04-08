
CFLAGS=-Os -Wall -ansi -pedantic-errors -lm
CFLAGS_DEBUG=-g -DDEBUG -Wall -ansi -pedantic-errors -lm

all: rt_ut rt_ut_debug

clean:
	-rm -f rt_ut rt_ut_debug

rt_ut: *.c unit-tests/*.c
	gcc $(CFLAGS) *.c unit-tests/*.c ../password/*.c ../common/*.c ../DEHT/*.c -o rt_ut  

rt_ut_debug: *.c unit-tests/*.c
	gcc $(CFLAGS_DEBUG) *.c unit-tests/*.c ../password/*.c ../common/*.c ../DEHT/*.c -o rt_ut_debug
