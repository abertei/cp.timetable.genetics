#ifndef __CHROMOSOME__H
#define __CHROMOSOME__H

#define DAYS_HOURS  12
#define DAYS_NUM 5  
#include "hashing.h" 
#include "array.h" 


typedef struct slot{
  Varray * classes_array; 
}Slot; 

typedef struct chromosome{
  Slot* slots; // Slot entries; 
  unsigned slots_size; 
  int * classes_start; 
  int  *criteria ;  // Bool vector. is class i satisfactory? 
  float fitness; 
}Chromosome; 


void *serialize_cromos(unsigned n_2_send,  
		       int * best_cromos,
		       Chromosome * *cromos, 
		       int *size_in_bytes 
		       );

Chromosome **deserialize_cromos(unsigned char *buffer, unsigned *cr);
Chromosome *clone_deserialized(Chromosome *c); 

void delete_new_cromos(Chromosome **, int ); 
void evaluate_fitness_of_chromosome(Chromosome *cromo); 
//Creates Offspring from two parents; 
Chromosome *crossover( Chromosome *p1, Chromosome *p2);
void mutation(Chromosome *p); 
Chromosome * init(int numberOfCrossoverPoints, int mutationSize,
		  int crossoverProbability, int mutationProbability);

//Randomly exchange classes; 
void mutation(); 
Chromosome *create_chromo_by_proto(Chromosome *prototype); 
void release(Chromosome *cromo); 
#endif 
