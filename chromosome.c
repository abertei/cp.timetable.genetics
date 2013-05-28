#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "chromosome.h"
#include "hash.h"
#include "entities.h"
#include "array.h"
#include "main.h"
#define SEED_HASH 1337

//TODO - set slot and hash foo right
int slot_cmp_foo(const void *v1, const void *v2){
  char *v1t = (char *) v1;
  char *v2t = (char *) v2;
  return strcmp(v1t, v2t);
}

unsigned slot_hash_foo(const void *k, unsigned m){
  return (unsigned) MurmurHash2(k, strlen(k), SEED_HASH)  % m ;
}

int crossover_points=20; 
int mutation_size=100; 
int crossover_probability=90; 
int mutation_probability=10; 

// Initializes chromosomes with configuration block (setup of chromosome)
Chromosome * init(int numberOfCrossoverPoints, int mutationSize,
		  int crossoverProbability, int mutationProbability){

  int class_size = Varray_length(global.classes);
  crossover_points= numberOfCrossoverPoints >= class_size ? class_size : numberOfCrossoverPoints; 
 mutation_size = mutationSize; 
 crossover_probability=crossoverProbability; 
 mutation_probability=mutationProbability; 
  
  Chromosome *cromo = malloc(sizeof(Chromosome)); 
  if (!cromo) return NULL; 

  cromo->fitness = 0; 
  cromo->slots_size = DAYS_HOURS * DAYS_NUM * cardHashS(global.rooms); 
  cromo->slots = (Slot *) malloc(sizeof(Slot) * cromo->slots_size ); 
  if (!cromo->slots) return NULL; 
  cromo->criteria = malloc(sizeof(int) * Varray_length(global.classes) * 5); 
  if (!cromo->criteria) return NULL; 
  int index; 

  for (index = 0; index < cromo->slots_size  ; index++){
    Varray_init(&(cromo->slots[index].classes_array)); 
  }

  //Init hash tables; 
  cromo->classes_start = calloc(Varray_length(global.classes), sizeof(int));
  if (!cromo->classes_start) return NULL; 
  return cromo; 
}

Chromosome *clone_deserialized(Chromosome *c){
  Chromosome *nc = malloc(sizeof(Chromosome)); 
  if (!c) quitp(); 

  nc->slots_size = DAYS_HOURS * DAYS_NUM * cardHashS(global.rooms); 
  nc->slots = malloc(sizeof(Slot) * nc->slots_size); 
  if (!nc->slots) quitp();
  int index; 
  for (index = 0; index < nc->slots_size  ; index++){
    Varray_init(&(nc->slots[index].classes_array)); 
  }

  nc->criteria = malloc(sizeof(int) * Varray_length(global.classes) * 5); 
  if (!nc->criteria) quitp(); 

  //Simple copy. 
  nc->classes_start = c->classes_start; 
  nc->fitness = c->fitness; 
  
  //Need to fill slots from classes start. 
  for (index = 0 ; index < Varray_length(global.classes) ; index++){
    Class *c = Varray_get(global.classes, index); 
    int start_c = nc->classes_start[c->class_id]; 
    int j; 
    for (j = 0 ; j < c->periods; j++){
      Varray_push(nc->slots[start_c + j].classes_array, c); 
    }
  }
  return nc; 
}

void delete_new_cromos(Chromosome **cromos, int size){
  int i; 
  for (i = 0 ; i < size ; i++){
    //free(cromos[i]->classes_start); 
    free(cromos[i]); 
  }
  free(cromos); 
}

Chromosome *copy_init(Chromosome * cromo, int setup_only_bool){
  Chromosome *copied_cromo = malloc(sizeof(Chromosome)); 
  if (!copied_cromo) return NULL; 

  if (!setup_only_bool){
    
    copied_cromo->slots = malloc(sizeof(Slot) * cromo->slots_size); 
    copied_cromo->slots_size = cromo->slots_size; 
    if (copied_cromo->slots == NULL ) return NULL; 
    int i; 
    for (i = 0; i < cromo->slots_size ; i++){
      Varray * v=  cromo->slots[i].classes_array; 
      Varray *v_copy; Varray_init(&v_copy); 
      void *ptr;
      int z; 
      for (z = 0 , ptr = Varray_get(v, z); 
	   z < Varray_length(v); 
	   z++, ptr= Varray_get(v,z)){
	Varray_push(v_copy, ptr); 
      }
      copied_cromo->slots[i].classes_array = v_copy; 
    }
    
    int si = Varray_length(global.classes);
    copied_cromo->classes_start = calloc(si, sizeof(int));
    if (!cromo->classes_start) return NULL; 
    for (i  = 0 ; i < si ; i++){
      copied_cromo->classes_start[i] = cromo->classes_start[i];
    }
    copied_cromo->criteria = malloc(sizeof(int) * Varray_length(global.classes) * 5); 
    if (copied_cromo->criteria == NULL) return NULL; 
    for (i = 0 ; i < Varray_length(global.classes) * 5 ; i++){
      copied_cromo->criteria[i] = cromo->criteria[i];
    }
    copied_cromo->fitness = cromo->fitness; 
  }
  else{
    copied_cromo->slots = malloc(sizeof(Slot) * cromo->slots_size); 
    copied_cromo->slots_size = cromo->slots_size; 
    if (copied_cromo->slots == NULL ) return NULL; 
    int si = Varray_length(global.classes);
    copied_cromo->classes_start = calloc(si, sizeof(int));
    if (!cromo->classes_start) return NULL; 
    int index; 
    for (index = 0; index < cromo->slots_size  ; index++){
      Varray_init(&(copied_cromo->slots[index].classes_array)); 
    }
    copied_cromo->criteria = malloc(sizeof(int) * Varray_length(global.classes) * 5); 
    if (copied_cromo->criteria == NULL) return NULL; 
  }

  copied_cromo->slots_size = cromo->slots_size;   
  
  return copied_cromo; 
}

Chromosome *create_chromo_by_proto(Chromosome *prototype){
  // make new chromosome, copy chromosome setup
  Chromosome* new_chromosome = copy_init( prototype, 1);
  if (new_chromosome == NULL) return NULL; 
  int i; 
  
  // place classes at random position
  Class * it;  
  for (i =0, it = Varray_get(global.classes, i) ;  i < Varray_length(global.classes) ;  i++, it = Varray_get(global.classes, i) ){
    // determine random position of class
    int nr = cardHashS(global.rooms); 
    if (nr == 0 ) puts("Hi there future (present?) Fabio. This is past Fabio. Something bad happen@chromosome.c:124\n"); 
    
    unsigned dur = it->periods;
    int day = rand() % DAYS_NUM;
    int room = rand() % nr;
    //TODO - can get a position that does not satisfy criteria. ? 
    int time = rand() % ( DAYS_HOURS + 1 - dur );
    int pos = day * nr * DAYS_HOURS + room * DAYS_HOURS + time;

    // fill time-space slots, for each hour of class
    int j;
    for( j = dur - 1; j >= 0; j-- ){
      Varray_push(new_chromosome->slots[pos + j].classes_array, it); 
    }
      // insert in class table of chromosome
    new_chromosome->classes_start[it->class_id] = pos; 
  }
  evaluate_fitness_of_chromosome(new_chromosome); 
  // return smart pointer
  return new_chromosome;
}

Chromosome *crossover( Chromosome *p1, Chromosome *p2){
  int i; 
  // check probability of crossover operation
  if( rand() % 100 > crossover_probability ){
    // no crossover, just copy first parent
    return copy_init(p1, 0);
  }

  // new chromosome object, copy chromosome setup
  Chromosome* n = copy_init(p1, 1);
  if (n == NULL ) return NULL; 

  // number of classes
  int size = Varray_length(global.classes);

  int *cp = calloc(size,sizeof(int)); 
  if (cp == NULL) {
    release(n); 
    return NULL; 
  }
  
  // determine crossover point (randomly)
  for(i = crossover_points; i > 0; i-- ){
    while( 1 ){
      int p = rand() % size;
      if( !cp[ p ] ){
	cp[ p ] = 1;
	break;
      }
    }
  }

  //TODO - is it safe to assume everything running? 
  // make new code by combining parent codes
  int first = rand() % 2 == 0;

  for(i = 0; i < size; i++ ){
    Class *c = Varray_get(global.classes,i); 
    int slot_start = first ? p1->classes_start[i] : p2->classes_start[i]; 
    n->classes_start[i] = slot_start; 
    // all time-space slots of class are copied
    int j; 
    for(j = c->periods - 1; j >= 0; j-- ){
      Varray_push(n->slots[slot_start + j].classes_array, c);
    }
    // crossover point
    if( cp[ i ] )
      // change soruce chromosome
      first = !first;
  }
  evaluate_fitness_of_chromosome(n); 
  free(cp); 
  return n;
}

void evaluate_fitness_of_chromosome(Chromosome *cromo){
	// chromosome's score
	int score = 0;
	int number_of_rooms = cardHashS(global.rooms); 
	int daySize = DAYS_HOURS * number_of_rooms;
	int ci = 0;

	// check criterias and calculate scores for each class in schedule

	Class *c; 
	int c_start; 
	int class_index;
	int *array;

	for (class_index = 0 , ci =0 ,c = Varray_get(global.classes, class_index), c_start = cromo->classes_start[class_index] ;
	     class_index < Varray_length(global.classes) -1; 
	     class_index++, ci+=5, 
	       c = Varray_get(global.classes,class_index), 
	       array = cromo->classes_start, 
	       c_start = array[class_index]){

	  // coordinate of time-space slot
		int p = c_start;
		int day = p / daySize;
		int time = p % daySize;
		int room = time / DAYS_HOURS;
		char room_str[15]; 
		sprintf(room_str, "%d", room); 
		time = time % DAYS_HOURS;
		int dur = c->periods;		
	
	// check for room overlapping of classes
		int ro = 0;
		int period;
		for(period = dur - 1; period >= 0; period-- ){
		  //TODO - varray length semantics? 
		  if( Varray_length(cromo->slots[ p + period ].classes_array) > 1 ){
		    ro = 1;
		    break;
		  }
		}
		// on room overlaping
		if( !ro ){
		  score++;
		}

		cromo->criteria[ ci + 0 ] = !ro;

		Class* cc = c; 
		
		Room* r = (Room *) searchHashS(global.rooms, room_str);
		// does current room have enough seats
		cromo->criteria[ ci + 1 ] = (r->seats >= cc->seats);
		if( cromo->criteria[ ci + 1 ] ){
			score++;
		}
		// does current room have computers if they are required
		cromo->criteria[ ci + 2 ] = (cc->type != LAB || ( cc->type == LAB && r->type == LAB ));
		if( cromo->criteria[ ci + 2 ] ){
			score++;
		}

		int po = 0, go = 0;
		int room_index; 
		int dayt; 
		// check overlapping of classes for professors and student groups
		for( room_index = number_of_rooms, dayt = day * daySize + time ;
		     room_index > 0; 
		     room_index--, dayt += DAYS_HOURS ){
			// for each hour of class
		  int hour; 
		  for( hour = dur - 1; hour >= 0; hour-- ){
		    // check for overlapping with other classes at same time
		    Varray  * cl = cromo->slots[dayt + hour].classes_array; 
		    int indexx;
		    Class *it;
		    for ( indexx = 0, it = Varray_get(cl, indexx) ; indexx < Varray_length(cl) ; indexx++, it = Varray_get(cl, indexx)){
		      if( cc != it ){
			// professor overlaps?
			if( !po && (strcmp(cc->teacher_id,it->teacher_id) ==0) ){
			  po = 1;
			}
			// student group overlaps?
			if( !go && group_overlap(cc, it )){
			  go = 1;
			}
			// both type of overlapping? no need to check more
			if( po && go )
			  goto total_overlap;
		      }
		    }
		  }
		}

	total_overlap:
		
		// professors have no overlaping classes?
		if( !po ){
		  score++;
		}
		cromo->criteria[ ci + 3 ] = !po;

		// student groups has no overlaping classes?
		if( !go ){
		  score++;
		}
		cromo->criteria[ ci + 4 ] = !go;
	}
	// calculate fitness value based on score
	cromo->fitness = (float) score * 1.0  /( Varray_length(global.classes) *  DAYS_NUM );

}  

void mutation(Chromosome *cromo){
  // check probability of mutation operation
  if( rand() % 100 > mutation_probability){
    return;
  }
  // number of classes
  int numberOfClasses = Varray_length(global.classes); 
  // move selected number of classes at random position
  int i;
  for(i = mutation_size; i > 0; i-- ){
    // select random chromosome for movement
    int mpos = rand() % numberOfClasses;
    
    Class *cc1 = Varray_get(global.classes, mpos) ; 
    int pos1 =  cromo->classes_start[mpos]; 
    // current time-space slot used by class

    // determine position of class randomly
    int nr = cardHashS(global.rooms); 
    int dur = cc1->periods; 
    int day = rand() % DAYS_NUM;
    int room = rand() % nr;
    int time = rand() % ( DAYS_HOURS + 1 - dur );
    int pos2 = day * nr * DAYS_HOURS + room * DAYS_HOURS + time;

    // move all time-space slots
    int j;
    for(j = dur - 1; j >= 0; j-- ){
      // remove class hour from current time-space slot
      Varray *cl = cromo->slots[pos1 +j].classes_array; 
      Varray *cl_new; 
      Varray_init(&cl_new); 
      int index; 
      void *ptr; 
      for (index = 0, ptr= Varray_get(cl,index); index < Varray_length(cl) ; index ++, ptr = Varray_get(cl, index)){
	if (ptr != cc1){
	  Varray_push(cl_new, ptr);
	}
      }
      Varray_clear(cl); 
      cromo->slots[pos1 + j].classes_array = cl_new; 
      // move class hour to new time-space slot
      Varray_push(cromo->slots[ pos2 + j ].classes_array,  cc1 );
    }
    cromo->classes_start[cc1->class_id] = pos2; 
    // change entry of class table to point to new time-space slots
  }
  evaluate_fitness_of_chromosome(cromo); 
}

void release(Chromosome *cromo){
  int i; 
  for (i = 0 ; i < cromo->slots_size ; i++){
    Varray_free(cromo->slots[i].classes_array); 
  }
  free(cromo->slots); 
  free(cromo->classes_start); 
  free(cromo->criteria); 
  free(cromo); 
}


unsigned int size_of_serialized_cromo(); 
void serialize_cromo(Chromosome *c, unsigned char * buffer); 

void *serialize_cromos(unsigned n_2_send,  
		       int * best_cromos,
		       Chromosome **cromos, 
		       int *size_in_bytes 
		       ){
  // Number of chromosomes : 
  // Chromosome 1 | Chromosome 2 | Chromosome 3 
  *size_in_bytes = n_2_send * size_of_serialized_cromo() + sizeof(unsigned); 
  unsigned char *buffer = malloc( n_2_send * size_of_serialized_cromo() + sizeof(unsigned)); 

  if (!buffer) return NULL; 
  unsigned char *bp = buffer; 
  int i = 0 ; 
  memcpy(bp, &n_2_send ,  sizeof(unsigned)); 
  bp+= sizeof(unsigned); 
  for (i = 0 ; i < n_2_send ; i++){
    serialize_cromo((cromos[best_cromos[i]]),bp);    
    bp += size_of_serialized_cromo(); 
  }
  return buffer; 
}

unsigned int size_of_serialized_cromo(){
  return Varray_length(global.classes)* sizeof(int)  + sizeof(float);
}

void serialize_cromo(Chromosome *c, unsigned char * buffer){
  // fitness | Classes array |
  memcpy(buffer, &(c->fitness), sizeof(float));
  memcpy(buffer + sizeof(float), c->classes_start , sizeof(int) * Varray_length(global.classes));
}

Chromosome *deserialize_cromo(unsigned char * buffer); 

Chromosome **deserialize_cromos(unsigned char *buffer, unsigned *cr){
  *cr = * (unsigned *) buffer; 
  buffer += sizeof(unsigned);
  int i;
  Chromosome * *cromos= malloc(sizeof(Chromosome*)  * (* cr) );
  if (!cromos) return NULL;
  for (i=0; i < *cr; i++){
    cromos[i] = deserialize_cromo(buffer);
    if (cromos[i] == NULL) return NULL;
    buffer += (size_of_serialized_cromo())  ;
  }
  return cromos;
}

Chromosome *deserialize_cromo(unsigned char * buffer){
  Chromosome *c = init(0,0,0,0);
  if (!c) return NULL;
  memcpy(&(c->fitness), buffer, sizeof(float));
  memcpy(c->classes_start, buffer + sizeof(float), Varray_length(global.classes) * sizeof(int) );
  return c;
}
