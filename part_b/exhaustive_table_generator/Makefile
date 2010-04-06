all: exhaustive_table_generator

clean:
	-rm exhaustive_table_generator exhaustive_table_generator.o md5.o misc.o sha1.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_generator.o password_part_generator.o rule_segment.o single_cased_dictionary_word_generator.o

exhaustive_table_generator: exhaustive_table_generator.o md5.o misc.o sha1.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_generator.o password_part_generator.o rule_segment.o single_cased_dictionary_word_generator.o
	gcc -ansi -pedantic-errors -g -lm -o exhaustive_table_generator exhaustive_table_generator.o md5.o misc.o sha1.o alphabet_phrase_generator.o dictionary.o dictionary_word_generator.o mixed_cased_dictionary_word_generator.o password_generator.o password_part_generator.o rule_segment.o single_cased_dictionary_word_generator.o

exhaustive_table_generator.o: exhaustive_table_generator.c exhaustive_table_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o exhaustive_table_generator.o exhaustive_table_generator.c

md5.o: ../../common/md5.c ../../common/md5.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o md5.o ../../common/md5.c

misc.o: ../../common/misc.c ../../common/misc.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o misc.o ../../common/misc.c

sha1.o: ../../common/sha1.c ../../common/sha1.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o sha1.o ../../common/sha1.c

alphabet_phrase_generator.o: ../../password/alphabet_phrase_generator.c ../../password/alphabet_phrase_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o alphabet_phrase_generator.o ../../password/alphabet_phrase_generator.c

dictionary.o: ../../password/dictionary.c ../../password/dictionary.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o dictionary.o ../../password/dictionary.c

dictionary_word_generator.o: ../../password/dictionary_word_generator.c ../../password/dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o dictionary_word_generator.o ../../password/dictionary_word_generator.c

mixed_cased_dictionary_word_generator.o: ../../password/mixed_cased_dictionary_word_generator.c ../../password/mixed_cased_dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o mixed_cased_dictionary_word_generator.o ../../password/mixed_cased_dictionary_word_generator.c

password_generator.o: ../../password/password_generator.c ../../password/password_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o password_generator.o ../../password/password_generator.c

password_part_generator.o: ../../password/password_part_generator.c ../../password/password_part_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o password_part_generator.o ../../password/password_part_generator.c

rule_segment.o: ../../password/rule_segment.c ../../password/rule_segment.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o rule_segment.o ../../password/rule_segment.c

single_cased_dictionary_word_generator.o: ../../password/single_cased_dictionary_word_generator.c ../../password/single_cased_dictionary_word_generator.h 
	gcc -ansi -pedantic-errors -c -Wall -g -o single_cased_dictionary_word_generator.o ../../password/single_cased_dictionary_word_generator.c
