/*  
    Manipulação de listas ligadas

*/

/* TODO 
   -walklist typedef-foo WILL REPLACE GETNODE
   -fold/map list 
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dictlist.h"

#include "common.h"
#include <stdio.h>

typedef struct nodo Nodo;
struct dictlista{
  int nelem;          /* number of elements   */
  pt_foo_cmp cmp; /* função de comparação entre keys  */
  Nodo *head;     /* cabeça da lista */ 
  Nodo *current_iterator_element; 
};

struct nodo{
  void  *key; /*  key do nodo, permite identificação  */
  void *data;   /* dados do nodo */
  Nodo *next; /* proximo nodo */
};

void * begin_dl(DictList *dlist){
  dlist->current_iterator_element = dlist->head; 
  return next_dl(dlist); 
}


void * next_dl(DictList *dlist){
  if (dlist->current_iterator_element->next != NULL){
    dlist->current_iterator_element = dlist->current_iterator_element->next; 
    return dlist->current_iterator_element->data; 
  }
  return NULL;
}

void *current_key_dl(DictList * dlist){
  return dlist->current_iterator_element->key; 
}

int has_next_dl(DictList *dlist){
  return dlist->current_iterator_element->next != NULL; 
}

/* size_t  visitKey(void *keyptr, FILE *fp) */
/* { */
/*   char *key = (char *) keyptr; */

/*   size_t keysize = strlen(key) +1 ;  */
/*   ITEM stream; */
/*   stream.size = keysize;  */

/*   stream.id = 0;  */

/*    fwrite(&stream, sizeof(ITEM) , 1 , fp);  */
/*    fwrite(keyptr, sizeof(char) , keysize , fp); \ */
/*    return ( sizeof(ITEM) + keysize * sizeof(char)); */
/* } */

/* size_t setNodeStream(Nodo *node , pt_foo_visita visitkey, pt_foo_visita visitdata, FILE *fp) */
/* { */

/*   ITEM stream; */

/*   size_t keysize;  */
/*   size_t datasize; */
/*   size_t node_size; */
/*   stream.id  = 0;  */
/*   long aux, aux2; */

/*   aux = ftell(fp); */
/*   fwrite(&stream, sizeof(stream), 1 , fp); */

/*   keysize = visitkey(node->key, fp, fundata);  */
/*   datasize = visitdata(node->data, fp, fundata);  */

/*   stream.size = keysize + datasize;  */

/*   aux2 = ftell(fp); */
/*   fseek(fp, aux , SEEK_SET); */

/*   node_size = fwrite(&stream, sizeof(ITEM), 1 , fp); */

/*   fseek(fp, aux2 , SEEK_SET); */

/*   return (stream.size + sizeof(ITEM)); */
/* } */

/* Nodo *readNodeStream(FILE *fp ) */
/* { */
/*   Nodo *new;  */
/*   ITEM stream; */
/*   new = malloc(sizeof(Nodo)); */
/*   if (!new) return NULL;  */
  
/*   fread(&stream, sizeof(ITEM) , 1 ,  fp); */
/*   new->key = malloc(stream.size); */
/*   fread(new->key , sizeof(char)  , stream.size , fp);  */

/*   return new; */
/* } */

/* size_t setListStream(DictList *list, pt_foo_visita visitkey , pt_foo_visita visitdata, FILE *fp, void *fundata) */
/* { */
/*   ITEM stream;  */
/*   stream.id = 0;  */
/*   size_t totalsize = 0; */

/*   long aux,aux2; */

/*   aux = ftell(fp); */
/*   fwrite(&stream , sizeof(ITEM), 1 , fp); */

/*   size_t curr_node_size; */

/*   Nodo *curr;  */
/*   for (curr =list->head->next ; curr ; curr = curr->next) */
/*     { */

/*       curr_node_size = setNodeStream(curr, visitkey, visitdata, fp, fundata); */

/*       totalsize +=  curr_node_size; */
/*     } */

/*   stream.size = totalsize; */

/*   aux2 = ftell(fp); */

/*   fseek(fp, aux , SEEK_SET); */

/*   fwrite(&stream, sizeof(ITEM), 1 , fp); */

/*   fseek(fp, aux2 , SEEK_SET); */

/*   return (totalsize + sizeof(ITEM)); */
/* } */

/* DictList *readListStream(FILE *fp) */
/* { */
/*   DictList *new;  */
/*   new = malloc(sizeof(DictList));  */

/*   new->head = malloc(sizeof(Nodo)); */
  
/*   Nodo *curr = new->head;  */
  
/*   ITEM stream;  */
/*   fread(&stream, sizeof(ITEM) , 1 , fp);  */
/*   size_t total = stream.size;  */
/*   size_t read = 0; */
/*   int nelem= 0; */

/*   for ( curr = new->head ;  read != total ; curr = curr->next, nelem++) */
/*     { */
/*       fread(&stream, sizeof(ITEM) , 1 ,  fp); */
/*       curr->next =  readNodeStream(fp); */
/*       read += sizeof(ITEM) + stream.size; */
/*     } */
/*   new->nelem = nelem; */
/*   return new; */
/* } */

/*  Inicia a lista ligada*/

DictList *initDictList(pt_foo_cmp cmp)
{
  DictList *new;  /*  ptr para nova lista */
  
  new = (DictList*) malloc(sizeof(DictList));
  if (!new) return NULL;

  new->cmp = cmp;
  new->nelem = 0; 

  new->head = (Nodo *) malloc(sizeof(Nodo)); /* cabeça fantasma da lista*/
  if (!new->head) return NULL; 
  new->head->next = NULL;
  new->head->key = NULL; 
  new->head->data = NULL;

 
  return new;
}

/* Nº de elementos na lista */
int cardDictList(DictList *list)
{
  return list->nelem; 
}

/*  Adiciona um novo nodo á lista */
int add2DictList(DictList *list, void *key, void *data)
{
  Nodo *new;                             /* Elemento novo */
  Nodo *prev;                            /* Ao percorrer a lista indica o elemento anterior ao actual */
  Nodo *curr;                            /* Ao percorrer a lista indica o elemento actual*/
  pt_foo_cmp cmp = list->cmp; /* Função de comparação entre as keys dos nodos */ 
  int c=1;                               /* resultado da função de comparação entre as keys de 2 nodos */

  /* criar e inicializar novo nodo*/
  new=  (Nodo *)malloc(sizeof(Nodo)); 
  if (!new) return MEM_ERROR; 

  new->key = key; 
  new->data = data;
  new->next = NULL; 
  
  /* Percorrer a lista */
  prev = list->head;  
  curr =list->head->next; 

  while (curr && ( (c=cmp(curr->key, key)) < 0 ))
    {
      prev = curr; 
      curr = curr->next;
    }

  /* elementos iguais */
  if (c==0)
    {
      new->next = curr->next; 
      prev->next = new;
      free(curr); 
    }
  /* se não ou o nodo actual é maior que o novo
     ou é NULL(fim da lista), em qualquer dos casos 
     new->next deve  ser inserido antes do actual */
  else
    { 
      new->next = curr; 
      prev->next = new; 
    }

  list->nelem ++;

  return 0;
}


void *rmFromDictList(DictList *list, void *key) 
{
  Nodo *prev;      /* elemento anterior */
  Nodo *curr;   /* elemento actual */
  pt_foo_cmp cmp = list->cmp; /* Função de comparação entre as keys dos nodos */ 
  int c=1;            /* resultado da comparação entre dois nodos*/
  void *data;  /* Apontador para os dados do nodo */

  /* Percorrer a lista */
  prev = list->head;
  curr = list->head->next; 
  
  while (curr && ((c=cmp(curr->key,key)) < 0))
    {
      prev  = curr;
      curr = curr->next;
    }    

  /* se o elemento existir */
  if (c==0)
    {
      prev->next = curr->next; 
      data=curr->data;
      free(curr);
      list->nelem --; 
      return data;
    }
  return NULL;

}

static Nodo *getnode(DictList *list, void *key)
{
  Nodo *prev;      /* elemento anterior */
  Nodo *curr;   /* elemento actual */
  pt_foo_cmp cmp  = list->cmp; /* Função de comparação entre as keys dos nodos */ 
  int c=1;            /* resultado da comparação entre dois nodos*/

  /* Percorrer a lista */
  prev = list->head;
  curr = list->head->next; 
  
  while (curr && ((c=cmp(curr->key,key)) < 0))
    {
      prev  = curr;
      curr = curr->next;
    }    
  /* se o elemento existir */
  if (c==0)
    return curr; 

  return NULL; 
}

/*
  Aplica foo a cada elemento da lista 

 */

void mapDictList(DictList *list,void *fundata, void  (*foo)(void *,void *,void *))
{
  Nodo *curr; 
  for (curr =list->head->next ; curr ; curr = curr->next)
    foo(curr->key, curr->data,fundata);

}

void deleteDictList(DictList *list) 
{
  Nodo *curr,*aux;
  int i;

  for (i =0,curr = list->head->next; curr;  i++)
    {
      aux = curr; 
      curr = curr->next;
      free(aux); 
    }
  free(list->head); 
  free(list); 

}

void *headDictList(DictList *list)
{

  if (list->nelem > 0)
    return list->head->next->data;
  
  return NULL;
}

void *takeHeadDictList(DictList *list)
{
  Nodo *node;
  void *data;

  
  if (list->nelem > 0)
    {
      node = list->head->next;
      data = node->data;
      list->head->next = node->next;
      free(node);
      list->nelem --; 
      return data;
  }

  return NULL; 
}

void *searchDictList(void *list, void *key)
{
  Nodo *find = getnode(list,key); 
  if (find) return find->data;
  else return NULL; 
}

struct Teste{
  unsigned int n;
  char *str;
}teste[] = {
  { 1, "berlin"},
  { 2 ,"lisboa"},
  { 3, "moncao"},
  { 4, "porto"},
  { 5, "algarve"},
  { 6, "braga"},
  { 7, "barcelos" },
  { 8, "guimaraes" },
  { 9, "acores" },
  {10,"spaulo"},
  {11,"DI"},
  {12,"barbeita"},
  {13,"trute"},
  {14,"lua"},
  {15,"terra"},
  {16,"marte"},
  {17,"ny"},
  {18,"DC"},
  {19,"boston"},
  {20,"dallas"},
  {21,"la"}
};

int compare(const void *key1, const void *key2)
{
  return   matches((char *) key1, (char *) key2);
}

/* int main() */
/* { */

/*   DictList *lista = initDictList(compare); */
/*   int i; */
/*   for (i = 0 ; i< 21 ; i++) */
/*     add2DictList(lista, teste[i].str , NULL); */

/*   FILE *fp = fopen("saving.test" , "w+"); */
  
  
/*   size_t maluk = setListStream(lista, visitKey , NULL ,  fp); */
  
/*   fclose(fp); */

/*   fp = fopen("saving.test" , "r");  */
  
  
  
/*    DictList *nova = readListStream( fp); */
/*    nova->cmp = compare;   */

/*   return 0; */
/* } */
