#ifndef __MAIN__H
#define __MAIN__H
void quitp(); 
typedef struct ssargs{
  int p; 
  float stop_fitness;
  int generation_sync_period;
  int benchmarking; 
  int population;
  int replace_by_generation ;  
  int track_best ; 
  int crossover_points; 
  int mutation_size ; 
  int crossover_probability ; 
  int mutation_probability; 
}sargs; 


#endif

