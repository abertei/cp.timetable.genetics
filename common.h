#ifndef __COMMON_H
#define __COMMON_H

/** @file common.h definição de códigos de erros e Tipos */

/**  Códigos de erro
 */
#define MEM_ERROR -10 /**< Falhas de chamdas a funções tipo malloc */
#define DUPLICATE_RECORD -11 /**< Indica que um elemento já existe na estrutura */
#define INVALID_REF -12      /**< O elemento indicado não existe na estrutura */
#define OVERFLOW -13         /**< O parametro indicado cai fora dos limites da estrutura*/

/**
   Tipo de uma função de comparação entre elementos
*/
typedef int (*pt_foo_cmp)(const void *v1, const void *v2);  /* Funções de comparação */

/**
   Tipo de uma função de hashing, deve retornar sempre um inteiro
   positivo de 0 até (m-1); 
 */
typedef unsigned  (*pt_foo_hash)(const void *, unsigned  m);  /* Funções de hashing */

/**
  Tipo de uma função de map. Estas função são normalmente 
  passadas a estruturas de dados para serem aplicadas a todos os seus elementos
  os argumentos devem função são os seguintes (void *elemkey, void *elemdata, void *foodata). 
 */
typedef void (*pt_foo_map)(void *, void *, void *);         /* Funções para map's */


int matches(const char *s1, const char *s2);

int factorial (int n);
int words(const char *line,  char **argv);

#endif
