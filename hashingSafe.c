/*
	Implementação de uma HashTable. As colisões são tratadas com um dicionário de listas ligadas
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hashingSafe.h"
#include "dictlist.h"
#include "common.h"

#include <pthread.h>
#include <semaphore.h> 
#include "cool_sem_t.h"

#define DEFAULT_LOAD  0.8

#define MAX_THREADS 100

struct hashTable_s{
  cool_sem_t *global_lock; // Maximum threads inside 
  pthread_mutex_t *buckets_lock; // Array of locks 
  DictList **hashArray;          /*  hashArray[i] contém a lista de elementos cujo hashfoo(key) = i */
  unsigned size;              /* Nº de elementos do array */
  unsigned nelem;             /* Nº de elementos presentes na hash */
  float maxLoad;              /* load factor máximo (para quando a tabela puder ser automaticamente redimensionada */
  pt_foo_cmp cmpfoo ;         /* função de comparação para dois elementos da tabela */
  pt_foo_hash hashfoo;        /* função de hash da tabela  */
  int pos; 
  /* Bool Values */ 
  int allow_resize;           /* Permite o redimensionamento automatico da tabela */         
};

static int add2Hash_(HashTableS *htable, void *key,void *elem); 

unsigned  cardHashS(HashTableS *htable){
  unsigned v ;
  semc_incr(htable->global_lock, 1); 
  v =  htable->nelem;
  semc_decr(htable->global_lock, 1); 
  return v; 
}

/*
  tamanhos permitidos para a hash table. Tudo primos
  com uma boa distancia entre potências de 2 
*/
#define SIZES_OF_HASH_TABLE 27
static const unsigned sizes_of_hash_table[SIZES_OF_HASH_TABLE] = {
  11,
  53,  
  97, 
  193, 
  389, 
  769, 
  1543,
  3079,
  6151,
  12289,
  24593,
  49157,
  98317,
  196613, 
  393241,
  786433,
  1572869,
  3145739,
  6291469,
  12582917,
  25165843,
  50331653,
  100663319,
  201326611,
  402653189,
  805306457,
  1610612741
};

static inline unsigned  right_size_of_table(unsigned  size)
{
  int i;
  for (i = 0 ; i < SIZES_OF_HASH_TABLE  && size <= sizes_of_hash_table[i] ; i ++)
    continue; 
				
  return sizes_of_hash_table[i]; 
}
/*
  esta inicialização é com valor por defeito 
*/
HashTableS *initHashS(pt_foo_cmp cmpfoo, pt_foo_hash hashfoo)
{
  HashTableS *table;  /* nova tabela */
  table = (HashTableS *) malloc(sizeof(HashTableS));
  if(!table) return NULL;
  table->global_lock = semc_init(MAX_THREADS); 
  

  table->buckets_lock = malloc(sizeof(pthread_mutex_t) * 11); 
  if (!table->buckets_lock) return NULL; 
  int jj; 
  for (jj=0; jj < 11 ; jj++){
    pthread_mutex_init(&table->buckets_lock[jj], NULL); 
  }
  

  /* inicialização de elementos */
  table->nelem = 0;
  table->size = sizes_of_hash_table[0];   /* tamanho mínimo */
  table->pos = 0; 

  /* 
     cada elemento é um apontador para NULL (calloc inicializa a 0) do
     tipo DictList.
  */

  table->hashArray = (DictList **) calloc(table->size ,sizeof(DictList *));
  if (!table->hashArray) return NULL; 
  
  table->cmpfoo = cmpfoo; 
  table->hashfoo = hashfoo;
  table->maxLoad = DEFAULT_LOAD;
  table->allow_resize = 1; 
  
  return table; 
}

/* 
   a função vai adicionar um novo elemento a uma hashtable
   fornecida em foodata
   aloca espaço para o resultado de addHash deve ser libertado 
   vamos aplicar esta função a todos os nodos da lista 
 */
static void addHashFromList(void *key, void *data, void *foodata) /* Utilizada em resizeHash */ 
{
  HashTableS *hashptr = (HashTableS *) foodata;
  add2Hash_(hashptr, key,data);
}


/* 
   criar nova tabela de hash maior, 
   hashar cada elemento da antiga na nova
 */
static HashTableS * resizeHash(HashTableS *htable)
{
  DictList *lista;         /* DictList ligada de elementos com a mesma hash(key) */
  int i;
  unsigned newsize =   sizes_of_hash_table[htable->pos +1 ];
  
  HashTableS *new = malloc(sizeof(HashTableS)); 
  if (!new) return NULL; 
  
  
  memcpy(new, htable, sizeof(HashTableS)); 

  /* Nova hash table */
  new->hashArray =  calloc(sizeof(DictList *) , newsize); 
  if (!new->hashArray) return NULL; 

  new->size = newsize; 
  new->pos ++; 
  new->nelem = 0; 

  /* Copiar todos os elementos para a nova hashtable */
  for (i = 0 ; i < htable->size ;  i++) 
    {
      lista = htable->hashArray[i]; 
      if (lista)
	{
	  htable->hashArray[i] = NULL; 
	  mapDictList(lista, new, addHashFromList);
	  deleteDictList(lista);
	}
    }

  return new;
} 

static void lock(HashTableS *, int );
static void unlock(HashTableS * , int);
/*
  Adiciona um novo elemento à tabela
*/
int add2HashS(HashTableS *htable, 
	    void *key,
	    void *elem){
  unsigned int pos;  /* Posição na table */

  pos = htable->hashfoo(key, htable->size);  
  semc_incr(htable->global_lock, 1);
  if (pos >= htable->size) {
    semc_decr(htable->global_lock, 1); 
    return OVERFLOW;
  }
    
  int changed =0 ; 
  /* resize se for necessário */
  if ((htable->nelem / (float) htable->size ) >= DEFAULT_LOAD){
    semc_decr(htable->global_lock, 1); // Otherwise two threads could be stuck inside this position. 
    changed = 1; 
    semc_incr(htable->global_lock, MAX_THREADS); 
    if ((htable->nelem / (float) htable->size ) >= DEFAULT_LOAD){ //Check again, could have changed meanwhile.
      HashTableS *new;
      new = resizeHash(htable);
      if (!new)  {
	semc_decr(htable->global_lock, MAX_THREADS); 
	return MEM_ERROR; 
      }
      free(htable->hashArray); 
      memcpy(htable, new, sizeof(HashTableS)); 
    }
    semc_decr(htable->global_lock, MAX_THREADS); 
  }
  
  if (!changed) { //Most usual scenario. Could exploit processor branch prediction. 
    //changed = 0 => we have a lock acquired in the beginning of this function. 
    semc_decr(htable->global_lock, 1);
  }
  else{
    //changed = 1 => we do not have any lock now
    pos = htable->hashfoo(key, htable->size);
    semc_incr(htable->global_lock,1);
    if (pos >= htable->size){
      semc_decr(htable->global_lock, 1); 
      return OVERFLOW;
    }
    semc_decr(htable->global_lock, 1); 
  }

  //LOCK 
  lock(htable, pos); 
  /* Adicionar á tabela */
  if (htable->hashArray[pos] == NULL)  /*  Não existe nada ainda  */
    htable->hashArray[pos] = initDictList(htable->cmpfoo);  

  if (add2DictList(htable->hashArray[pos], key, elem) == 0)  /*  Tudo ok  */
    {
      htable->nelem ++;
      unlock(htable,pos); 
      return 0;
    }
  //UNLOCK 
  unlock(htable, pos); 
  /* return MEM_ERROR */
  return -1;
}

void *searchHashS(HashTableS *htable, void *key){
  unsigned int pos;
  DictList *lista;
  void *data;
  pos = htable->hashfoo(key,htable->size); 

  semc_incr(htable->global_lock, 1);
  if (pos >= htable->size) {
    semc_decr(htable->global_lock, 1);
    return NULL; /* <overflow */
  }
  semc_decr(htable->global_lock, 1);
 
  lock(htable,pos);
  lista = htable->hashArray[pos]; 

  if (lista) 
    if ((data= searchDictList(lista,key)) !=NULL) {
      unlock(htable,pos); 
      return data;
    }

  unlock(htable,pos); 
  return NULL;
}

/*
	Aplicar uma função (foo) a todos os elementos da tabela 
*/
void mapHashS(HashTableS *htable, 
	     void *foodata, 
	     void (*foo)(void *key, void *data, void *fundata))
{
  semc_incr(htable->global_lock, MAX_THREADS); 
  int i; /* contador */
  int m; /*total de elementos */
  DictList *list;
  for (i =0, m = htable->size ; i < m ; i++)
    {
      list = htable->hashArray[i];
      if (list && cardDictList(list)> 0)
	mapDictList(list,foodata,foo);
    }
  semc_decr(htable->global_lock, MAX_THREADS); 
}

int add2Hash_(HashTableS *htable, 
	    void *key,
	    void *elem){
  unsigned int pos;  /* Posição na table */

  pos = htable->hashfoo(key, htable->size);  
  if (pos >= htable->size) return OVERFLOW;
  
  /* resize se for necessário */
  if ((htable->nelem / (float) htable->size ) >= DEFAULT_LOAD)
    {
      if ((htable->nelem / (float) htable->size ) >= DEFAULT_LOAD){
	HashTableS *new;
	new = resizeHash(htable);
	if (!new)  return MEM_ERROR; 
	free(htable->hashArray); 
	memcpy(htable, new, sizeof(HashTableS)); 
      }
    }

  pos = htable->hashfoo(key, htable->size);
  if (pos >= htable->size) return OVERFLOW;
  
  //LOCK 
  /* Adicionar á tabela */
  if (htable->hashArray[pos] == NULL)  /*  Não existe nada ainda  */
    htable->hashArray[pos] = initDictList(htable->cmpfoo);  

  if (add2DictList(htable->hashArray[pos], key, elem) == 0)  /*  Tudo ok  */
    {
      htable->nelem ++;
      return 0;
    }
  //UNLOCK 
  /* return MEM_ERROR */
  return -1;
}

void lock(HashTableS * ht ,  int pos){
  int v;
  if ((v= pthread_mutex_lock(&(ht->buckets_lock[pos % 11]))) != 0){
    printf("Buckets: %d\n",v);
    //    perror("Locking buckets : ");
  }
}

void unlock(HashTableS * ht, int pos){
 if (pthread_mutex_unlock(&(ht->buckets_lock[pos % 11])) != 0){
    perror("Unlocking buckets : ");
  }
}
