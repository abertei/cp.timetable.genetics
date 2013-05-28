#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h> 
#include "array.h" 
#include <pthread.h>

void lock_foo(Varray *); 
void unlock_foo(Varray *); 

/**
 * WARNING NOT SAVE TO SHARE BEFORE INIT IS FINISHED
 */

void Varray_init(Varray **array){

  *array = (Varray*) malloc (sizeof(Varray));
  pthread_mutexattr_settype(&((*array)->attr), PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&((*array)->lock), &((*array)->attr));
  (*array)->memory = NULL;
  (*array)->allocated = 0;
  (*array)->used = 0;
  (*array)->index = -1;
}
 
void
Varray_push(Varray *array, void *data)
{
  lock_foo(array); 
   size_t toallocate;
   size_t size = sizeof(void*);
   if ((array->allocated - array->used) < size) {
      toallocate = array->allocated == 0 ? size : (array->allocated * 2);
      array->memory = realloc(array->memory, toallocate);
      array->allocated = toallocate;
   }
 
   array->memory[++array->index] = data;
   array->used = array->used + size;
   unlock_foo(array); 
}
 
int
Varray_length(Varray *array){
  lock_foo(array);
  int v = array->index +1; 
  unlock_foo(array); 
  return v;
}
 
void
Varray_clear(Varray *array)
{
  lock_foo(array); 
   int i;
   for(i = 0; i < Varray_length(array); i++)
   {
      array->memory[i] = NULL;
   }
   array->used = 0;
   array->index = -1;
   unlock_foo(array); 
}
 
void
Varray_free(Varray *array)
{
  lock_foo(array); 
   free(array->memory);
   unlock_foo(array); 
   free(array);
}
 
void*
Varray_get(Varray *array, int index)
{
  lock_foo(array);
  if (index < 0 || index > array->index){
    unlock_foo(array);
      return NULL;
  }
  void * ptr =array->memory[index];
  unlock_foo(array);
  return ptr;
}

void
Varray_insert(Varray *array, int index, void *data)
{
  lock_foo(array);
  if (index < 0 || index > array->index){
    unlock_foo(array); 
    return;
  }
  array->memory[index] = data;
  unlock_foo(array); 
}

void lock_foo(Varray *a){
  pthread_mutex_lock(&(a->lock)); 
}

void unlock_foo(Varray *a){
  pthread_mutex_unlock(&(a->lock)); 
}
