
#ifndef _LOAD_FILE_H_
#define _LOAD_FILE_H_
/******************************************************************/
/*#ifndef _LOAD_FILE_H_ and then #define _LOAD_FILE_H_            */
/* is a famous trick to make sure Header will be parsed only once.*/
/* We will learn about it in course later .                       */
/******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

char *load_file(const char *filename);
/**********************************************************************************/
/*Loads a file completely into huge string (terminated by '\0').                  */
/*Memory is allocated by this function, so use free (see below) when not needed.  */
/*File is open, read completely into buffer, '\0' is put and then file is closed. */
/*If file cannot be read or not exist then informative error is print into stderr.*/
/*   and NULL is returned.    Notice: do not free NULL.                           */
/**********************************************************************************/
/* Example: char *buf = load_file("mantra.txt"); if(buff==NULL) return 1;         */
/*          printf ("\nMantra is: '%s'\n",buf);  free(buf);                       */
/**********************************************************************************/

#endif  /* #ifndef _LOAD_FILE_H_*/

