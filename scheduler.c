/*
Name: Jonathan
Reviewer: Tsach
Description: scheduler
*/

#include <stdlib.h> /* malloc */
#include <assert.h> /* assert */
#include <unistd.h> /* sleep */
#include <stdio.h>


#include "../include/scheduler.h"
#include "../include/p_queue.h"

#define ON (1)
#define OFF (0)
#define FAILURE (1)
#define SUCCESS (0)
#define EMPTY (0)

static int CompareTask(const void *data1, const void *data2, void *param);
static int UIDIsMathe(const void *uid1, const void *uid2, void *param);

struct sched
{
    p_queue_t *p_queue;
    int is_running;
};

/*********  ScedCreate  *********/
sched_t *SchedCreate(void)
{
    void *param = NULL;
    
    sched_t *new_sched_list = (sched_t *) malloc(sizeof(sched_t));
    if (NULL == new_sched_list)
    {
        return NULL;
    }
    
    new_sched_list->p_queue = PQueueCreate(CompareTask, param);
    if (NULL == new_sched_list->p_queue)
    {
        free(new_sched_list);
        new_sched_list = NULL;
        return NULL;
    }
    
    return new_sched_list;
}

/*********  ScedDestroy  *********/
void SchedDestroy(sched_t *sched)
{
    assert(sched);
    assert(sched->p_queue);
    
    SchedClear(sched);
    
    PQueueDestroy(sched->p_queue);
    sched->p_queue = NULL;
    
    free(sched);
    sched = NULL;
}

/*********  SchedClear  *********/
void SchedClear(sched_t *sched)
{
    task_t *task = NULL;
    assert(sched);
    assert(sched->p_queue);
    
    while(!SchedIsEmpty(sched))
    {
        task = PQueueDequeue(sched->p_queue);
        TaskDestroy(task);
        task = NULL;
    }
}

/*********  SchedAddTask  *********/
uid129_t SchedAddTask(sched_t *sched, task_action_t ac_func, void *action_param, task_cleanup_t cl_func, void *cleanup_param, time_t execute_time)
{
    task_t *new_task = NULL;
    
    assert(sched);
	assert(sched->p_queue);
	assert(ac_func);
	assert(cl_func);
	
	new_task = TaskCreate(ac_func, action_param, cl_func, cleanup_param, execute_time);
	if (new_task == NULL)
	{
		return g_bad_uid;
	}

    if (FAILURE == PQueueEnqueue(sched->p_queue, new_task))
    {
        free(new_task);
        new_task = NULL;
        
        return g_bad_uid;
    }
    
    return (TaskGetUID(new_task));
}

/*********  SchedRemoveTask  *********/
int SchedRemoveTask(sched_t *sched, uid129_t uid)
{
    void *task = NULL;
    
    assert(sched);
    assert(sched->p_queue);
    /*assert(!SchedIsEmpty(sched));*/

    task = PQueueErase(sched->p_queue, UIDIsMathe , &uid);
    if(NULL == task)
    {
        return FAILURE;
    }
    
    TaskDestroy(task);
    
    return SUCCESS;
}

/*********  SchedIsEmpty  *********/
int SchedIsEmpty(const sched_t *sched)
{
	assert(sched);
	assert(sched->p_queue);

    return PQueueIsEmpty(sched->p_queue);
}

/*********  SchedSize  *********/
size_t SchedSize(const sched_t *sched)
{
	assert(sched);
	assert(sched->p_queue);

    return PQueueSize(sched->p_queue);
}

/*********  SchedRun  *********/
int SchedRun(sched_t *sched)
{
    task_t *top_task = NULL; 
    int run_status = SUCCESS;
	uid129_t uid = {0,0,0};
    int action_status = 0;
    time_t time_to_run = 0;
    
	assert(sched);
	assert(sched->p_queue);
    
    sched->is_running = ON;

    while(!SchedIsEmpty(sched) && ON == sched->is_running)
    {
            top_task = PQueuePeek(sched->p_queue);
            uid = TaskGetUID(top_task);
            
            time_to_run = TaskGetTimeToRun((task_t *)top_task) - time(NULL);
            
            while(sleep(time_to_run) > 0)
            {    
            }

            action_status = TaskExecute(top_task);
           /* printf("action_status is %d whith %d\n", action_status, (int *)top_task);*/

            if (action_status > 0)
            {
                PQueueDequeue(sched->p_queue);
                TaskSetTimeToRun(top_task ,time(NULL) + action_status);
                
                if (PQueueEnqueue(sched->p_queue, top_task))
                {
                    SchedRemoveTask(sched , uid);
                    run_status =  FAILURE;
                }
            }
            else
            {
                SchedRemoveTask(sched , uid);
                if (action_status < 0)
                {
                   run_status =  FAILURE;
                } 
            }
            
    }
    
    return run_status;
}

/*********  SchedStop  *********/
void SchedStop(sched_t *sched)
{
	assert(sched);
	assert(sched->p_queue);

    sched->is_running = 0;
}

/*********  UIDIsMathe  *********/
static int UIDIsMathe(const void *task, const void *uid2, void *param)
{
    uid129_t uid1 = {0,0,0};
    assert(task);
    
    (void)param;
    
    uid1 = TaskGetUID(task);
    
    return UIDIsSame(uid1, *(uid129_t*)uid2);
}

/*********  CompareTask  *********/  
static int CompareTask(const void *data1, const void *data2, void *param)
{
    (void)param;
    return TaskGetTimeToRun((task_t *)data1) - TaskGetTimeToRun((task_t *)data2);
}



