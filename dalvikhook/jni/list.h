/*****************************************************************************
*                                                                            *
*  -------------------------------- list.h --------------------------------  *
*                                                                            *
*****************************************************************************/

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>


/*****************************************************************************
*                                                                            *
*  Define a structure for linked list elements.                              *
*                                                                            *
*****************************************************************************/

typedef struct ListElmt_ {

//void               *data;
struct dalvik_hook_t *data;
struct ListElmt_   *next;
char dexpath[256];
int loaded;
char hashvalue[512];

} ListElmt;

/*****************************************************************************
*                                                                            *
*  Define a structure for linked lists.                                      *
*                                                                            *
*****************************************************************************/

typedef struct List_ {

int                size;

int                (*match)(const void *key1, const void *key2);
void               (*destroy)(void *data);

ListElmt           *head;
ListElmt           *tail;

} List;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

void list_init(List *list, void (*destroy)(void *data));

void list_destroy(List *list);

int list_ins_next(List *list, ListElmt *element, const void *data, char*);

int list_rem_next(List *list, ListElmt *element, void **data);

void* search_in_list(List *list, char *val);

#define list_size(list) ((list)->size)

#define list_head(list) ((list)->head)

#define list_tail(list) ((list)->tail)

#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_is_tail(element) ((element)->next == NULL ? 1 : 0)

#define list_data(element) ((element)->data)

#define list_next(element) ((element)->next)

#endif
