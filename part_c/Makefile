all: main

clean:
	-rm main main.o io.o md5.o misc.o rand_utils.o sha1.o ui.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o

main: main.o io.o md5.o misc.o rand_utils.o sha1.o ui.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o
	gcc -ansi -pedantic-errors -g -lm -o main main.o io.o md5.o misc.o rand_utils.o sha1.o ui.o all_password_enumerator.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_enumerator.o password_generator.o password_part_generator.o random_password_enumerator.o rule_segment.o single_cased_dictionary_word_generator.o

main.o: main.c
	gcc -ansi -pedantic-errors -c -Wall -g -o main.o main.c

io.o: ../common/io.c ../common/io.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o io.o ../common/io.c

md5.o: ../common/md5.c ../common/md5.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o md5.o ../common/md5.c

misc.o: ../common/misc.c ../common/misc.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o misc.o ../common/misc.c

rand_utils.o: ../common/rand_utils.c ../common/rand_utils.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o rand_utils.o ../common/rand_utils.c

sha1.o: ../common/sha1.c ../common/sha1.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o sha1.o ../common/sha1.c

ui.o: ../common/ui.c ../common/ui.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o ui.o ../common/ui.c

all_password_enumerator.o: ../password/all_password_enumerator.c ../password/all_password_enumerator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o all_password_enumerator.o ../password/all_password_enumerator.c

alphabet_phrase_generator.o: ../password/alphabet_phrase_generator.c ../password/alphabet_phrase_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o alphabet_phrase_generator.o ../password/alphabet_phrase_generator.c

dictionary.o: ../password/dictionary.c ../password/dictionary.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o dictionary.o ../password/dictionary.c

dictionary_word_generator.o: ../password/dictionary_word_generator.c ../password/dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o dictionary_word_generator.o ../password/dictionary_word_generator.c

mixed_cased_dictionary_word_generator.o: ../password/mixed_cased_dictionary_word_generator.c ../password/mixed_cased_dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o mixed_cased_dictionary_word_generator.o ../password/mixed_cased_dictionary_word_generator.c

password_enumerator.o: ../password/password_enumerator.c ../password/password_enumerator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o password_enumerator.o ../password/password_enumerator.c

password_generator.o: ../password/password_generator.c ../password/password_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o password_generator.o ../password/password_generator.c

password_part_generator.o: ../password/password_part_generator.c ../password/password_part_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o password_part_generator.o ../password/password_part_generator.c

random_password_enumerator.o: ../password/random_password_enumerator.c ../password/random_password_enumerator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o random_password_enumerator.o ../password/random_password_enumerator.c

rule_segment.o: ../password/rule_segment.c ../password/rule_segment.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o rule_segment.o ../password/rule_segment.c

single_cased_dictionary_word_generator.o: ../password/single_cased_dictionary_word_generator.c ../password/single_cased_dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o single_cased_dictionary_word_generator.o ../password/single_cased_dictionary_word_generator.c
