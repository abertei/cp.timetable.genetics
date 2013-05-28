#include "chromosome.h"
#include "entities.h"
#include "hashing.h"
#include <stdlib.h>
#include "output.h" 
#include "genetics.h"
#include <stdio.h> 
#include <time.h> 
#include <string.h>
#include "main.h" 
#include "hash.h"
#include <mpi.h>
#define WORLD MPI_COMM_WORLD
extern int rank; 
Chromosome **chromosomes; 
unsigned chromosomes_size;   
int  * best_chromosomes; 
int *best_flags; 
int current_best_size=0; 
int current_generation=0; 
unsigned track_best;
int replace_by_generation; 
Chromosome *prototype; 
void add_to_best(int index); 
#include <signal.h>
void print_best(); 
int init_population(unsigned chromosomes_size_, int replace_by_generation_,unsigned track_best_, 
	Chromosome* prototype_
	){

  chromosomes_size = chromosomes_size_; 
  replace_by_generation = replace_by_generation_; 
  track_best = track_best_; 
  prototype = prototype_; 
  // Copy parameters passed; 

  // Initialization
  if( chromosomes_size < 2 )
    chromosomes_size = 2;
  
  if( track_best < 1 ){
    track_best = 1;
  }

  if( replace_by_generation < 1 ){
    replace_by_generation = 1;
  }

  else if( replace_by_generation > (chromosomes_size - track_best) ){
    replace_by_generation = chromosomes_size - track_best;
  }

  chromosomes = malloc(sizeof(Chromosome*) * chromosomes_size); 
  if (!chromosomes) quitp(); 
  best_flags = calloc(chromosomes_size,sizeof(int) ); 
  if (!best_flags) quitp();

  best_chromosomes = malloc(sizeof(int) * track_best); 
  if (!best_chromosomes) quitp(); 

  int i; 
  for ( i = 0 ; i < chromosomes_size ; i++){
    chromosomes[i] = NULL; 
    best_flags[i] = 0; 
  }  

  // initialize new population with chromosomes randomly built using prototype
  for (i = 0 ; i < chromosomes_size ; i++){
    chromosomes[i] = create_chromo_by_proto(prototype); 
    add_to_best(i); 
  }

  return 0; 
}

Chromosome **create_offsprings(){
  // produce offspring
  Chromosome **offsprings = malloc( sizeof(Chromosome*) * replace_by_generation); 
  if (!offsprings) return NULL; 

  int j; 
  for( j = 0; j < replace_by_generation; j++ ){
      // selects parent randomly
      Chromosome* p1 = chromosomes[ rand() % chromosomes_size ];
      Chromosome* p2 = chromosomes[ rand() % chromosomes_size ];
      offsprings[j] = crossover(p1,p2);
      mutation(offsprings[j]); 
    }
  return offsprings; 
}

static pthread_mutex_t lock_stop = PTHREAD_MUTEX_INITIALIZER; 
int to_stop = 0; 
#define USER_STOP 10
#define REACHED_CRITERIA 1 

void *check_stop_input(void *x){
  while(1){
    char input[100];
    scanf("%s", input); 
    if (strncmp(input,"stop", 100)==0){
      pthread_mutex_lock(&lock_stop); 
      to_stop = USER_STOP; 
      pthread_mutex_unlock(&lock_stop); 
    }
  }
}

float best_fit(){
  if (chromosomes[best_chromosomes[0]] != NULL) {
    return chromosomes[best_chromosomes[0]]->fitness; 
  }
  return 0.0; 
}
Chromosome *get_best(){
    return chromosomes[best_chromosomes[0]]; 
}

int stop(float fitness_stop){
  if (rank !=0) return 0; 
  if (best_fit() >= fitness_stop) return REACHED_CRITERIA; 
  int v ; 
  pthread_mutex_lock(&lock_stop); 
  v = to_stop;
  pthread_mutex_unlock(&lock_stop); 
  return v; 
}

void init_random(){
  int v = (int) time(NULL) + (rank+1)*1000; 
  unsigned vv = MurmurHash2(&v, (rank+1)*100, (rank+1)*1000 ); 
  /* switch(rank){ */
  /* case 0:  */
  /*   vv = 1404674042U;  */
  /*   break; */
  /* case 1: */
  /*   vv= 2749416652U;  */
  /*   break; */
  /* case 2: */
  /*   vv = 1851252467U;  */
  /*   break;  */
  /* case 3: */
  /*   vv = 2587662629U;  */
  /*   break;  */
  /* } */
  srand(vv); 
  printf("%d - %u\n", rank, vv); 
}

void init_processes(){
  if (rank == 0){
    pthread_t stop_thread; 
    pthread_create(&stop_thread, NULL,check_stop_input,NULL); 
  }
  init_random(); 
}


void add_received_cromos_to_population(Chromosome **new_cromos, int size){
  float max=0.0; 
  int i; 
  for (i = 0 ; i < size ; i++){
    if (chromosomes[best_chromosomes[current_best_size -1]]->fitness <= new_cromos[i]->fitness ){
      if (new_cromos[i]->fitness > max){
	max = new_cromos[i]->fitness; 
      }
      int not_best_i;
      do {
	// select chromosome for replacement randomly
	not_best_i = rand() % (int) chromosomes_size;
	// protect best chromosomes from replacement
      } while(best_flags[not_best_i]);
      release(chromosomes[not_best_i]);
      chromosomes[not_best_i] = clone_deserialized(new_cromos[i]);
      add_to_best(not_best_i);
    }
  }
  //printf("Process %d - add %f\n", rank,max); 
}

void slave_check_stop(void *data, int size, int bench){
  int sum = 0;
  int i; 
  unsigned char *array = data;
  for (i = 0; i < size; ++i) {
    sum +=  array[i];
    if (sum != 0 ) return ;
  }
  if (!bench)
    printf("(end) -  Process %d received end signal from master.\n", rank); 
  fflush(stdout);
  MPI_Finalize(); 
  exit(0); 
}

void sync_period(int master_stop, int bench){
  int  size_send_data;
  int ps;  MPI_Comm_size(WORLD, &ps);  
  void *data = serialize_cromos(track_best, best_chromosomes, chromosomes, &size_send_data); 
  if (master_stop){
    if (!bench)
      printf("master sending signal\n"); 
    //master process sends "sign" to others.. hey we stop .. go home
    memset(data, 0, size_send_data); 
  }
  unsigned char  *recv_buffer = malloc(size_send_data * ps);
  MPI_Allgather(data, (int) size_send_data, MPI_BYTE, recv_buffer, (int) size_send_data, MPI_BYTE, WORLD); 

  if (rank != 0){
    //slave processes for sure. Must listen to sign 
    slave_check_stop(recv_buffer, size_send_data,bench);
  }

  unsigned received_cromos; 
  unsigned process_index =0; 
  unsigned char *rcv_buffer_ptr = recv_buffer;  
  for (process_index= 0; process_index < ps ; process_index++){
    if (process_index != rank) { 
      Chromosome **new_cromos = deserialize_cromos(rcv_buffer_ptr, &received_cromos);
      add_received_cromos_to_population(new_cromos, received_cromos); 
      delete_new_cromos(new_cromos, received_cromos);
    }
   rcv_buffer_ptr += size_send_data; 
  }
  free(recv_buffer);
  free(data); 
  Chromosome *best = chromosomes[best_chromosomes[0]]; 
  if (!bench)
    printf("(update) - Process %d current generation - after exchange: %d. Best fitness :%f\n", rank, current_generation, best->fitness  );
}

void next_generation(){
  Chromosome **offsprings = create_offsprings(); 
  if (offsprings == NULL) {
    perror("");
    exit(-1);
  }
  // replace chromosomes of current operation with offspring
  int j; 

  for(j = 0; j < replace_by_generation; j++ ){
    int ci;
    do{
      // select chromosome for replacement randomly
      ci = rand() % (int) chromosomes_size; 
      // protect best chromosomes from replacement
    } while(best_flags[ci]);
    // replace chromosomes
    release(chromosomes[ci]); //MEM management 
    chromosomes[ci] = offsprings[ j ];
    // try to add new chromosomes in best chromosome group
    add_to_best( ci );
  }
  free(offsprings); 
  current_generation++;
}

void end_algorithm(int stop_value, int bench){
  if (rank == 0){
    //Master finished (do not care why). Send signal to everyone. 
    sync_period(1,bench); 
    if (stop_value == REACHED_CRITERIA){
      if (!bench)
	printf(" Reached criteria\n"); 
    }
    else{
      if (!bench)
	printf("User stop'd execution\n"); 
    }
  }
}

#include <sys/time.h>
#include <sys/resource.h>

double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

void *start(void *args){
  sargs *config = (sargs *) args; 
  const float stop_fit = config->stop_fitness; 
  const int interval = config->generation_sync_period; 
  const int ps = config->p; 
  const int bench = config->benchmarking;
  init_processes(); 
  if (rank == 0){
    printf("Executor is starting . Enter the string stop to terminate (Output generated then...)\n");
    //printf("(config) - Stop fitness : %f\n", config->stop_fitness); 
    //printf("(config) - Generation Sync Period : %d\n", config->generation_sync_period); 
  }

  prototype = init(config->crossover_points, 
		   config->mutation_size, 
		   config->crossover_probability,
		   config->mutation_probability); 
		   
  if (!prototype)  quitp(); 
  init_population(
		      config->population,
		      config->replace_by_generation,
		      config->track_best, prototype); 
  current_generation = 0;
  int stop_value; 
  double start_time; 

  if (bench && rank ==0){
    start_time =  get_time(); 
  }

  while(! (stop_value = stop(stop_fit))){  
    if ((current_generation % interval) == 0 ){
      if (!bench) printf("(update) - Process %d current generation: %d. Best fitness :%f\n", rank, current_generation, best_fit());
      if (ps > 1) {
	sync_period(0,bench);
      }
     }
    next_generation(); 
  }
  if (bench && rank ==0 ){
    printf("{%d,%f,", config->p, get_time() - start_time);
  }
  if (ps > 1){
    end_algorithm(stop_value,bench); 
  }
  if (bench){
    printf("%f,}, // %f\n",get_time() - start_time, best_fit());
  }
  //Only the master should get here. 
  if (!bench){
    printf("Going to print best chromo for file. Fitness: %f ; Generations : %d\n", best_fit(), current_generation); 
  }
  print(get_best(), "./out.html"); 
  MPI_Finalize(); 
  return NULL; 
}

void add_to_best(int index){
  // don't add if new chromosome hasn't fitness big enough for best chromosome group
  // or it is already in the group?
  if((current_best_size == track_best && 
      chromosomes[
		  best_chromosomes[current_best_size -1] 
		  ]->fitness 
      >= 
      chromosomes[ index ]->fitness) || 
     //TODO - not sure best_flags works. Trying to avoid adding the same cromo to the group. 
     best_flags[index]){
    return;
  }

  unsigned i = current_best_size; 

 // find place for new chromosome
  for( ; i > 0; i-- ){
      // group is not full?
    if( i < track_best ){
      // position of new chromosomes is found?
      if( chromosomes[ best_chromosomes[ i - 1 ] ]->fitness > chromosomes[index]->fitness){
	    break;
      }
      // move chromosomes to make room for new
      best_chromosomes[i] = best_chromosomes[i - 1];
    }
    else{
      // group is full remove worst chromosomes in the group
      best_flags[best_chromosomes[i - 1]] = 0;
    }
  }

  // store chromosome in best chromosome group
    best_chromosomes[ i ] = index; 
    best_flags[ index ] = 1;

  // increase current size if it has not reached the limit yet
  if( current_best_size < track_best )
    current_best_size++;
}

///////////////////////////////////////////////////////////////////

