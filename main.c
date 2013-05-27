#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <ctype.h> 
#include <unistd.h>
#include "util.h" 
#include "entities.h"
#include "hashing.h" 
#include "genetics.h"
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <mpi.h>
#define WORLD MPI_COMM_WORLD
void broadcast_file(char *fpath); 
void start_mpi(int argc, char **argv); 
void receive_file(int); 
FILE *open_my_file(const char *mode); 
int pipe_fd[2];

void quitp(){
  perror("Quiting due to unexpected error: ");
  exit(-1); 
}

int user_threads_file_descriptors[USER_N][2]; 
#define MAX_SIZE_OF_INPUT 10000000
void parse_command(char * input);
char * read_line_from_file();
void parse_command(char * input); 

char * read_line_from_file(FILE *fp){
  char *line = NULL;
  ssize_t readd; 
  size_t len = 0; 
  
  if ((readd = getline(&line, &len, fp)) != -1){
    line[readd-1] = '\0'; 
    return line; 
  }
  return NULL; 
}

void parsing(){
  char *input; 
  FILE *fp = open_my_file("r");   
  while (!feof(fp)){
      while ( (input = read_line_from_file(fp)) != NULL){
	parse_command(input);
	free(input); 
      }
    }
}

/**
 * This will be used to comunicate with the user threads 
 * that are responsible for updating the graph. Indices should 
 * be used as defined in timetable_users.h  (USER_THREAD_XXXXXX) 
 */

void parse_command(char * input){
  // Decide wich pipe to send the input. 
  char **words = split(input, NULL); 
  //TODO - i think words is never NULL.. 
  if (words != NULL){
    int fd; 
    //TODO - take this strcmp out of here. 
    if (strcasecmp(words[0], "ADD")==0){ 
	if (strcasecmp(words[1],USER_STAFF_NAME)==0){
	  fd = user_threads_file_descriptors[USER_STAFF_FD_INDEX][WRITE_END];
	}
	else if (strcasecmp(words[1], USER_EDUCATION_NAME) ==0){
	  fd = user_threads_file_descriptors[USER_EDUCATION_FD_INDEX][WRITE_END]; 
	}
	else if (strcasecmp(words[1], USER_SPACE_NAME)==0){
	  fd = user_threads_file_descriptors[USER_SPACE_FD_INDEX][WRITE_END]; 
	}
	else{
	  fprintf(stderr, "Command \"%s\" not recognized as valid due to unrecognized option :  \"%s\".\n", input, words[1]); 
	  fd = -1;
	}
	if (fd != -1){
	  //Write the command to the fd
	  int result ;
	  if ((result =write (fd, input, sizeof(char) * (strlen(input))) ) <0 ){
	    perror("writing : "); 
	  }
	  //TODO - really needed? 
	  write (fd, "\n" , sizeof(char) ); 
	}
      }
    freeArray(words);
    }
}

int staff_n=1, edu_n =1, space_n =1;
void initialize_threads(){
  //Create array of threads; 
  //Initialize pipes
  int i; 
  for (i =0 ; i < USER_N ; i++){
    if (pipe(user_threads_file_descriptors[i]) != 0){
      perror("Creating pipes error: "); 
      exit(-1); 
    }
  }

  //start threads. 
  initialize_user(USER_STAFF, user_threads_file_descriptors[USER_STAFF_FD_INDEX], staff_n);
  initialize_user(USER_EDUCATION, user_threads_file_descriptors[USER_EDUCATION_FD_INDEX], edu_n);
  initialize_user(USER_SPACE, user_threads_file_descriptors[USER_SPACE_FD_INDEX], space_n);
  //close the  end  of the file descriptor that we are not concerned. We will be writing to the pipe. Threads will be reading. 
  //TODO - threads must close the write side. 
}

void close_descriptors(){
 int i; 
 for (i =0 ; i < USER_N ; i++){
    if (close(user_threads_file_descriptors[i][WRITE_END]) != 0){
      perror("Closing pipes  error: "); 
      exit(-1); 
    }
  }
}

int parse(const char *needle, int argc, char **argv){
  while (*argv != NULL){
    if (strncasecmp(*argv++, needle, strlen(needle)) == 0){
      //found needle;
      if (*argv != NULL){
	int v; 
	if (sscanf(*argv,"%d", &v) == 1){
	  return v; 
	}
	else {
	  fprintf(stderr, "%s is not a valid number\n", *argv); 
	  exit(0); 
	}
      }
      else {
	fprintf(stderr, "Expected to found a number after %s\n", *(--argv)); 
	exit(0); 
      }
    }
  }
  return 1; 
}

char * get_path (int argc, char **argv){
  while(*argv != NULL){
    if (strncasecmp(*argv++, "--input",sizeof("--input")) == 0){
      if (*argv != NULL) {
	return *argv; 
      }
      else{
	fprintf(stderr, "Not valid input\n"); 
	exit(0); 
      }
    }
 }
  return NULL; 
}

sargs read_alg_config(argc, argv){
  sargs args; 
  args.stop_fitness = 0.8;
  args.generation_sync_period =100; 
  return args; 
}

int main(int argc , char **argv){
  if ((argc != 3 && argc != 4 && argc != 6 && argc != 8  && argc == 4) || get_path(argc,argv) == NULL){
    printf("Usage:\n\t%s [--staff N] [--edu N] [--space N]\n\tYou can specify [] arguments in any order\n", argv[0]); 
    printf("\tAlso %s --normal runs with one thread for every user\n", argv[0]);
    exit(1); 
  }

  if (argc == 3 ){
    ;
  }
  else {
    staff_n = parse ("--staff", argc, argv);
    edu_n = parse ("--edu", argc, argv);
    space_n = parse ("--space", argc, argv);
  }

  printf ("Starting %d staff users, %d edu users and %d space users\n", staff_n, edu_n, space_n); 

  sleep(3); 
  start_mpi(argc, argv);
  init_entities(); 
  initialize_threads(); 
  parsing(); 
  close_descriptors(); 

  int i =0; 
  for (i = 0 ; i < get_threads_created() ; i++){
    pthread_t t = threads_array[i]; 
    //printf("Going to wait for thread %d\n", i); 
    if (pthread_join(t, NULL)!=0){
      perror("Waiting for threads: "); 
    } 
  }

  finito();
  pthread_t exec; 
  signal(SIGINT, SIG_IGN); 
  sargs alg_config = read_alg_config(argc,argv); 
  int result = pthread_create(&exec, NULL, start, &alg_config);
  if (result != 0){
    perror("Could not create executor :"); 
    exit(-1);
  }
  pthread_join(exec, NULL); 
  return 0; 
}
int numprocs, rank, namelen;

void start_mpi(int argc, char **argv){
   char processor_name[MPI_MAX_PROCESSOR_NAME];

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Get_processor_name(processor_name, &namelen);

   if ( rank == 0 ) {
      printf( "[%02d/%02d %s]: I am the master\n", rank, numprocs, processor_name );
      broadcast_file(get_path(argc, argv));
   } else {
      printf( "[%02d/%02d %s]: I am a servant\n", rank, numprocs, processor_name );
      receive_file(rank); 
   }

}

void broadcast_file(char *fpath){
  FILE * fp = fopen(fpath, "r"); 
  if (!fp) quitp(); 
  fseek(fp, 0L, SEEK_END); 
  long sz = ftell(fp); 
  rewind(fp); 
  MPI_Bcast(&sz , 1, MPI_LONG, 0, WORLD); 
  
  char * buffer =calloc(1, sz); 
  if (!buffer) quitp(); 
  
  if (1!=fread(buffer, sz, 1, fp))
    quitp();
  fclose(fp); 

  fp = fopen("./input0", "w+");
  if (!fp) {
    quitp();
  }
  if (fwrite(buffer, 1, sz, fp) < sz) {
    quitp(); 
  }
  fflush(fp); 
  fclose(fp); 

  fflush(stdout); 
  MPI_Bcast(buffer, sz, MPI_CHAR, 0, WORLD); 
}

FILE *open_my_file(const char *mode){
  char file[100]; 
  sprintf(file,"./input%d", rank); 
  FILE *fp = fopen(file, mode); 
  if (!fp) quitp(); 
  return fp; 
}

void receive_file(int rank){
  FILE *fp = open_my_file("w+"); 
  long sz;
  MPI_Bcast(&sz, 1 , MPI_LONG, 0 , WORLD); 
  
  char *buffer = calloc(sizeof(char), sz); 
  if (!buffer) quitp(); 

  MPI_Bcast(buffer, sz, MPI_CHAR, 0, WORLD); 
  if (fwrite(buffer, 1, sz, fp) < sz) 
    quitp(); 
  fclose(fp);
}

/// helper functions. 
