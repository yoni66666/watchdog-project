/*
Name: Jonathan
Reviewer: Eliraz
Description: task
*/
#include <stdlib.h> /* malloc */
#include <assert.h> /* assert */

#include "../include/task.h"

struct task
{
    task_action_t ac_func;
    void *action_param;
    task_cleanup_t cl_func;
    void *cleanup_param;
    time_t executed_time;
    uid129_t uid;
};

/*********  TaskCreate  *********/
task_t *TaskCreate(task_action_t ac_func, void *action_param, task_cleanup_t cl_func, 
				   void *cleanup_param, time_t executed_time)
{
    task_t *new_task = NULL;
    
    assert(ac_func);
    assert(cl_func);
    
    new_task = (task_t *)malloc(sizeof(task_t));
    if (NULL == new_task)
    {
        return NULL;
    }
    
    new_task->ac_func = ac_func;
    new_task->action_param = action_param;
    new_task->cl_func = cl_func;   
    new_task->cleanup_param = cleanup_param;
    new_task->executed_time = executed_time;
    new_task->uid = UIDGet();
    
    if(UIDIsSame(new_task->uid , g_bad_uid))
    {
        free(new_task);
        new_task = NULL;
    }

    return new_task;
}

/*********  TaskDestroy  *********/
void TaskDestroy(task_t *task)
{
    assert(task);
    
    task->cl_func(task->cleanup_param);
    free(task);
    task = NULL;
}

/*********  TaskExecute  *********/
int TaskExecute(const task_t *task)
{
    assert(task);

    return (task->ac_func(task->action_param));
}

/*********  TaskSetTimeToRun  *********/
void TaskSetTimeToRun(task_t *task, time_t new_time)
{
    assert(task);
    
    task->executed_time = new_time;
}

/*********  TaskGetTimeToRun  *********/
time_t TaskGetTimeToRun(const task_t *task)
{
    assert(task);
    
    return (task->executed_time);
}

/*********  TaskGetUID  *********/
uid129_t TaskGetUID(const task_t *task)
{
    return (task->uid);
}



    


	   
	   
