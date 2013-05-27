#ifndef __ENTITIES__H
#define __ENTITIES__H
#include <pthread.h>
#include "hashingSafe.h" 
#include "array.h"

void initialize_user (int userType, int fd[2], int users); 
extern pthread_t threads_array[1000]; 
int get_threads_created(); 
#define WRITE_END 1
#define READ_END 0

#define USER_STAFF_FD_INDEX  0
#define USER_EDUCATION_FD_INDEX 1
#define USER_SPACE_FD_INDEX 2

#define USER_STAFF_NAME "STAFF"
#define USER_EDUCATION_NAME "EDUCATION"
#define USER_SPACE_NAME "SPACES"

#define USER_STAFF 1
#define USER_EDUCATION 2
#define USER_SPACE 3

typedef struct teacher{
  char *id;
  char *name; 
}Teacher;


// A gang is a set of students with a set of classes. We do not care about the students;
typedef struct gang{
  char *id;
  char *name; 
  unsigned size;  
}StudentsGroups;

typedef enum {LAB, NOT_LAB } RoomType; 

void finito(); 
typedef struct room{
  char *id; 
  char *idd; 
  unsigned seats; 
  RoomType type; 
}Room;

typedef struct curse{
  char *id; 
  char *name; 
}Course; 

// Entities .
typedef struct class{
  int class_id; 
  char *teacher_id; 
  char *course_id; 
  RoomType type; 
  Varray *students; 
  unsigned seats; 
  unsigned periods;
}Class;

typedef struct global_info {
   Varray *classes; 
   HashTableS * rooms; 
   HashTableS *students_groups; 
   HashTableS * teachers; 
  HashTableS *courses; 
}GlobalInfo;

extern GlobalInfo global; 
int group_overlap(Class *c1, Class *c2); 
void init_entities(); 
#define USER_N 3
#endif
