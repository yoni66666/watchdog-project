/*
Name: Jonathan
Reviewer: Bar
Description: p_queue
*/

#include <stdlib.h> /* malloc */
#include <assert.h> /* assert */

#include "p_queue.h"
#include "../include/slist.h"

#define FAILURE (1)
#define SUCCESS (0)
#define EMPTY (0)

struct p_queue
{
    slist_t *slist;
    size_t num_of_element;    
};

/*********  PQueueCreate  *********/
p_queue_t *PQueueCreate(p_queue_cmp_t compare_func, void *param)
{
    p_queue_t *new_p_queue = NULL;
    assert(compare_func);
    
    new_p_queue = (p_queue_t *) malloc(sizeof(p_queue_t));
    if (NULL == new_p_queue)
    {
        return NULL;
    }
    
    new_p_queue->slist = SListCreate(compare_func, param);
    if (NULL == new_p_queue->slist)
    {
        free(new_p_queue);
        new_p_queue = NULL;
        return NULL;
    }
    
    new_p_queue->num_of_element = EMPTY;
    
    return new_p_queue;
}

/*********  PQueueDestroy  *********/
void PQueueDestroy(p_queue_t *p_queue)
{
    assert(p_queue);
    
    SListDestroy(p_queue->slist);
    free(p_queue);
    p_queue = NULL;
}

/*********  PQueueEnqueue  *********/
int PQueueEnqueue(p_queue_t *p_queue ,void *data)
{
    int status = SUCCESS;
    assert(p_queue);
    
    if (SListIterIsEqual(SListInsert(p_queue->slist, data),
       SListEnd(p_queue->slist)))
    {
        status = FAILURE;
    }
    else
    {
       ++(p_queue->num_of_element); 
    }
    
    return status;
}

/*********  PQueueDequeue  *********/
void *PQueueDequeue(p_queue_t *p_queue)
{
    assert(p_queue);
    --(p_queue->num_of_element);
    
    return SListPopFront(p_queue->slist);
}

/*********  PQueuePeek  *********/
void *PQueuePeek(const p_queue_t *p_queue)
{
    assert(p_queue);
    assert(!PQueueIsEmpty(p_queue));
    
    return SListGetData(SListBegin(p_queue->slist));
}

/*********  PQueueIsEmpty  *********/
int PQueueIsEmpty(const p_queue_t *p_queue)
{
    return (0 == p_queue->num_of_element);
}

/*********  PQueueSize  *********/
size_t PQueueSize(const p_queue_t *p_queue)
{
    assert(p_queue);
    
    return p_queue->num_of_element;
}

/*********  PQueueIsEmpty  *********/
void PQueueClear(p_queue_t *p_queue)
{
    assert(p_queue);
    
    while(!PQueueIsEmpty(p_queue))
    {
        SListPopFront(p_queue->slist);
        --(p_queue->num_of_element);
    }
}

/*********  PQueueErase  *********/
void *PQueueErase(p_queue_t *p_queue, p_queue_is_match_t IsMatchFunc, void *data)
{
    slist_iter_t slist_iter = {NULL};
    void *param = NULL;
    void *element_data = NULL;
    
    assert(p_queue);

    
    slist_iter = SListFindIf(SListBegin(p_queue->slist),
                 SListEnd(p_queue->slist), data, IsMatchFunc, param);
    
    if (!SListIterIsEqual(slist_iter, SListEnd(p_queue->slist)))
    {
        element_data = SListGetData(slist_iter);
        SListRemove(slist_iter);
        --(p_queue->num_of_element);
    }
    
    return element_data; 
}

























