#ifndef __SEM__COOL_
#define __SEM__COOL_

typedef struct cool_sem cool_sem_t;

cool_sem_t *semc_init(unsigned max);

void semc_incr( cool_sem_t *s, unsigned delta );

void semc_decr( cool_sem_t *s, unsigned delta );


#endif
