#include "load_file.h"
char *load_file(const char *filename)
{
  int len;
  char *res;
  FILE *fp;

  fp = fopen(filename,"rb");
  if(fp==NULL) { perror(filename); return NULL; } 
  fseek(fp,0,SEEK_END);
  len = ftell(fp);
  if(len<2){ fclose(fp); return NULL; } 
  res = (char *)(malloc(len+1));
  if(res==NULL) { 
    fprintf(stderr,"%s: out of memory",filename); 
    return NULL; 
  }
  fseek(fp,0,SEEK_SET);
  if(fread(res,len,1,fp)<1){ 
    free(res); fclose(fp); perror(filename); 
    return NULL;
  }
  fclose(fp);
  res[len] = '\0';
  return res;
}

