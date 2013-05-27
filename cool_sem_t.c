#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct cool_sem {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    unsigned  count;
} cool_sem_t;

cool_sem_t *semc_init(unsigned  max){
  cool_sem_t *s = malloc(sizeof(cool_sem_t)); 
  if (!s) return NULL; 
  if (pthread_mutex_init(&(s->lock), NULL) != 0){
    perror("Creating semc mutex : ");
    return NULL; 
  }
  if (pthread_cond_init( &(s->cond), 0 ) != 0){
    perror("Creating semc cond: "); 
    return NULL;
    ;
  }
  s->count = max;
  return s; 
}

void semc_incr( cool_sem_t *s, unsigned delta ){
  if (pthread_mutex_lock(&(s->lock )) != 0){
    perror("semc acquire mutex :" );
    exit(-1);
  }
  s->count += delta;
  pthread_cond_broadcast( &s->cond );
  if (pthread_mutex_unlock( &s->lock )!= 0){
    perror("semc release : ") ;
    exit(-1); 
  }
}

void semc_decr( cool_sem_t *s, unsigned delta ){
  if (pthread_mutex_lock( &s->lock ) != 0){
    perror("semc decr /acquire : ");
    exit(-1);
  }
    do {
        if (s->count >= delta) {
            s->count -= delta;
            break;
        }
        pthread_cond_wait( &s->cond, &s->lock );
    } while (1);
    if (pthread_mutex_unlock(&s->lock)!= 0){
      perror("semc decr release :"); 
      exit(-1); 
    }
}
