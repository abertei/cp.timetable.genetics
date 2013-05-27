#ifndef __MPI__GENETICS__H
#define __MPI__GENETICS__H

#include "entitities.h" 
#include "chromosome.h"

/**
 * 
 * @n_2_send Number of cromosome to serialize. Must be lower than best_cromos
 * @best_cromos The index of the best cromosomes in cromos
 * @cromos array of chromosomes. 
 */

void *serialize_cromos(unsigned n_2_send,  
		       int * best_cromos,
		       Chromosome * cromos, 
		       size_t *size_in_bytes, 
		       ); 

Chromosome *deserialize(size_t *n_cromos,  
			void * byte_array, 
			size_t size_in_bytes); 



#endif 
