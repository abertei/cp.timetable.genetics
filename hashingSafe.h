/**    @file hashing.h */
#ifndef __HASHING_SAFE_H
#define __HASHING_SAFE_H

#include "common.h"
/**
   Uma HashTable permite associações entre dados e keys, sendo 
   que é optimizada para retornar estes valores muito rápidamente. 
   

   Os valores inseridos na HashTable - keys e dados - são apenas apontadores, 
   e o seu conteúdo nunca é copiado para a  tabela.
   Logo é necessário que sejam alocados propriamente , e disponíveis
   durante o seu tempo de inclusão na HashTable. Além disso devem ser também
   libertados aquando a sua remoção. Sugere-se por isso que  os dados de um elemento
   contenham a sua chave, visto que a consulta ou remoção à Tabela retornam apenas 
   os dados associados a uma dada key. 


  Uma HashTable deve apenas ser manipulada através das funções aqui apresentadas. 
*/
typedef struct hashTable_s HashTableS; 


/**
   @brief Inicializa uma HashTable 
   
   A função inicializa uma tabela de hash 
   com resize automático (aquando a inserção de elementos) 
   e um load factor máximo de 0.8

   @param cmpfoo é uma função de comparação entre keys
   @param hashfoo é uma função de hashing. 

   @return NULL se não conseguir alocar memória
   Apontador para uma HashTable caso contrário.
 */


HashTableS *initHashS(pt_foo_cmp cmpfoo, pt_foo_hash hashfoo);


/**
   @brief Número do elementos na tabela

   @param htable Apontador para a tabela de Hash

   @return O número de elementos...
 */
unsigned int cardHashS(HashTableS *htable);

  
/** 
   @brief Adiciona elemento à tabela de Hash

   A função indexa um apontador para elem , indexado por key. 
   Note-se , não são guardadas cópias nem da key
   nem do elemento , apenas os apontadores!!
   
   Se já existir um elemento com a mesma key é completamente 
   substituído.
   
   @param htable Apontador para tabela de Hash
   @param key Key do elemento
   @param elem Apontador para dados do elemento
   
   @return OVERFLOW se o índice do  da hashfoo(key) for maior que o número
   total de blocos suportados pela tabela.\n
   MEM_ERROR se não conseguir alocar memória 
   0 se conseguir adicionar à tabela. 
*/

int add2HashS(HashTableS *htable, 
	    void *key,
	    void *elem);
 
/**
   @brief  Pesquisa na tabela de Hash
   
   @param htable Apontador para tabela de hash
   @param key Apontador para a key do elemento a procurar 

   @return NULL se o elemento não existir
   ou um apontador para os dados do mesmo , caso contrário
*/
void *searchHashS(HashTableS *htable, void *key); 

/** 
    @brief  Remove um elemento da Tabela de hash
    
    @param   htable   Apontador para a tabela de hash
    @param key Apontador para a key do elemento

    @return NULL caso o elemento não exista ou
    um apontador para os dados inseridos com o elemento
*/
void mapHashS(HashTableS *htable,  void *foodata,pt_foo_map foo);


#endif 


  
  

  

