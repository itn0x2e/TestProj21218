
all: out_dir create_authentication authenticate

out_dir:
	-mkdir ../bin/

create_authentication: create_authentication.c auth_file.c ../common/misc.c ../common/md5.c ../common/sha1.c
	gcc create_authentication.c auth_file.c ../common/misc.c ../common/md5.c ../common/sha1.c -o ../bin/create_authentication

authenticate: authenticate.c auth_file.c ../common/misc.c ../common/md5.c ../common/sha1.c
	gcc authenticate.c auth_file.c ../common/misc.c ../common/md5.c ../common/sha1.c -o ../bin/authenticate



