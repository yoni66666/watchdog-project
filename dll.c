/*
Name: Yoni
Reviewer: Sasha
Description: dll
*/

#include <stdlib.h> /* malloc */
#include <assert.h> /* assert */
#include <stddef.h> /*  size_t */

#include "../include/dll.h"

#define ITER_TO_NODE(dll_iter) ((node_t *)dll_iter)
#define IS_EOL(dll_iter) (NULL == dll_iter->next)
#define IS_EMPTY(dll_iter) (NULL == dll_iter->prev->prev && IS_EOL(dll_iter))
#define IS_FIRST(dll_iter) (NULL == dll_iter->prev)
#define FAILURE (1) 
#define SUCCESS (0)

typedef struct dll_node node_t ;

struct dll_node
{
    void *data;
    /* can do whithout a pointer */
	node_t *next;
	node_t *prev;
};
 

struct dll
{
     struct dll_node *head;
     struct dll_node *tail;
};

static void InitDll(dll_t *new_dll);
static int Add1(void *data, void *param);
static dll_iter_t EndOfList(dll_iter_t dll_iter);

/*********  DLLCreate  *********/
dll_t *DLLCreate(void)
{
    dll_t *new_dll = (dll_t *) malloc(sizeof(dll_t));
    if (NULL == new_dll)
    {
        return NULL;
    }
    /* erase*/
    new_dll->head = (node_t *)malloc(sizeof(node_t));
    if (NULL == new_dll->head)
    {
        free(new_dll);
        new_dll = NULL;
        return NULL;
    }
    /* erase*/
    new_dll->tail = (node_t *)malloc(sizeof(node_t));
    if (NULL == new_dll->tail)
    {
        free(new_dll->head);
        new_dll->head = NULL;
        free(new_dll);
        new_dll = NULL;
        return NULL;
    }
    
    InitDll(new_dll);

    return new_dll;
}

/*********  DLLDestroy  *********/
void DLLDestroy(dll_t *dll)
{
    assert(dll);
    
    while(!DLLIterIsEqual(DLLBegin(dll), DLLEnd(dll)))
    {
        DLLRemove(DLLBegin(dll));
    }
    /* erase*/
    free(dll->head);
    dll->head = NULL;
    /* erase*/
    free(dll->tail);
    dll->tail = NULL;
    free(dll);
    dll = NULL;
}

/*********  DLLInsert  *********/
dll_iter_t DLLInsert(void *data, dll_iter_t dll_iter)
{
    node_t *new_node = NULL; 
    
    assert(dll_iter);
    
    new_node = (dll_iter_t) malloc(sizeof(node_t));
    if(NULL == new_node)
    {
        return EndOfList(dll_iter);
    }
    /* connect func  twice*/
    new_node->data = data;
    new_node->prev = ITER_TO_NODE(dll_iter)->prev;
    new_node->next = ITER_TO_NODE(dll_iter);
    dll_iter->prev->next = new_node;
    dll_iter->prev = new_node;
    
    return new_node;
}

/*********  DLLRemove  *********/
dll_iter_t DLLRemove(dll_iter_t dll_iter)
{
    dll_iter_t next_iter = NULL;
    
    assert(dll_iter);
    assert(dll_iter->next);
    
    next_iter = dll_iter->next;
    
    dll_iter->next->prev = dll_iter->prev;
    dll_iter->prev->next = dll_iter->next;
    
    free(dll_iter);
    dll_iter = NULL;
    
    return next_iter;  
}

/*********  DLLFind  *********/
dll_iter_t DLLFind(const dll_iter_t from, const dll_iter_t to , const void *data, is_match_t is_match_func, void *param)
{
    dll_iter_t from_runner = NULL;
    
    assert(from);
    assert(to);
    assert(is_match_func);
    
    from_runner = from;
    
    while(!DLLIterIsEqual(from_runner, to) &&
          !is_match_func(from_runner->data, data, param))
    {
        from_runner = DLLNext(from_runner);
    }
    
    return from_runner;
}

/*********  DLLCount  *********/
size_t DLLCount(const dll_t *dll)
{
    size_t count = 0;
    
    assert(dll);
    
    DLLForEach(DLLBegin(dll), DLLEnd(dll) , Add1, &count);
    
    return count;    
}

/*********  DLLIsEmpty  *********/
int DLLIsEmpty(const dll_t *dll)
{
    return (dll->head->next == dll->tail);
}

/*********  DLLSetData  *********/
void DLLSetData(void *data, dll_iter_t dll_iter)
{
    assert(dll_iter);
    assert(dll_iter->next);
    /* add assert on head*/
    
    dll_iter->data = data;
}

/*********  DLLGetData  *********/
void *DLLGetData(const dll_iter_t dll_iter)
{
    assert(dll_iter->next);
    assert(dll_iter);
      /* add assert on head*/
    
    return dll_iter->data;
}

/*********  DLLNext  *********/
 dll_iter_t DLLNext(const dll_iter_t dll_iter)
{
    assert(dll_iter);
    
    if (IS_EMPTY(dll_iter) || IS_EOL(dll_iter))
	{
		return EndOfList(dll_iter);
	}
    
    return dll_iter->next;
}
 
/*********  DLLPrev  *********/ 
dll_iter_t DLLPrev(const dll_iter_t dll_iter)
{
    assert(dll_iter);
	assert(!IS_FIRST(dll_iter) && !IS_EMPTY(dll_iter));
	
	if (IS_EMPTY(dll_iter))
	{
		return EndOfList(dll_iter);
	}
	
    return dll_iter->prev;
}

/*********  DLLBegin  *********/
dll_iter_t DLLBegin(const dll_t *dll)
{
   assert(dll); 
   
   return (dll->head->next);
}

/*********  DLLEnd  *********/
dll_iter_t DLLEnd(const dll_t *dll)
{
    assert(dll); 
   
    return (dll->tail); 
}

/*********  DLLIterIsEqual  *********/
int DLLIterIsEqual(const dll_iter_t dll_iter1, const dll_iter_t dll_iter2)
{
    assert(dll_iter1);    
    assert(dll_iter2);
    
    return (dll_iter1 == dll_iter2);
}

/*********  DLLPushFront  *********/
dll_iter_t DLLPushFront(void *data, dll_t *dll)
{   
    assert(dll);
    
    return DLLInsert(data, DLLBegin(dll));
}

/*********  DLLPushBack  *********/
dll_iter_t DLLPushBack(void *data, dll_t *dll)
{
    assert(dll);
    
    return DLLInsert(data, DLLEnd(dll));
}

/*********  DLLPopFront  *********/
void *DLLPopFront(dll_t *dll)
{
	void *data_temp = NULL;
	
	assert(dll);
	assert(!DLLIsEmpty(dll));

    data_temp = DLLBegin(dll)->data;
    DLLRemove(DLLBegin(dll));
    
    return data_temp;
}

/*********  DLLPopBack  *********/
void *DLLPopBack(dll_t *dll)
{
	void *data_temp = NULL;
	
	assert(dll);
	assert(!DLLIsEmpty(dll));

    data_temp = (DLLPrev(DLLEnd(dll)))->data;
    
    DLLRemove(DLLPrev(DLLEnd(dll)));
    
    return data_temp;
}

/*********  DLLForEach  *********/
dll_iter_t DLLForEach(dll_iter_t from, const dll_iter_t to,
                      action_t action_func, void *param)
{
    assert(from);
    assert(to);
    assert(action_func);
    
    while(!DLLIterIsEqual(from, to) && SUCCESS == action_func(from->data, param))
    {
        from = DLLNext(from);
    }
    
    return from;   
}

/*********  DLLSplice  *********/
void DLLSplice(dll_iter_t from, const dll_iter_t to, dll_iter_t dll_iter)
{
    dll_iter_t temp = NULL;
    assert(from);
    assert(to);
        
    temp = to->prev;     
    
    from->prev->next = to;
    to->prev = from->prev;
    /* func connect */
    from->prev = dll_iter->prev;
    dll_iter->prev->next = from;
    temp->next = dll_iter;
    dll_iter->prev = temp;
}

/*********  DLLMultiFind  *********/
dll_iter_t DLLMultiFind(dll_iter_t from, const dll_iter_t to,void *data,
                        void *param, is_match_t func, dll_t *dest_dll)
{
    dll_iter_t from_runner = NULL;
    
    assert(from);
    assert(to);
    assert(func);
    assert(dest_dll);
    
    from_runner = from;
    
    while(!DLLIterIsEqual(from_runner, to))
    {
        from_runner = DLLFind(from_runner, to, data, func, param);
        if (from_runner != to)
        {
            DLLPushBack(from_runner->data, dest_dll);
            /* this can fail...*/
            from_runner = DLLNext(from_runner);
        }
    }
    
    return from_runner;
}

/*********  InitDll  *********/
static void InitDll(dll_t *new_dll)
{
    assert(new_dll);
    /* make a func of connecting */
    new_dll->head->prev = NULL;
    new_dll->head->next = new_dll->tail;
    new_dll->head->data = NULL;
    
    /* make a func of connecting */
    new_dll->tail->prev = new_dll->head;
    new_dll->tail->next = NULL;
    new_dll->tail->data = NULL;
}
    
/*********  action_t  *********/
static int Add1(void *data, void *param)
{
   (void)data;
   
   assert(param);
   
   *(size_t*)param = *(size_t*)param + 1;
   return 0;
}

/*********  EndOfList  *********/
static dll_iter_t EndOfList(dll_iter_t dll_iter)
{
	while (!IS_EOL(dll_iter))
	{
		dll_iter = DLLNext(dll_iter);
	}
	
	return dll_iter;
}
















