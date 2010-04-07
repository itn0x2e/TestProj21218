all: create_authentication authenticate

clean:
	-rm create_authentication authenticate create_authentication.o authenticate.o io.o md5.o misc.o sha1.o ui.o auth_file.o

create_authentication: create_authentication.o io.o md5.o misc.o sha1.o ui.o auth_file.o
	gcc -ansi -pedantic-errors -g -lm -o create_authentication create_authentication.o io.o md5.o misc.o sha1.o ui.o auth_file.o

authenticate: authenticate.o io.o md5.o misc.o sha1.o ui.o auth_file.o
	gcc -ansi -pedantic-errors -g -lm -o authenticate authenticate.o io.o md5.o misc.o sha1.o ui.o auth_file.o

create_authentication.o: create_authentication.c
	gcc -ansi -pedantic-errors -c -Wall -g -o create_authentication.o create_authentication.c

authenticate.o: authenticate.c
	gcc -ansi -pedantic-errors -c -Wall -g -o authenticate.o authenticate.c

io.o: ../common/io.c ../common/io.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o io.o ../common/io.c

md5.o: ../common/md5.c ../common/md5.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o md5.o ../common/md5.c

misc.o: ../common/misc.c ../common/misc.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o misc.o ../common/misc.c

sha1.o: ../common/sha1.c ../common/sha1.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o sha1.o ../common/sha1.c

ui.o: ../common/ui.c ../common/ui.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o ui.o ../common/ui.c

auth_file.o: auth_file.c auth_file.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o auth_file.o auth_file.c
