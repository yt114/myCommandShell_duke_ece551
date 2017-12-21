#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printLines(char ** data, size_t i){
  for(int j=0;j<i;j++){
    printf("%s",data[j]);
    free(data[j]);
  }
}


//This function is used to figure out the ordering
//of the strings in qsort.  You do not need
//to modify it.
int stringOrder(const void * vp1, const void * vp2) {
  const char * const * p1 = vp1;
  const char * const * p2 = vp2;
  return strcmp(*p1, *p2);
}
//This function will sort and print data (whose length is count).
void sortData(char ** data, size_t count) {
  qsort(data, count, sizeof(char *), stringOrder);
}


void read(FILE * f){
  char * line=NULL;
  char ** data =NULL;
  size_t sz;
  size_t i=0; //how many lines in data
  //mallocate a char pointer array (data) of which size is 1;

  while (getline(&line,&sz,f) >= 0){
    data=realloc(data,(i+1)*sizeof(*data));
    data[i]=line;
    line=NULL;
    i++;
  }
  
  free(line);
  //release line
  sortData(data,i);
  printLines(data,i);
  free(data);
}


int main(int argc, char ** argv) {
  
  //WRITE YOUR CODE HERE!
  if(argc==1){
   read(stdin);   
  }
  else{
    for(int i=1;i<argc;i++){
      FILE * f=fopen(argv[i],"r");
      if (f == NULL) {
	perror("Could not open file");
	return EXIT_FAILURE;
      }
      read(f);
      fclose(f);
    }
  }
  return EXIT_SUCCESS;
}
