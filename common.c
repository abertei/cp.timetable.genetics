#include <string.h>
#include "common.h"
#include <ctype.h>
#include <stdlib.h>


/* char  *strndup(const char *str, size_t n); */
int factorial(int n)
{
  int result = 1;
  int i;
  for (i = 2 ; i <=n ; i++)
    result *= i ; 

  return result;
}

int matches(const char *s1,const  char *s2)
{
  int len1,len2;
  len1= strlen(s1); 
  len2 = strlen(s2); 

  if (len1 > len2) 
    return strncmp(s1,s2,len2); 
  return strncmp(s1,s2,len1);
}

/** 
    words do haskell;
 */
int words(const char *line,  char **argv)
{
  int i; /* ajuda a determinar a len de uma palavra*/

  /*    Uma palavra é definida como um  conjunto de caracteres
	entre espaço em branco. */

  /* encontra ínicio da palavra */
  while (isblank(*line))
    line++;

  if (*line == '\0')
    {
      *argv = NULL;
      return 0;
    }

  /* caminha até ao fim da palavra */
  for (i=0; !isblank(line[i]) && line[i] && line[i]!='\n' ; i++)
    continue;

  if (i == 0)
    return 0; 

  /* copia a palavra */
      argv[0] = malloc(sizeof(char) *i);
      memcpy(argv[0] , line , sizeof(char) *(i)); 
      argv[0][i] = '\0';

/*   strndup(line,sizeof(char) * i); */

  if (*argv == NULL)
    return -1;

  return (1+ words(line+i, argv+1));
}
