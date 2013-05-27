#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h> 
//Free char ** arrays
#define MAX_SIZE_OF_INPUT 100000000
void freeArray(char **splitted){
  char **ptr = splitted;
  while (*ptr != NULL){
    free(*ptr++);
  }
  
}
void *augment_struct (void *data, int *size, int *current); 

char *find_substring(char *string, char **save_end_ptr); 

//Split function based on a separator; 
char **split( char *line, int *count){
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
  if (pthread_mutex_lock(&lock) !=0){
    perror("lock : "); 
  }

  char **splitted = NULL; 
  int size=0,current =0; 


  char *input= strndup(line, MAX_SIZE_OF_INPUT); 
  if (!input ) return NULL; 

  char *str; 
  
  while (1){
    str = find_substring(input, &input); 
    if (str != NULL){
      splitted = augment_struct(splitted, &size, &current); 
      if (splitted== NULL) return NULL; 
      splitted[current] =  str; 
      current++; 
    }
    else break; 
  }

  /* while ((str = strsep(&input, " \t\f\n\r\t\v")) != NULL){ */
  /*   if (!isspace(*str) && *str != '\0'){ */
  /*     splitted = augment_struct(splitted, &size, &current);  */
  /*     if (!splitted) return NULL;  */
  /*     splitted[current] =  strndup(str, MAX_SIZE_OF_INPUT);  */
  /*     //char *str2 = splitted[current] ;  */
  /*     current++;  */
  /*     /\* while (*str2){ *\/ */
  /*     /\* 	*str2 = tolower(*str2); *\/ */
  /*     /\* 	str2++;  *\/ */
  /*     /\* } *\/ */
  /*   } */
  /* } */

  splitted = augment_struct(splitted, &size, &current); 
  if (!splitted) return NULL; 
  splitted[current++] = NULL; 
  if (count != NULL) {
    *count  = current; 
  }
  pthread_mutex_unlock(&lock); 
  return splitted; 

}

char *find_substring(char *string, char **save_end_ptr){
  char *str_begin = string; 
  while (*str_begin != '\0' && isspace(*str_begin) ) str_begin++; 
  char *str_end = str_begin; 
  while (*str_end != '\0' && !isspace(*str_end)) str_end++; 
  if (str_end != str_begin){
    char * new_string = malloc(sizeof(char) * (str_end - str_begin) + 1); 
    strncpy(new_string, str_begin, (str_end - str_begin)); 
    new_string[(str_end -str_begin)] = '\0'; 
    *save_end_ptr = str_end; 
    return new_string; 
  }
  else return NULL; 
}

void *augment_struct (void *data, int *size, int *current){
  if ((*size)  == (*current )){
  	//Augment data structure.
    *size =   *size == 0 ? 1 : *size * 2 ;
    return realloc(data, sizeof(char *) * (*size));
    }
  return data;

}
