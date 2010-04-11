all: create_authentication create_salty_authentication authenticate salty_authenticate exhaustive_table_generator exhaustive_query create_rainbow_table crack_using_rainbow_table text_export_rainbow_table

clean:
	-rm create_authentication create_salty_authentication authenticate salty_authenticate exhaustive_table_generator exhaustive_query create_rainbow_table crack_using_rainbow_table text_export_rainbow_table create_authentication.o create_salty_authentication.o authenticate.o salty_authenticate.o exhaustive_table_generator.o exhaustive_query.o create_rainbow_table.o crack_using_rainbow_table.o text_export_rainbow_table.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o textual_dumper.o

create_authentication: create_authentication.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o create_authentication create_authentication.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

create_salty_authentication: create_salty_authentication.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o create_salty_authentication create_salty_authentication.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

authenticate: authenticate.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o authenticate authenticate.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

salty_authenticate: salty_authenticate.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o salty_authenticate salty_authenticate.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

exhaustive_table_generator: exhaustive_table_generator.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o exhaustive_table_generator exhaustive_table_generator.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

exhaustive_query: exhaustive_query.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o exhaustive_query exhaustive_query.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

create_rainbow_table: create_rainbow_table.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o create_rainbow_table create_rainbow_table.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

crack_using_rainbow_table: crack_using_rainbow_table.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o
	gcc -ansi -pedantic-errors -s -Os -lm -o crack_using_rainbow_table crack_using_rainbow_table.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o

text_export_rainbow_table: text_export_rainbow_table.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o textual_dumper.o
	gcc -ansi -pedantic-errors -s -Os -lm -o text_export_rainbow_table text_export_rainbow_table.o io.o md5.o misc.o rand_utils.o sha1.o ui.o DEHT.o hash_funcs.o auth_file.o authenticate_common.o create_authentication_common.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o rainbow_table.o textual_dumper.o

create_authentication.o: part_a/create_authentication.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o create_authentication.o part_a/create_authentication.c

create_salty_authentication.o: part_a/create_salty_authentication.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o create_salty_authentication.o part_a/create_salty_authentication.c

authenticate.o: part_a/authenticate.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o authenticate.o part_a/authenticate.c

salty_authenticate.o: part_a/salty_authenticate.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o salty_authenticate.o part_a/salty_authenticate.c

exhaustive_table_generator.o: part_b/exhaustive_table_generator.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o exhaustive_table_generator.o part_b/exhaustive_table_generator.c

exhaustive_query.o: part_b/exhaustive_query.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o exhaustive_query.o part_b/exhaustive_query.c

create_rainbow_table.o: part_c/create_rainbow_table.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o create_rainbow_table.o part_c/create_rainbow_table.c

crack_using_rainbow_table.o: part_c/crack_using_rainbow_table.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o crack_using_rainbow_table.o part_c/crack_using_rainbow_table.c

text_export_rainbow_table.o: part_c/text_export_rainbow_table.c
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o text_export_rainbow_table.o part_c/text_export_rainbow_table.c

io.o: common/io.c common/io.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o io.o common/io.c

md5.o: common/md5.c common/md5.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o md5.o common/md5.c

misc.o: common/misc.c common/misc.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o misc.o common/misc.c

rand_utils.o: common/rand_utils.c common/rand_utils.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o rand_utils.o common/rand_utils.c

sha1.o: common/sha1.c common/sha1.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o sha1.o common/sha1.c

ui.o: common/ui.c common/ui.h
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o ui.o common/ui.c

DEHT.o: DEHT/DEHT.c DEHT/DEHT.h
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o DEHT.o DEHT/DEHT.c

hash_funcs.o: DEHT/hash_funcs.c DEHT/hash_funcs.h
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o hash_funcs.o DEHT/hash_funcs.c

auth_file.o: part_a/auth_file.c part_a/auth_file.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o auth_file.o part_a/auth_file.c

authenticate_common.o: part_a/authenticate_common.c part_a/authenticate_common.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o authenticate_common.o part_a/authenticate_common.c

create_authentication_common.o: part_a/create_authentication_common.c part_a/create_authentication_common.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o create_authentication_common.o part_a/create_authentication_common.c

all_password_enumerator.o: password/all_password_enumerator.c password/all_password_enumerator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o all_password_enumerator.o password/all_password_enumerator.c

alphabet_phrase_generator.o: password/alphabet_phrase_generator.c password/alphabet_phrase_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o alphabet_phrase_generator.o password/alphabet_phrase_generator.c

dictionary.o: password/dictionary.c password/dictionary.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o dictionary.o password/dictionary.c

dictionary_word_generator.o: password/dictionary_word_generator.c password/dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o dictionary_word_generator.o password/dictionary_word_generator.c

mixed_cased_dictionary_word_generator.o: password/mixed_cased_dictionary_word_generator.c password/mixed_cased_dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o mixed_cased_dictionary_word_generator.o password/mixed_cased_dictionary_word_generator.c

password_enumerator.o: password/password_enumerator.c password/password_enumerator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o password_enumerator.o password/password_enumerator.c

password_generator.o: password/password_generator.c password/password_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o password_generator.o password/password_generator.c

password_part_generator.o: password/password_part_generator.c password/password_part_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o password_part_generator.o password/password_part_generator.c

random_password_enumerator.o: password/random_password_enumerator.c password/random_password_enumerator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o random_password_enumerator.o password/random_password_enumerator.c

rule_segment.o: password/rule_segment.c password/rule_segment.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o rule_segment.o password/rule_segment.c

single_cased_dictionary_word_generator.o: password/single_cased_dictionary_word_generator.c password/single_cased_dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o single_cased_dictionary_word_generator.o password/single_cased_dictionary_word_generator.c

rainbow_table.o: rainbow_table/rainbow_table.c rainbow_table/rainbow_table.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o rainbow_table.o rainbow_table/rainbow_table.c

textual_dumper.o: rainbow_table/textual_dumper.c rainbow_table/rainbow_table.h 
	gcc -ansi -pedantic-errors -c -Wall -s -Os -o textual_dumper.o rainbow_table/textual_dumper.c
