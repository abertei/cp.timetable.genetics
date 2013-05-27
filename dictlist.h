
#ifndef __DICTLIST_H
#define __DICTLIST_H

#include "common.h"

/**@file dictlist.h 
   Uma DictList permite associações entre dados e keys.  No entanto
   as operações  de inserção, remoção e pesquisa , podem no pior das hipoteses
   necessitar de percorrer todos os elementos presentes no dicionário.\n 

   A DictList mantém os elementos presentes na lista ordenados de forma crescente ou 
   decrescente de acordo com uma função de comparação entre as keys fornecidada pelo utilizador.
   Desta forma é possível aceder a cada um deste de acordo com a sua posição na lista.\n

   Os valores inseridos na DictList - keys e dados - são apenas apontadores, 
   e o seu conteúdo nunca é copiado para a lista.\n

   Logo é necessário que sejam alocados propriamente , e disponíveis
   durante o seu tempo de inclusão na HashTable. Além disso devem ser também
   libertados aquando a sua remoção. Sugere-se por isso que  os dados de um elemento
   contenham a sua chave, visto que a consulta ou remoção à lista retornam apenas 
   os dados associados a uma dada key. \n


   Uma DictList vazia não é um apontador para NULL.

   Uma Dict deve apenas ser manipulada através das funções aqui apresentadas. \n

*/



/**
  Tipo da DictList 
 */
typedef  struct dictlista DictList;

/**
   @brief  Inicializa uma DictList

   Inicializa uma nova lista ligada 
   
   @param cmp Função de comparaçao das keys
   @return NULL se não arranjar memória
   @return Apontador para a nova lista ligada vazia

 */
DictList *initDictList(pt_foo_cmp cmp);

/**
   @brief Indica o nº de elementos presentes na lista
 */
extern int cardDictList(DictList *list);
  
/**
   @brief Adiciona um novo elemento  à lista ligada

   A função indexa um apontador para elem , indexado por key. 
   Note-se , não são guardadas cópias nem da key
   nem do elemento , apenas os apontadores!!

   Se algum já existir um elemento com a mesma key 
   é completamente substituído.

   @param head Apontador para cabeça da lista
   @param key  Apontador para a key do elemento 
   @param elem Apontador para os dados do elemento 

   @return MEM_ERROR se não alocar memória , 
   0 se inserir o elemento.
*/
extern int add2DictList(DictList *head, void *key, void *elem);

/** @brief Pesquisa numa lista 
    
    @param list Apontador para a lista ligada
    @param key Apontador para a key do elemento

    @return NULL se o elemento não existir
    ou um apontador para os dados caso contrário
*/
extern void *searchDictList(void *list, void *key);

/** @brief  Remove um elemento  da lista
    
    @param  list   Apontador para  a lista ligada
    @param  key Apontador para a key do elemento

    @return NULL caso o elemento não exista ou
    um apontador para os dados inseridos com o elemento
*/
extern void *rmFromDictList(DictList *list, void *key);



/**
   @brief Aplicar uma função a todos os elementos de uma DictList 

   Aplica a função foo a todos os elementos da lista
   Não retorna valores, sugere-se que o controlo de erros seja efectuado
   a partir da passagem de parâmetros em foodata. 
   p
   @param list  Apontador para uma DictList 
   @param  foodata Último argumento passado á função foo 
   @param foo A função a aplicar a cada elemento da tabela
   os argumentos devem função são os seguintes (void *elemkey, void *elemdata, void *foodata). 
   
 */

extern void mapDictList(DictList *list,void *foodata, pt_foo_map  foo);

/** 
    @brief Retorna os dados da cabeça da lista 

    
 */
extern void *headDictList(DictList *list);


/** 
    @brief Retirar a cabeça da lista. 

    @return  Apontador para os dados do elemento
    que estava presenta à cabeça da lista, eliminando o mesmo.
    @return NULL se a lista estiver vazia.
   
 */
extern void *takeHeadDictList(DictList *list);


/** 
    @brief Apaga a \ref DictList
 */
extern void deleteDictList(DictList *list);

/* size_t setListStream(DictList *list, pt_foo_visita visitkey , pt_foo_visita visitdata, FILE *fp); */
/* DictList *readListStream(FILE *fp); */


void * begin_dl(DictList *dlist);

void * next_dl(DictList *dlist);

void *current_key_dl(DictList * dlist);

int has_next_dl(DictList *dlist);


#endif 
