#include "chromosome.h"
#include "entities.h"
#include "hashing.h"
#include <stdlib.h>
#include <stdio.h> 
#include <time.h> 
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
struct sigaction setup_action;
sigset_t block_mask;

void intHandler(int dummy) {
  stop =1;
}

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
  if (!chromosomes) return -1; 
  best_flags = calloc(chromosomes_size,sizeof(int) ); 
  if (!best_flags) return -1; 

  best_chromosomes = malloc(sizeof(int) * track_best); 
  if (!best_chromosomes) return -1; 

  int i; 
  for ( i = 0 ; i < chromosomes_size ; i++){
    chromosomes[i] = NULL; 
    best_flags[i] = 0; 
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
double fitness_stop;

void *check_stop_input_from_master(void *x){
  MPI_Barrier(WORLD); 
  pthread_mutex_lock(&lock_stop); 
  to_stop = 1; 
  pthread_mutex_unlock(&lock_stop); 
}


void *check_stop_input(void *x){
  while(1){
    int v; 
    scanf("%d", &v); 
    if (v == 0){
      pthread_mutex_lock(&lock_stop); 
      to_stop = 1; 
      pthread_mutex_unlock(&lock_stop); 
    }
  }
}

int stop(){
  int v ; 
  Chromosome* best = chromosomes[best_chromosomes[0]]; 
  if (best->fitness >= fitness_stop) return 1; 

  pthread_mutex_lock(&lock_stop); 
  v = to_stop;
  pthread_mutex_unlock(&lock_stop); 
  return v; 
}

void *start(double s){
  fitness_stop = s; 
  pthread_t stop_thread; 
  if (rank == 0){

    pthread_create(&stop_thread, NULL,check_stop_input,NULL); 
  }
  else{
    pthread_create(&stop_thread, NULL,check_stop_input_from_master,NULL); 

  }
  int v = (int) time(NULL) + rank*1000; 
  printf("Executor is starting . Hit ^C to terminate (Output generated then...) \n");

  unsigned vv = MurmurHash2(&v, rank*100, rank*1000 ); 
  srand(vv);
  printf("%d\n", vv); 
  prototype = init(2,2,80,3); 
  if (!prototype) {
    perror(""); 
    exit(-1);
  }
  if (init_population(1000,20,10,prototype) <0){
    perror("");
    exit(-1);
  } 

  // initialize new population with chromosomes randomly built using prototype
  int i = 0;
  for (i = 0 ; i < chromosomes_size ; i++){
    chromosomes[i] = create_chromo_by_proto(prototype); 
    add_to_best(i); 
  }

  /* double *avg_slots_occupied_best = malloc(sizeof(double) * track_best);  */
  /* double *avg_slots_occupied_total= malloc(sizeof(double) * chromosomes_size);   */
  /* int avg_i;  */

  current_generation = 0;
  /* FILE *fp_data = fopen("./results.2", "w+");  */
  
  while(!stop()){  
    Chromosome* best = chromosomes[best_chromosomes[0]]; 
    if ((current_generation % 100) == 0 ){ 
       printf("Current generation: %d. Best fitness :%f\n", current_generation, best->fitness  );
       int  size_send_data;
       int pi =0; 
       void *data = serialize_cromos(10, best_chromosomes, chromosomes, &size_send_data); 
       int ps;  MPI_Comm_size(WORLD, &ps);  
       unsigned char  *recv_buffer = malloc(size_send_data * ps);
       MPI_Allgather(data, (int) size_send_data, MPI_BYTE, recv_buffer, (int) size_send_data, MPI_BYTE, WORLD); 
       free(data); 
       unsigned csd; 
       unsigned process_index =0; 
       unsigned char *rcv_buffer_ptr = recv_buffer;  
       for (process_index= 0; process_index < ps ; process_index++){
	 Chromosome **new_cromos = deserialize_cromos(rcv_buffer_ptr, &csd);
	 for (pi = 0 ; pi < 10 ; pi++){
	  if (chromosomes[best_chromosomes[current_best_size -1]]->fitness <= new_cromos[pi]->fitness ){
	    int not_best_i;
	    do {
	      // select chromosome for replacement randomly
	      not_best_i = rand() % (int) chromosomes_size;
	      // protect best chromosomes from replacement
	    } while(best_flags[not_best_i]);
	    release(chromosomes[not_best_i]);
	    //TODO - clone this beast;
	    chromosomes[not_best_i] = clone_deserialized(new_cromos[pi]);
	    add_to_best(not_best_i);
	    }
	 }

	 //TODO - free remaining stuff; 
	 rcv_buffer_ptr += size_send_data; 
	 delete_new_cromos(new_cromos, 10);
        }
       free(recv_buffer);
       best = chromosomes[best_chromosomes[0]]; 
       printf("Current generation - after exchange: %d. Best fitness :%f\n", current_generation, best->fitness  );
       fflush(stdout); 
     }
     

    /* if ((current_generation % 300) == 0 ){ */
    /* for (avg_i = 0; avg_i < current_best_size ; avg_i++){ */
    /*   Chromosome *c = chromosomes[best_chromosomes[avg_i]];  */
    /*   int si;  */
    /*   int total= DAYS_HOURS * DAYS_NUM * cardHashS(global.rooms);  */
    /*   float occupied = 0.0;  */
    /*   for (si = 0 ; si <  total; si++){ */
    /* 	if (Varray_length(c->slots[si].classes_array) > 0){ */
    /* 	  occupied++;  */
    /* 	} */
    /*   } */
      
    /*   avg_slots_occupied_best[avg_i] = occupied / total;  */
    /* } */
    
    /* for (avg_i = 0; avg_i < chromosomes_size ; avg_i++){ */
    /*   Chromosome *c = chromosomes[avg_i];  */
    /*   int si;  */
    /*   int total= DAYS_HOURS * DAYS_NUM * cardHashS(global.rooms);  */
    /*   float occupied = 0.0;  */
    /*   for (si = 0 ; si <  total; si++){ */
    /* 	if (Varray_length(c->slots[si].classes_array) > 0){ */
    /* 	  occupied++;  */
    /* 	} */
    /*   } */
    /*   avg_slots_occupied_total[avg_i] =  occupied / total;  */
    /* } */

    /* #include <gsl/gsl_statistics.h>  */
    /* double mean, variance, largest, smallest; */
    /* double *data  = avg_slots_occupied_best;  */
    /* size_t size =  current_best_size;  */
    /* mean     = gsl_stats_mean(data, 1, size); */
    /* variance = gsl_stats_variance(data, 1, size); */
    /* largest  = gsl_stats_max(data, 1, size); */
    /* smallest = gsl_stats_min(data, 1, size); */
    
    /* fprintf(fp_data,"%d;%g;%g;%g;%g\n", current_generation, mean, variance, largest, smallest); */
    
    /* data  = avg_slots_occupied_total;  */
    /* size =  chromosomes_size;  */
    /* mean     = gsl_stats_mean(data, 1, size); */
    /* variance = gsl_stats_variance(data, 1, size); */
    /* largest  = gsl_stats_max(data, 1, size); */
    /* smallest = gsl_stats_min(data, 1, size); */
    
    /* fprintf(fp_data,"%d;%g;%g;%g;%g\n", current_generation, mean, variance, largest, smallest); */
    /* fflush(fp_data);  */

    // algorithm has reached criteria?
    if( best->fitness >= 0.99 ){
      break;
    }

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
  if (rank != 0){
    send_best_to_master(); 
  }
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

void print_class(FILE *fp, Varray *classes, int room);
void html_add_start(FILE *fp);
void html_add_end(FILE *); 

void print_best(){
  if (rank != 0) return ; 
  Chromosome* best = chromosomes[best_chromosomes[0]]; 
  printf(" Execution stopped. Going to print best chromo for file. Fitness: %f ; Generations : %d\n", best->fitness, current_generation); 
  FILE *fp =fopen("./out.html", "w+"); 
  if (!fp) {
    perror("Could not open file for output:"); 
    return;
  }
  int day, period, room ; 

  html_add_start(fp); 
  int nr = cardHashS(global.rooms);
      for (period = 0 ; period < DAYS_HOURS ; period++){
      fprintf(fp,"<tr class=\"day\">");
      for (day = 0 ; day < DAYS_NUM ; day++){
	fprintf(fp, "<td>"); 
	for (room = 0 ; room <  nr; room++){
	  int pos = day * nr * DAYS_HOURS + room * DAYS_HOURS + period; 
	  Varray *classes = best->slots[pos].classes_array; 
	  if (Varray_length(classes) > 0){
	    if (Varray_length(classes) > 1){
	      fprintf(fp,"#conflict#"); 
	    }
	    print_class(fp,classes, room);
	    fprintf(fp, "<hr>"); 
	  }
	}
	fprintf(fp, "</td>"); 
      }
      fprintf(fp,"</tr>");
  }
  html_add_end(fp); 
  fclose(fp); 
  printf("Printed best\n"); 
}

void print_students(FILE *, Varray * students);

void print_class(FILE *fp, Varray *classes, int room){
  int i; 
  Class *c; 
  int has_next; 
  char room_id[32]; 
  sprintf(room_id, "%d", room); 
  
  Room *r = searchHashS(global.rooms, room_id); 
  for (i = 0 , c = Varray_get(classes,i), has_next = ((i+1) <  Varray_length(classes)) ; 
       i < Varray_length(classes); 
       i ++ , c = Varray_get(classes,i), has_next = ((i+1) <  Varray_length(classes))){
    fprintf(fp,"Class %d ", c->class_id); 
    fprintf(fp, "with teacher %s for class", c->teacher_id); 
    print_students(fp,c->students); 
    fprintf(fp, " @  %s", r->idd); 
    if (has_next){
      fprintf(fp, " # "); 
    }
  }
}

void print_students(FILE *fp, Varray * students){
  int i; 
  char *s; 
  int has_next; 
  if (Varray_length(students) > 1 ){
    fprintf(fp, "("); 
  }
  
  for (i = 0 , s = Varray_get(students,i), has_next = ((i+1) <  Varray_length(students)) ; 
       i < Varray_length(students); 
       i ++ , s = Varray_get(students,i), has_next = ((i+1) <  Varray_length(students))){
    fprintf(fp, "%s", s);
    if ( has_next)
      fprintf(fp, ","); 
  }

  if (Varray_length(students) > 1 ){
    fprintf(fp, ")"); 
  }
}

void html_add_start(FILE *fp){
const char *head = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n<link href=\"css/screen.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen, projection\" />\n<link href=\"css/print.css\" rel=\"stylesheet\" type=\"text/css\" media=\"print\" />\n<title>Calendar</title>\n</head>\n<body>\n<table cellspacing=\"0\", border=1>\n\t<caption>January 2079</caption>\n\t<colgroup>\n\t\t<col class=\"Mon\" />\n\t\t<col class=\"Tue\" />\n\t\t<col class=\"Wed\" />\n\t\t<col class=\"Thu\" />\n\t\t<col class=\"Fri\" />\n\t</colgroup>\n\t<thead>\n\t\t<tr>\n\t\t\t<th scope=\"col\">Monday</th>\n\t\t\t<th scope=\"col\">Tuesday</th>\n\t\t\t<th scope=\"col\">wednesday</th>\n\t\t\t<th scope=\"col\">Thursday</th>\n\t\t\t<th scope=\"col\">Friday</th>\n\t\t</tr>\n\t</thead><tbody>"; 
 fprintf(fp, "%s\n", head); 
 
}

void html_add_end(FILE *fp){
  const char  *end = "\t</tbody>\n</table>\n</body>\n</html>";
  fprintf(fp, "%s\n", end); 
}


///////////////////////////////////////////////////////////////////

