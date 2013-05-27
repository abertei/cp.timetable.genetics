#ifndef ARRAY_H 
#define ARRAY_H 

#include <pthread.h>
#include <unistd.h>

typedef struct varray_t
{
  pthread_mutex_t lock; 
  pthread_mutexattr_t    attr; //TODO not sure if this needs to be here. 
  void **memory;
  size_t allocated;
  size_t used;
  int index;
}Varray; 

void Varray_init(Varray **array);
void Varray_push(Varray *array, void *data); 
int Varray_length(Varray *array);
void Varray_clear(Varray *array);
void Varray_free(Varray *array);
void* Varray_get(Varray *array, int index);
void Varray_insert(Varray *array, int index, void *data);

#endif 
