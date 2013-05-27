/*
	Implementação de uma HashTable. As colisões são tratadas com um dicionário de listas ligadas
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hashing.h"
#include "dictlist.h"
#include "common.h"


#define DEFAULT_LOAD  0.8

struct hashTable{
  DictList **hashArray;          /*  hashArray[i] contém a lista de elementos cujo hashfoo(key) = i */
  unsigned size;              /* Nº de elementos do array */
  unsigned nelem;             /* Nº de elementos presentes na hash */
  float maxLoad;              /* load factor máximo (para quando a tabela puder ser automaticamente redimensionada */
  pt_foo_cmp cmpfoo ;         /* função de comparação para dois elementos da tabela */
  pt_foo_hash hashfoo;        /* função de hash da tabela  */
  int pos; 
  /* Bool Values */ 
  int allow_resize;           /* Permite o redimensionamento automatico da tabela */         
  
  int current_index_iterator; 
  int current_index_iterator_initialized_flag;
};

int _find_next_list_ht(HashTable *); 

void * begin_ht(HashTable *htable){
  if (htable->nelem > 0) {
    htable->current_index_iterator = 0; 
    if (htable->hashArray[0] == NULL){
      if (_find_next_list_ht(htable) == -1){
	return NULL; 
      }
    }
    return next_ht(htable); 
  }
  return NULL; 
}

int _find_next_list_ht(HashTable *htable){
  int i;
  for (i = htable->current_index_iterator +1  ; i < htable->size ; i++){
    if (htable->hashArray[i] != NULL) {
      htable->current_index_iterator = i; 
      htable->current_index_iterator_initialized_flag = 0; 
      return i; 
    }
  }
  htable->current_index_iterator = -1;  

  return -1; 
}

void * next_ht(HashTable *htable){
  int current =htable->current_index_iterator; 
  if (htable->current_index_iterator_initialized_flag == 0){
    //init iterator of list
    htable->current_index_iterator_initialized_flag = 1; 
    return begin_dl(htable->hashArray[current]); 
  }
  if (has_next_dl(htable->hashArray[current])){
    return next_dl(htable->hashArray[current]); 
  }
  if (_find_next_list_ht(htable) != -1){
    return next_ht(htable); 
  }
  return NULL; 
}

void *current_key_ht(HashTable *htable){
  return current_key_dl(htable->hashArray[htable->current_index_iterator]);  
}



unsigned  cardHash(HashTable *htable)
{
  return htable->nelem;
}

void set_htable_hash_foo(HashTable *htable, pt_foo_hash hashfoo)
{ 
  htable->hashfoo = hashfoo; 
}
void set_htable_cmp_foo(HashTable *htable, pt_foo_cmp cmpfoo)
{ 
  htable->cmpfoo = cmpfoo; 
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
HashTable *initHash(pt_foo_cmp cmpfoo, pt_foo_hash hashfoo)
{
  HashTable *table;  /* nova tabela */
  table = (HashTable *) malloc(sizeof(HashTable));
  if(!table) return NULL;
  
  
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
  HashTable *hashptr = (HashTable *) foodata;
  add2Hash(hashptr, key,data);
}


/* 
   criar nova tabela de hash maior, 
   hashar cada elemento da antiga na nova
 */
static HashTable * resizeHash(HashTable *htable)
{
  DictList *lista;         /* DictList ligada de elementos com a mesma hash(key) */
  int i;
  unsigned newsize =   sizes_of_hash_table[htable->pos +1 ];
  
  HashTable *new = malloc(sizeof(HashTable)); 
  if (!new) return NULL; 
  
  
  memcpy(new, htable, sizeof(HashTable)); 

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

/*
  Adiciona um novo elemento à tabela
*/
int add2Hash(HashTable *htable, 
	    void *key,
	    void *elem)
	 
{
  unsigned int pos;  /* Posição na table */
  int  changed = 0; 
  pos = htable->hashfoo(key, htable->size);  
  if (pos >= htable->size) return OVERFLOW;

  /* resize se for necessário */
  if ((htable->nelem / (float) htable->size ) >= DEFAULT_LOAD)
    {
      changed =1;
      HashTable *new;
      new = resizeHash(htable);
      if (!new)  return MEM_ERROR; 
      free(htable->hashArray); 
      memcpy(htable, new, sizeof(HashTable)); 
    }

  if (changed ){
    pos = htable->hashfoo(key, htable->size);  
    if (pos >= htable->size) return OVERFLOW;
  }

  /* Adicionar á tabela */
  if (htable->hashArray[pos] == NULL)  /*  Não existe nada ainda  */
    htable->hashArray[pos] = initDictList(htable->cmpfoo);  
  
  if (add2DictList(htable->hashArray[pos], key, elem) == 0)  /*  Tudo ok  */
    {
      htable->nelem ++;
      return 0;
    }

  /* return MEM_ERROR */
  return 0;

}

void *rmFromHash(HashTable *htable, void *key)
{

  unsigned int pos;
  DictList *lista;
  void *data;

  pos = htable->hashfoo(key,htable->size);
  
  if (pos >= htable->size) return NULL;  /* overflow */
  
  lista = htable->hashArray[pos]; 
  htable->nelem --; //TODO hammered here. 
  if (lista) 
    if ((data =rmFromDictList(lista,key))) 
	if (cardDictList(lista) == 0)
	  {
	    deleteDictList(lista); 
	    htable->hashArray[pos] = NULL; 
	    return data;
	  }


  return NULL;
}

/* unclean */
void *searchHash(HashTable *htable, void *key)
{
  unsigned int pos;
  DictList *lista;
  void *data;
  pos = htable->hashfoo(key,htable->size); 
  
  if (pos >= htable->size) return NULL; /* overflow */

  lista = htable->hashArray[pos]; 

  if (lista) 
    if ((data= searchDictList(lista,key)) !=NULL) 
      return data;

  return NULL;
}

/*
	Aplicar uma função (foo) a todos os elementos da tabela 
*/
void mapHash(HashTable *htable, 
	     void *foodata, 
	     void (*foo)(void *key, void *data, void *fundata))
{
  int i; /* contador */
  int m; /*total de elementos */
  DictList *list;
  for (i =0, m = htable->size ; i < m ; i++)
    {
      list = htable->hashArray[i];
      if (list && cardDictList(list)> 0)
	mapDictList(list,foodata,foo);
    }
}
