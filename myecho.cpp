/* myecho.c */
/*not my echo
  got it from http://man7.org/linux/man-pages/man2/execve.2.html
  as a test.
*/
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
  int j;
  for (j = 0; j < argc; j++)
    printf("argv[%d]: %s\n", j, argv[j]);
  exit(EXIT_SUCCESS);
}
