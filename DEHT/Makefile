all: deht_ut deht_ut_debug

OBJS=DEHT.o unit-tests/deht_ut.o hash_funcs.o io.o md5.o sha1.o misc.o 
DEBUG_OBJS=debug__DEHT.o debug__hash_funcs.o unit-tests/debug__deht_ut.o debug__io.o debug__md5.o debug__sha1.o debug__misc.o

CFLAGS= -pedantic-errors -Wall -ansi -Os -c
CFLAGS_DEBUG = -pedantic-errors -Wall -ansi -g -DDEBUG -c

LDFLAGS= 
LDFLAGS_DEBUG=


clean:
	-rm deht_ut deht_ut_debug $(OBJS) $(DEBUG_OBJS) 

deht_ut: $(OBJS)
	gcc $(LDFLAGS) -o deht_ut $(OBJS)

deht_ut_debug: $(DEBUG_OBJS)
	gcc $(LDFLAGS_DEBUG) -o deht_ut_debug $(DEBUG_OBJS)


DEHT.o:
	gcc $(CFLAGS) -o DEHT.o DEHT.c

unit-tests/deht_ut.o: unit-tests/deht_ut.c 
	gcc $(CFLAGS) -o unit-tests/deht_ut.o unit-tests/deht_ut.c

hash_funcs.o: hash_funcs.c 
	gcc $(CFLAGS) -o hash_funcs.o hash_funcs.c


io.o: ../common/io.c ../common/io.h 
	gcc $(CFLAGS) -o io.o ../common/io.c

misc.o: ../common/misc.c ../common/misc.h 
	gcc $(CFLAGS) -o misc.o ../common/misc.c

md5.o: ../common/md5.c ../common/md5.h 
	gcc $(CFLAGS) -o md5.o ../common/md5.c

sha1.o: ../common/sha1.c ../common/sha1.h 
	gcc $(CFLAGS) -o sha1.o ../common/sha1.c




debug__DEHT.o:
	gcc $(CFLAGS_DEBUG) -o debug__DEHT.o DEHT.c

unit-tests/debug__deht_ut.o: unit-tests/deht_ut.c 
	gcc $(CFLAGS_DEBUG) -o unit-tests/debug__deht_ut.o unit-tests/deht_ut.c

debug__hash_funcs.o: hash_funcs.c 
	gcc $(CFLAGS) -o debug__hash_funcs.o hash_funcs.c


debug__io.o: ../common/io.c ../common/io.h 
	gcc $(CFLAGS_DEBUG) -o debug__io.o ../common/io.c

debug__misc.o: ../common/misc.c ../common/misc.h 
	gcc $(CFLAGS_DEBUG) -o debug__misc.o ../common/misc.c

debug__md5.o: ../common/md5.c ../common/md5.h 
	gcc $(CFLAGS_DEBUG) -o debug__md5.o ../common/md5.c

debug__sha1.o: ../common/sha1.c ../common/sha1.h 
	gcc $(CFLAGS_DEBUG) -o debug__sha1.o ../common/sha1.c

