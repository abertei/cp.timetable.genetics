#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h> 

#include "hashingSafe.h"
#include "hash.h"
#include "util.h"
#include "entities.h"


#define SEED_HASH 1

static pthread_mutex_t lock_v = PTHREAD_MUTEX_INITIALIZER; 
static int threads_created = 0; 
int inc_id(){
  pthread_mutex_lock(&lock_v); 
  int i = threads_created++; 
  pthread_mutex_unlock(&lock_v);
  return i; 
}

int get_threads_created(){
  pthread_mutex_lock(&lock_v); 
  int i = threads_created;
  pthread_mutex_unlock(&lock_v);
  return i; 
}

//TODO - number of students; 
GlobalInfo global; 
pthread_t  threads_array[1000]; 
int str_cmp(const void *v1, const void *v2){
  char *v1t = (char *) v1;
  char *v2t = (char *) v2;
  return strcmp(v1t, v2t);
}

unsigned str_hash(const void *k, unsigned m){
  //unsigned val = (unsigned) MurmurHash2(k, strlen(k), SEED_HASH)  % m ;
  const char *ptr = k; 
  unsigned val=0; 
  while (*ptr){
    val +=*ptr++; 
  }
  return val % m ; 
}

void foreach_teacher_to_string(void *elem_key, void *elem_data, void *foo_data){
  Teacher *s = (Teacher *) elem_data; 
  printf("Teacher : %s - %s\n", (char *) elem_key, s->name); 
}

void course_to_string(void *elem_key, void *elem_data, void *foo_data){
  Course *c = (Course *) elem_data; 
  printf("Course : %s - %s\n", (char *) c->id, c->name); 
}


void foreach_student_group_to_string(void *elem_key, void *elem_data, void *foo_data){
  StudentsGroups *s= (StudentsGroups *) elem_data; 
  printf("%s -  %s - %d students - size(key) : %d\n", (char *) elem_key, s->name, s->size,(int)  strlen(s->id));
  
}

void foreach_room_to_string(void *elem_key, void *elem_data, void *foo_data){
  Room *r = (Room *) elem_data; 
  printf("Room : %s/%s - %d seats , type: %s\n",  r->id,r->idd, r->seats , r->type == LAB ? "LAB" : "LECTURE" ); 
}

// A gang is a set of students with a set of classes. We do not care about the students;

void init_entities(){
  global.rooms = initHashS(str_cmp, str_hash);
  global.students_groups = initHashS(str_cmp,str_hash); 
  global.teachers = initHashS(str_cmp, str_hash); 
  global.courses = initHashS(str_cmp, str_hash); 
  Varray_init(&(global.classes)); 
}

void * init_staff(void *arg);
void * init_edu(void *arg);
void * init_space(void *arg);

struct tuple {
  int fd; 
  int users;
};


void initialize_user (int userType, int fd[2], int users){
  int result;
  
  struct tuple *tuple = malloc(sizeof(struct tuple)); 
  if (!tuple) {
    perror("Memory :"); 
    exit(-1); 
  }
  
  tuple->fd = fd[READ_END]; 
  tuple->users = users; 
  int id = inc_id(); 
  

  switch(userType){
  case USER_STAFF:
    result = pthread_create(&(threads_array[id]), NULL, init_staff, tuple);
    break;
  case USER_EDUCATION:
    result = pthread_create(&(threads_array[id]), NULL, init_edu,tuple);
    break;
  case USER_SPACE:
    result =pthread_create(&(threads_array[id]), NULL, init_space, tuple);
    break; 
  default : 
    fprintf(stderr, "(Warning) Non recursive patterns in initialize_user...\n"); 
    return; 
  }
  if (result == 0){
    return ; 
  }
  else{
    perror("Initialization of threads failed :"); 
    exit(-1); 
  }
  
}

//TODO - perror . is errno shared?
void * error_(const char * s){
  perror(s);
  return NULL;
}

void * error_lock(const char *s, pthread_mutex_t * lock){
  perror(s);
  if (!pthread_mutex_unlock(lock)) perror("Unlocking : ");
  return NULL;
}

typedef struct runner_arg{
  FILE *fd;
  pthread_mutex_t * read_fd_lock;
}Runner_Arg;


void *staff_runner(void *arg);

void init_threads(Runner_Arg *args, int n_threads, void *(*foo)(void*)){
  int i; 
  for (i = 0 ; i < n_threads ; i++){
    int k = inc_id();

    int result = pthread_create(&(threads_array[k]), NULL, foo,args);
    if (result !=0){
      perror("Could not initialize threads: "); 
      exit(-1);
    }
  }
}
void * init_staff(void *arg){
  struct tuple *tuple = (struct tuple *) arg; 
  //init  hteachers
  Runner_Arg *args = malloc(sizeof(Runner_Arg)); 
  if (!args){
    perror("MEM :"); 
    exit(-1); 
  }
  pthread_mutex_t *read_fd = malloc(sizeof(pthread_mutex_t));
  if (!read_fd){
    perror("MEM :"); 
    exit(-1); 
  }

  args->fd =fdopen(tuple->fd, "r");
  if (args->fd == NULL) return error_("Could not create file from  pipe reading end\n");
  args->read_fd_lock = read_fd;
  if (pthread_mutex_init(read_fd, NULL)!=0) return error_("Initializing mutex failed: \n");
  // init n- 1 threads...
  init_threads(args, tuple->users -1, staff_runner); 
  //because we run a thread already:
  return staff_runner(args);
}

char *read_line_from_input(FILE *f, int *is_fd_close){
  char *line = NULL;
  size_t linecap = 0;
  *is_fd_close = 0; 
  ssize_t linelen = getline(&line, &linecap, f);
  if (linelen == -1){
    if (feof(f)){
      *is_fd_close = 1; 

    }
  }
  if (linelen > 0 ) return line ;
  return NULL;
}
  
int read_teacher(Runner_Arg *rarg , char **arguments, int read){
      Teacher *new_teacher = malloc(sizeof(Teacher));
      if (!new_teacher) return -1 ; 
      
      new_teacher->id = strdup(arguments[3]);
      if (!new_teacher->id) return -1; 

      new_teacher->name = strdup(arguments[4]); 
      if (!new_teacher->name) return -1; 
      if (add2HashS(global.teachers, new_teacher->id, new_teacher) != 0) return -1;
      //printf("Adding %s\n", new_teacher->teacher_id);
  return 0; 
}

int read_course(Runner_Arg *rarg, char **arguments, int read){
  Course *c = malloc(sizeof(Course)); 
  if (!c ) return -1; 

  c->id = strdup(arguments[3]); 
  if (!c->id) return -1; 

  c->name = strdup(arguments[4]); 
  if (!c->id) return -1; 

  if (add2HashS(global.courses, c->id, c) != 0) return -1;
  return 0; 
}

//   return "add Education group T1 TurmaMEI 60"; 
int read_turma(Runner_Arg *rarg , char **arguments, int read){
 StudentsGroups *new_gang = malloc(sizeof(StudentsGroups));
 if (!new_gang) return -1; 
      
 new_gang->id = strdup(arguments[3]);
 if (!new_gang->id) return -1; 
 new_gang->name = strdup(arguments[4]);
 if (!new_gang->name) return -1; 

 if (sscanf(arguments[5],"%d", &(new_gang->size)) != 1 ){
   fprintf(stderr, "%s is not a valid size", arguments[5]); 
   return 1; 
 }
 add2HashS(global.students_groups, new_gang->id, new_gang); 
 return 0; 
}

int inc_class(){
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
  pthread_mutex_lock(&lock); 
  static int v = 0; 
  int i = v++; 
  pthread_mutex_unlock(&lock);
  return i; 
}

char * inc_room(){
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
  char *m = malloc(sizeof(char) * 32); 
  if (!m) return NULL; 
  pthread_mutex_lock(&lock); 
  static int v = 0;
  int i = v++; 
  pthread_mutex_unlock(&lock); 
  sprintf(m, "%d", i); 
  return m;
}
static int max_seats_lab =0;
static int max_seats_nlab =0;

/*             0    1         2   3  4 5     6    7  8 */
/*    return "add Education class 2 C1 2 LECTURE T1 T2";  */
int read_room(Runner_Arg *rarg , char **arguments, int read){
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 

  Room *r = malloc(sizeof(Room)); 
  if (!r ) return -1; 
  r->id = inc_room(); 
  if (!r->id ) return -1; 
  r->idd = strdup(arguments[3]);
  sscanf(arguments[5], "%d", &(r->seats));
  r->type = strcasecmp(arguments[4], "lab")  == 0 ? LAB : NOT_LAB ; 


  pthread_mutex_lock(&lock); 
  switch (r->type){
  case LAB: 
    if (r->seats > max_seats_lab) max_seats_lab = r->seats; 
    break; 
  case NOT_LAB: 
    if (r->seats > max_seats_nlab) max_seats_nlab = r->seats; 
    break; 
  }
  pthread_mutex_unlock(&lock); 

  add2HashS(global.rooms, r->id, r);
  return 0; 
}

int read_class(Runner_Arg *rarg , char **arguments, int read){
  Class *c = malloc(sizeof(Class)); 
  if (!c) return -1; 

  c->class_id = inc_class();
  

  c->teacher_id = strdup(arguments[3]);
  if (!c->teacher_id) return -1; 

  c->course_id = strdup(arguments[4]); 
  if (!c->course_id) return -1; 

  sscanf(arguments[5], "%d", &c->periods); 
  c->seats = 0; 
  c->type = LAB; 
  if (strcasecmp(arguments[6], "lecture") == 0){
    c->type = NOT_LAB; 
  }

      /* if (classes > 0){ */
      /* 	new_teacher->classes = malloc(sizeof(char *) * classes); */
      /* 	if (!new_teacher->classes) return -1; */
      /* 	int i,j; */
      /* 	for (j=0, i = 4 ; arguments[i] != NULL  ; i++, j++){ */
      /* 	  new_teacher->classes[j] = strdup(arguments[i]); */
      /* 	  if (!new_teacher->classes[j]) return -1; */
      /* 	} */
      //new_teacher->classes[j] = NULL; 
	//Add teacher :
  int i; 
  Varray_init(&(c->students));
  for (i = 7 ; i < (read -1) ; i++){
    Varray_push(c->students, strdup(arguments[i])); 
  }
  
  Varray_push(global.classes, c); 
  return 0; 

//TODO - must have seats filled by checking all classes   
}

void *staff_runner(void *arg){
  Runner_Arg *rarg  = (Runner_Arg *) arg;
  int read;
  int fd_is_close; 

  while(1){
    // lock ; read file;
    int locked = pthread_mutex_lock(rarg->read_fd_lock);
      if (locked!=0) return error_lock("Could not lock on file : ", rarg->read_fd_lock);
      char *line = read_line_from_input(rarg->fd, &fd_is_close);
      if (!line){
	if (fd_is_close){
	  pthread_mutex_unlock(rarg->read_fd_lock); 
	  //TODO - next step. 
	  return NULL; 
	}
	return error_lock("Strange... could not allocate memory? : " , rarg->read_fd_lock);
      }
      if (pthread_mutex_unlock(rarg->read_fd_lock) != 0){
	perror("Unlock :");
      }
     char **arguments = split(line, &read);
    // Check arguments is clean:
    if (arguments == NULL )  return error_("Could not allocate in split of input line: ");
    if (read != 6 ){
      fprintf(stderr, "Ignoring input line: %s | %d.", line, read);
      freeArray(arguments);
      continue;
    }
    //Create new Staff entity.
    if (strcasecmp(arguments[2], "teacher") == 0){
      if (read_teacher(arg, arguments, read) != 0){
	return NULL; 
      }
    }
    else{
      fprintf(stderr, "EntityType  %s is not valid\n", arguments[2]);
    }
    freeArray(arguments); 
  }
}

void *edu_runner(void *arg);
void * init_edu(void *arg){
  struct tuple *tuple = (struct tuple *) arg; 
  //init  hteachers
  //init  hteachers
  Runner_Arg *args = malloc(sizeof(Runner_Arg)); 
  if (!args){
    perror("MEM :"); 
    exit(-1); 
  }
  pthread_mutex_t *read_fd = malloc(sizeof(pthread_mutex_t));
  if (!read_fd) {
    perror("MEM:"); 
    exit(-1); 
  }

  args->fd =fdopen(tuple->fd, "r");
  if (args->fd == NULL) return error_("Could not create file from  pipe reading end");
  args->read_fd_lock = read_fd;
  if (pthread_mutex_init(read_fd, NULL)!=0) return error_("Initializing mutex failed: ");
    // init n- 1 threads...
  init_threads(args, tuple->users -1, edu_runner); 
  //because we run a thread already:
  return edu_runner(args);
}

void *edu_runner(void *arg){
  Runner_Arg *rarg  = (Runner_Arg *) arg;
  int read;
  int fd_is_close; 
  while(1){
    // lock ; read file;
    int locked = pthread_mutex_lock(rarg->read_fd_lock);
      if (locked!=0) return error_lock("Could not lock on file : ", rarg->read_fd_lock);
      char *line = read_line_from_input(rarg->fd, &fd_is_close);
      if (!line){
	if (fd_is_close){
	  pthread_mutex_unlock(rarg->read_fd_lock); 
	  //TODO - next step. 
	  return NULL; 
	}
	return error_lock("Strange... could not allocate memory? : " , rarg->read_fd_lock);
      }
      if (pthread_mutex_unlock(rarg->read_fd_lock) != 0){
	perror("Unlock :");
      }

    char **arguments = split(line, &read);
    // Check arguments is clean:
    if (arguments == NULL )  return error_("Could not allocate in split of input line: ");
    
    if (read < 5 ){
      fprintf(stderr, "Ignoring input line: %s .", line);
      freeArray(arguments);
      continue;
    }

    //    return "add Education course C2 MI";
    if (strcasecmp(arguments[2], "course")== 0) {
      if (read_course(rarg, arguments, read) != 0){
	return error_(""); 
      }
      else{
	freeArray(arguments);
	continue; 
      }
    }
    //   return "add Education group T1 TurmaMEI 60"; 
    //Create new Staff entity.
    if (strcasecmp(arguments[2], "group") == 0){
      if (read_turma(rarg, arguments, read) != 0){
	return error_("Reading arguments group: "); 
      }
      else{
	freeArray(arguments); 
	continue; 
      }
    }

    //return "add Education class 1 C1 4 LAB T1";  
    if (strcasecmp(arguments[2], "class") ==0){
      int ret;
      if ((ret = read_class(rarg, arguments, read)) != 0){
	return error_("Reading arguments class: "); 
      }
      else {
	freeArray(arguments);
	continue; 
      }
    }
    freeArray(arguments); 
    //We got here. So we do not recognize this shit
    fprintf(stderr, "EntityType  %s is not valid\n", arguments[2]);
  }
}

void *space_runner(void *arg);
void * init_space(void *arg){
  struct tuple *tuple = (struct tuple *) arg; 
  //init  hteachers
  Runner_Arg *args = malloc(sizeof(Runner_Arg)); 
  if (!args){
    perror("MEM :"); 
    exit(-1); 
  }
  pthread_mutex_t *read_fd = malloc(sizeof(pthread_mutex_t ));
  if (!read_fd){
    perror("MEM:");
    exit(-1);
  }

  args->fd =fdopen(tuple->fd, "r");
  if (args->fd == NULL) return error_("Could not create file from  pipe reading end");
  args->read_fd_lock = read_fd;
  if (pthread_mutex_init(read_fd, NULL)!=0) return error_("Initializing mutex failed: ");
 init_threads(args, tuple->users -1, space_runner); 
  //this thread runs staff_runner . new Staffs runners are launched by that method.
  return space_runner(args);
}

void *space_runner(void  *arg){
  Runner_Arg *rarg  = (Runner_Arg *) arg;
  int read;
  int fd_is_close; 
  while(1){
    // lock ; read file;
    int locked = pthread_mutex_lock(rarg->read_fd_lock);
      if (locked!=0) return error_lock("Could not lock on file : ", rarg->read_fd_lock);
      char *line = read_line_from_input(rarg->fd, &fd_is_close);
      if (!line){
	if (fd_is_close){
	  pthread_mutex_unlock(rarg->read_fd_lock); 
	  //TODO - next step. 
	  return NULL; 
	}
	return error_lock("Strange... could not allocate memory? : " , rarg->read_fd_lock);
      }
      if (pthread_mutex_unlock(rarg->read_fd_lock) != 0){
	perror("Unlock :");
      }

     char **arguments = split(line, &read);
    // Check arguments is clean:
    if (arguments == NULL )  return error_("Could not allocate in split of input line: ");
    
    if (read != 7 ){
      fprintf(stderr, "Ignoring input line (!= %d): %s - ",  read, line);
      freeArray(arguments);
      continue;
    }

    //    return "add Education course C2 MI";
    if (strcasecmp(arguments[2], "room")== 0){
      if (read_room(rarg, arguments, read) != 0){
	return error_(""); 
      }
      else{
	freeArray(arguments); 
	continue; 
      }
    }
    freeArray(arguments); 
    //We got here. So we do not recognize this shit
    fprintf(stderr, "EntityType  %s is not valid\n", arguments[2]);
  }

}

int group_overlap(Class *c1, Class *c2){
  int i1,i2; 
  char *s1,*s2; 
  for( i1=0 , s1 = Varray_get(c1->students,i1) ;  i1 < Varray_length(c1->students) ;  i1++, s1 = Varray_get(c1->students, i1) ){
    for(i2=0, s2 = Varray_get(c2->students,i2) ; i2 < Varray_length(c2->students); i2++, s2 = Varray_get(c2->students, i2)){
      if (strcmp(s1,s2) ==0){
	return 1;
      }
    }
  }
 return 0;
}

void finito(){
  //  mapHashS(global.rooms,  NULL , foreach_room_to_string ); 
  //  mapHashS(global.teachers,  NULL , foreach_teacher_to_string); 
  //  mapHashS(global.courses,  NULL ,course_to_string ); 
  //  mapHashS(global.students_groups,  NULL , foreach_student_group_to_string ); 
  int ind;
  
  for (ind =0 ; ind < Varray_length(global.classes) ; ind++){
    int ss = 0; 
    Class *c = (Class *) Varray_get( global.classes, ind);
    for (ss = 0; ss < Varray_length(c->students) ; ss++){
      char *students = Varray_get(c->students, ss); 
      StudentsGroups *g = searchHashS(global.students_groups, students); 
      if (g != NULL){
	c->seats += g->size; 
      }
      else {
	fprintf(stderr, "%s is not a valid students groups\n", students); 
      }
    }
      int max_seats;
      switch(c->type){
      case LAB: 
	max_seats = max_seats_lab;
	break; 
      case NOT_LAB : 
	max_seats = max_seats_nlab;
	break; 
      }
      if (c->seats > max_seats){
	fprintf(stdout, "%d (%d) can not be allocatted in any room (%d)\n", c->class_id, c->seats, max_seats); 
      }
    }
    /*    printf("Class : id : %d | teacher: %s | course : %s | %s -  %d periods %d seats (" ,
	   c->class_id, 
	   c->teacher_id , 
	   c->course_id , 
	   c->type == LAB ? "LAB" : "LECTURE" ,  c->periods, c->seats  );   
    
    for (ss = 0; ss < Varray_length(c->students) ; ss++){
      printf(" %s ",(char *) Varray_get(c->students, ss)); 
    } 
    printf(")\n"); 
    */
}

// we must initialize the datastructures for each user to maintain.
