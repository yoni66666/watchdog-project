/*
Name: Yoni
Reviewer: Eliraz
Description: slist
*/

#include <stdlib.h> /* malloc */
#include <assert.h> /* assert */

#include "../include/slist.h"

#define NOT_EOL(slist_iter) (!SListIterIsEqual(slist_iter, SListEnd(slist_iter.sorted_list)))
#define FIRST_ELEMENT(slist_iter) !SListIterIsEqual(slist_iter, SListBegin(slist_iter.sorted_list))

struct slist
{
	dll_t *dll;
	slist_cmp_t cmp_func;
	void *param;
};

typedef struct w_param
{
    slist_cmp_t cmp_func;
    void *user_param;
}w_param_t;

static int CmpBoolWrapper(const void *func_data, const void *user_data, void *param);

/*********  SListCreate  *********/
slist_t *SListCreate(slist_cmp_t compare_func, void *param)
{
   slist_t *new_slist = (slist_t *)malloc(sizeof(slist_t));

    if (NULL == new_slist)
    {
        return NULL;
    }
    
    new_slist->dll = DLLCreate();
    if (NULL == new_slist->dll)
    {
        free(new_slist);
        new_slist = NULL;
        return NULL;
    }
   
    new_slist->cmp_func = compare_func;
    new_slist->param = param;
    
    return new_slist;
}

/*********  SListDestroy  *********/
void SListDestroy(slist_t *slist)
{
    assert(slist);
    
    DLLDestroy(slist->dll);
    free(slist);
    slist = NULL;
} 

/*********  SListInsert  *********/
slist_iter_t SListInsert(slist_t *slist, void *data)
{
    slist_iter_t slist_iter = {0};
    slist_iter_t eol_iter = {0};
    
    assert(slist);
    
    slist_iter = SListBegin(slist);
    eol_iter = SListEnd(slist);
    
    if(!SListIsEmpty(slist))
    {
        /* user data will be first when func return => 0 */
        while(!SListIterIsEqual(slist_iter, eol_iter) && 
        slist->cmp_func(SListGetData(slist_iter), data, slist->param) <= 0)
              
        {
            slist_iter = SListNext(slist_iter);
        }
    }
    
	#ifndef NDBUG
	slist_iter.sorted_list = slist;
	#endif
    
    slist_iter.slist_node = DLLInsert(data, slist_iter.slist_node);
    
    return slist_iter;
}

/*********  SListRemove  *********/
slist_iter_t SListRemove(slist_iter_t slist_iter)
{
    assert(slist_iter.slist_node);
    assert(!SListIsEmpty(slist_iter.sorted_list));
    
    DLLRemove(slist_iter.slist_node);
    
    return slist_iter;
}

/*********  SListBegin  *********/
slist_iter_t SListBegin(const slist_t *slist)
{
    slist_iter_t slist_iter = {0};
    
    assert(slist);
    assert(slist->dll);
    
    slist_iter.slist_node = DLLBegin(slist->dll);
    
#ifndef NDEBUG    
    slist_iter.sorted_list = slist;
#endif

    return slist_iter;
}

/*********  SListEnd  *********/
slist_iter_t SListEnd(const slist_t *slist)
{
    slist_iter_t slist_iter = {0};
    
    assert(slist);
    assert(slist->dll);
    
    slist_iter.slist_node = DLLEnd(slist->dll);
    
    #ifndef NDEBUG    
    slist_iter.sorted_list = slist;
    #endif

    return slist_iter;
}

/*********  SListGetData  *********/
void *SListGetData(const slist_iter_t slist_iter)
{
    void *data = NULL;
    
    assert(slist_iter.slist_node);
    assert(slist_iter.sorted_list);
    assert(NOT_EOL(slist_iter));
    
    data = DLLGetData(slist_iter.slist_node);
    
    return data;
}

/*********  SListNext  *********/
slist_iter_t SListNext(const slist_iter_t slist_iter)
{   
    slist_iter_t temp_iter = {NULL};
    
    assert(slist_iter.slist_node);
    assert(NOT_EOL(slist_iter));
    
    temp_iter = slist_iter;
    
    temp_iter.slist_node = DLLNext(temp_iter.slist_node);
    
    #ifndef NDEBUG    
    temp_iter.sorted_list = slist_iter.sorted_list;
    #endif
    
    return temp_iter;
}

/*********  SListPrev  *********/
slist_iter_t SListPrev(const slist_iter_t slist_iter)
{
    slist_iter_t temp_iter = {NULL};
    
    assert(slist_iter.slist_node);
    assert(FIRST_ELEMENT(slist_iter));
    
    temp_iter = slist_iter;
    
    temp_iter.slist_node = DLLPrev(temp_iter.slist_node);
    
    #ifndef NDEBUG    
    temp_iter.sorted_list = slist_iter.sorted_list;
    #endif
    
    return temp_iter;
}

/*********  SListIterIsEqual  *********/
int SListIterIsEqual(const slist_iter_t slist_iter1, const slist_iter_t slist_iter2)
{   
    assert(slist_iter1.sorted_list);
    assert(slist_iter2.sorted_list);
    
    return (slist_iter1.slist_node == slist_iter2.slist_node);
}

/*********  SListIsEmpty  *********/
int SListIsEmpty(const slist_t *slist)
{
    assert(slist);
    assert(slist->dll);
    
    return DLLIsEmpty(slist->dll);   
}

/*********  SListCount  *********/
size_t SListCount(const slist_t *slist)
{
    assert(slist);
    assert(slist->dll);
    
    return DLLCount(slist->dll);
}

/*********  SListPopFront  *********/
void *SListPopFront(slist_t *slist)
{
    assert(slist);
    assert(slist->dll);
    assert(!SListIsEmpty(slist));
    
    return DLLPopFront(slist->dll);
}

/*********  SListPopBack  *********/
void *SListPopBack(slist_t *slist)
{
    assert(slist);
    assert(slist->dll);
    assert(!SListIsEmpty(slist));
    
    return DLLPopBack(slist->dll);
}

/*********  SListFind  *********/
slist_iter_t SListFind(const slist_t *slist, const slist_iter_t from,
const slist_iter_t to, const void *data)
{
    slist_iter_t from_iter = {NULL};
    w_param_t struct_wrap = {NULL};
    
    assert(slist);
    assert(from.slist_node);
    assert(to.slist_node);
    assert(from.sorted_list == to.sorted_list);
    
    struct_wrap.cmp_func = slist->cmp_func;
    struct_wrap.user_param = slist->param;
    
    from_iter.slist_node = DLLFind(from.slist_node, to.slist_node, data,
     &CmpBoolWrapper, (void *)&struct_wrap);
     
 	#ifndef NDEBUG
    from_iter.sorted_list = from.sorted_list;;
    #endif

    return from_iter;
}

/*********  SListFindIf  *********/
slist_iter_t SListFindIf(const slist_iter_t from, const slist_iter_t to, const void *data, slist_is_match_t IsMatchFunc, void *param)
{
    slist_iter_t iter = {NULL};
    
    assert(from.slist_node);
    assert(to.slist_node);
    
    iter.slist_node = DLLFind(from.slist_node, to.slist_node, data, IsMatchFunc, param);
    
    #ifndef NDEBUG    
    iter.sorted_list = from.sorted_list;
    #endif
    
    return iter;
} 

/*********  SListForEach  *********/
slist_iter_t SListForEach(slist_iter_t from, const slist_iter_t to, slist_action_t ActionFunc, void *param)
{
    assert(from.slist_node);
    assert(to.slist_node);
    assert(ActionFunc);
    
    from.slist_node =  DLLForEach(from.slist_node , to.slist_node, ActionFunc, param);
    
    return from;  
}

/*********  SListMerge  *********/
slist_t *SListMerge(slist_t *dest_slist, slist_t *src_slist)
{
    slist_iter_t des_begin = {NULL};
    slist_iter_t des_runner = {NULL};
    slist_iter_t des_end = {NULL};
    slist_iter_t src_begin = {NULL};
    slist_iter_t src_runner = {NULL};
    slist_iter_t src_end = {NULL};
    
    assert(dest_slist);
    assert(src_slist);
    assert(dest_slist != src_slist);
    
    des_begin = SListBegin(dest_slist);
    des_runner = des_begin;
    src_begin = SListBegin(src_slist);
    src_runner = src_begin;
    src_end = SListEnd(src_slist);
    des_end = SListEnd(dest_slist);
    
    while (!SListIterIsEqual(src_runner, src_end)&&
           !SListIterIsEqual(des_runner, des_end))
    {
        /* while src is befor dest */
        if (0 <= dest_slist->cmp_func(SListGetData(des_runner), 
        SListGetData(src_runner), dest_slist->param))
        {
           src_begin = src_runner;
           /* while src is befor dest */
           while (!SListIterIsEqual(src_runner, src_end) &&
           0 <= dest_slist->cmp_func(SListGetData(des_runner), 
           SListGetData(src_runner), dest_slist->param)
           )
           {
                src_runner = SListNext(src_runner); 
           }
           
           DLLSplice(src_begin.slist_node, src_runner.slist_node,
                     des_runner.slist_node);
        }
        
        des_runner = SListNext(des_runner);   
    }
    
    if (!SListIsEmpty(src_slist))
    {
       DLLSplice(SListBegin(src_slist).slist_node, SListEnd(src_slist).slist_node,
        SListPrev(SListEnd(src_slist)).slist_node); 
    }
    
    return dest_slist;
}

static int CmpBoolWrapper(const void *func_data, const void *user_data, void *param)
{
    w_param_t *struct_wrap = param;
    
    return !struct_wrap->cmp_func(func_data, user_data, struct_wrap->user_param);
}


















