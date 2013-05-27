/**    @file hashing.h */
#ifndef __HASHING_H
#define __HASHING_H

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
typedef struct hashTable HashTable; 


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


HashTable *initHash(pt_foo_cmp cmpfoo, pt_foo_hash hashfoo);


/**
   @brief Número do elementos na tabela

   @param htable Apontador para a tabela de Hash

   @return O número de elementos...
 */
unsigned int cardHash(HashTable *htable);

  
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

int add2Hash(HashTable *htable, 
	    void *key,
	    void *elem);
 
/**
   @brief  Pesquisa na tabela de Hash
   
   @param htable Apontador para tabela de hash
   @param key Apontador para a key do elemento a procurar 

   @return NULL se o elemento não existir
   ou um apontador para os dados do mesmo , caso contrário
*/
void *searchHash(HashTable *htable, void *key); 

/** 
    @brief  Remove um elemento da Tabela de hash
    
    @param   htable   Apontador para a tabela de hash
    @param key Apontador para a key do elemento

    @return NULL caso o elemento não exista ou
    um apontador para os dados inseridos com o elemento
*/

void *rmFromHash(HashTable *htable, void *key);  

/**
  @brief Aplicar uma função a todos os elementos de uma HashTable 
  This name is kind of a mistake from a young wannabe. it should have been called foreach. 
  Refactoring modules in emacs needed please.  //REFACTOR

  Aplica a função foo a todos os elementos da tabela. 
  Não retorna valores, sugere-se que o controlo de erros seja efectuado
  a partir da passagem de parâmetros em foodata. 

  
  @param htable Apontador para uma HashTable 
  @param foodata Último argumento passado á função foo 
  @param foo A função a aplicar a cada elemento da tabela
  os argumentos devem função são os seguintes (void *elemkey, void *elemdata, void *foodata). 

 */
void mapHash(HashTable *htable,  void *foodata,pt_foo_map foo);


/** @brief  Indicar qual a função de comparação das keys

   @param htable Apontador para HashTable
   @param  cmpfoo Função de comparação entre keys
 */
void set_htable_cmp_foo(HashTable *htable, pt_foo_cmp cmpfoo);


/** @brief  Indicar qual a função de hashing para as keys

    @param htable Apontador para HashTable
    @param  hashfoo  Função de hashing 
 */
void set_htable_hash_foo(HashTable *htable, pt_foo_hash hashfoo);

void * begin_ht(HashTable *htable);


void * next_ht(HashTable *htable);

void *current_key_ht(HashTable *htable);

#endif 


  
  

  

