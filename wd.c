/*
Name: Jonathan
Reviewer: Tsach
Description: WD
*/

#define _POSIX_C_SOURCE (200112L)

#include <sys/types.h> /* pid_t , kill, SIGUSR1, SIGUSR2*/
#include <unistd.h> /* sleep, fork */
#include <signal.h> /* struct sigaction */
#include <stdio.h> /* printf */
#include <pthread.h>    /* pthread */
#include <semaphore.h>  /* sem_t */
#include <stdlib.h> /* getenv */
#include <fcntl.h> /* O_CREAT*/
#include <signal.h> /* sig_atomic_t */

#include "wd.h"
#include "../../ds/include/scheduler.h"

#define RESET   "\033[0m"
#define YEL     "\033[33m"       /* YEL */
#define RED     "\033[31m"      /* Red */
#define GRN     "\033[32m"      /* GRN */
#define FIRST_RUN (NULL == getenv("/IS_FIRST_RUN"))
#define FAILURE (-1)

enum bool
{
    FALSE = 0,
    TRUE
};

sched_t *g_sched = NULL;
pid_t g_pid_other_process = 0;
static sig_atomic_t g_count_signals_received = 0;
static sig_atomic_t g_both_pro_received_sig2 = FALSE;

/*** thread ***/
pthread_t g_app_thread = 0;
pthread_t g_wd_stop_thread = 0;

/*** flages ***/
int g_is_wd;

/*** arg ***/
int g_argc = 0;
char **g_argv = {NULL};

/*** semaphore ***/
sem_t *g_sem_wd_need_to_stop = NULL;
sem_t *g_sem_wd_finish_stop = NULL;
sem_t *g_sem_other_pro_ready = NULL;

/****  Tasks  ****/
static int SendSigTask(void *param);
static int CheckSignalReceivedTask(void *param);

/****  Handlers  ****/
static void SendHandler(int signum);
static void StopHandler(int signum);

/* func */
static wd_status_t CreatOtherProcess(int argc, char *argv[]);
static wd_status_t SetUp(void);
static void RunSched(void);
void *RunAppSched(void *param);
static void SleepOneSecond(void);
static wd_status_t Restart(void);
static void *InitArgs(int argc, char *argv[]);

/*** stop and clean ***/
static void StopApp(void);
static void *ExecutStopWd(void  *arg);
static void CleanTask(void *param);
static void CleanUp(void);

wd_status_t WDStart(int argc, char *argv[])
{
    wd_status_t status = SUCCESS;

    if (FIRST_RUN)
    {
        status = CreatOtherProcess(argc, argv);
        if (SUCCESS != status)
        {
            return status;
        } 
    }
    else /* if app or whatchdog creat by other */
    {
        g_pid_other_process = getppid();
    }

    status = SetUp();
    if (SUCCESS != status)
    {
        return status;
    }
    printf("did setup for g_is_wd is %d\n", g_is_wd);

    RunSched();

    return status;
}

static wd_status_t CreatOtherProcess(int argc, char *argv[])
{
    pid_t pid = 0;

    if (NULL == InitArgs(argc, argv))
    {
        return CREATE_FAIL;
    }

    pid = fork();
    if (0 > pid) 
    {
        /* "Failed to create chiled\n" */
        perror("fork errno");
        exit(1);
    }
    else if (0 == pid)  /* child process  */
    { 
        printf("child is start\n");
        if (FAILURE == execvp(g_argv[0], g_argv))
        {
            return CREATE_FAIL;
        }
    }
    else /* this process */
    {
        g_pid_other_process = pid;
        printf("father is start\n");
    }

    return SUCCESS;
}
static wd_status_t SetUp(void)
{
    int status = SUCCESS;
    uid129_t uid_ret = {0};

    struct sigaction struct_send_handler = {0};
    struct sigaction struct_stop_handler = {0};

    g_sem_other_pro_ready = sem_open("/OTHER_PRO_READY_TO_RUN", O_CREAT, 0666, 0);
    if (NULL == g_sem_other_pro_ready)
    {
        perror("sem_open error: ");
        return SETUP_FAIL;
    }
    
    struct_send_handler.sa_handler = &SendHandler;
    struct_stop_handler.sa_handler = &StopHandler;

    status = sigaction(SIGUSR1, &struct_send_handler, NULL);
    if (FAILURE == status)
    {
         return SETUP_FAIL;
    }

    status = sigaction(SIGUSR2, &struct_stop_handler, NULL);
    if (FAILURE == status)
    {
         return SETUP_FAIL;
    }

    g_sched = SchedCreate();
    if (NULL == g_sched)
    {
        return SETUP_FAIL;
    }

    uid_ret = SchedAddTask(g_sched, &SendSigTask, NULL, &CleanTask, NULL, time(NULL));
    if (UIDIsSame(g_bad_uid, uid_ret))
    {
        return SETUP_FAIL;
    }

    uid_ret = SchedAddTask(g_sched, &CheckSignalReceivedTask, NULL, &CleanTask, NULL, time(NULL) + 5);
    if (UIDIsSame(g_bad_uid, uid_ret))
    {
        return SETUP_FAIL;
    }   

    return status;
}

static void RunSched(void)
{
    if (!g_is_wd)
    {
        if (FIRST_RUN) /* user start app */
        {
            if (FAILURE == sem_wait(g_sem_other_pro_ready))
            {
                perror("sem_wait error: ");
                exit(1);
            }
        }
        else /* wd creat app */
        {
            if (FAILURE == sem_post(g_sem_other_pro_ready))
            {
                perror("sem_post error: ");
                exit(1);
            }
        }

        if (pthread_create(&g_app_thread, NULL, &RunAppSched, NULL))
        {
            perror("create error: ");
            exit(1);
        }
    }
    else /* is WD */
    {
        if (FAILURE == sem_post(g_sem_other_pro_ready))
        {
            perror("post error: ");
            exit(1);
        }

        pthread_create(&g_wd_stop_thread, NULL, &ExecutStopWd, NULL);

        if (SUCCESS != SchedRun(g_sched))
        {
            return;
        }

        SchedDestroy(g_sched);
    }
}

wd_status_t WDStop(void)
{
    int tryies = 3;    
    wd_status_t status = SUCCESS;

    g_sem_wd_need_to_stop = sem_open("/WD_NEED_TO_STOP", O_CREAT, 0666, 0);
    g_sem_wd_finish_stop = sem_open("/WD_FINISH_TO_STOP", O_CREAT, 0666, 0);

    printf(RED "try do WDStop\n" RESET);
    while (FALSE == g_both_pro_received_sig2 && (0 < tryies))
    {
        if (FAILURE == kill(g_pid_other_process, SIGUSR2))
        {
            perror("kill error: ");
            exit(1);
        }
        SleepOneSecond();

        --tryies;
    }

    /* kill WD strong... */
    if (FALSE == g_both_pro_received_sig2)
    {
        if (FAILURE == kill(g_pid_other_process, SIGKILL))
        {
            perror("kill error: ");
        }

        status = WD_FORCED_STOP;
    }
    else /* stop wd and clean */
    {
        sem_post(g_sem_wd_need_to_stop);

        while(sem_wait(g_sem_wd_finish_stop) < 0)
        {
            /* wait in loop becuse may get interrupt*/
        };
    }

    StopApp();

    CleanUp();

    return status;
}

static void StopApp(void)
{
    write(STDOUT_FILENO,"Stop APP task's \n",14);

    SchedStop(g_sched);

    free(g_argv);
    g_argv = NULL;
    
    if (FAILURE == sem_close(g_sem_other_pro_ready))
    {
        perror("close error: ");
        exit(1);
    }

    if (FAILURE == sem_close(g_sem_wd_need_to_stop))
    {
        perror("close error: ");
        exit(1);
    }

    if (FAILURE == sem_close(g_sem_wd_finish_stop))
    {
        perror("close error: ");
        exit(1);
    }
}

static void CleanUp(void)
{
    printf("clean\n");

    if (FAILURE == pthread_join(g_app_thread, NULL))
    {
        perror("join error: ");
        exit(1);
    }
    pthread_exit(NULL);

    if (FAILURE == unsetenv("/IS_FIRST_RUN"))
    {
        perror("unsetenv error: ");
        exit(1);
    }

    
    if (FAILURE == sem_unlink("/OTHER_PRO_READY_TO_RUN"))
    {
        perror("unlink error: ");
        exit(1);
    }

    if (FAILURE == sem_unlink("/WD_FINISH_TO_STOP"))
    {
        perror("unlink error: ");
        exit(1);
    }

    if (FAILURE == sem_unlink("/WD_NEED_TO_STOP"))
    {
        perror("unlink error: ");
        exit(1);
    }
}

static void SendHandler(int signum)
{
    (void)signum;
   
    __atomic_store_n(&g_count_signals_received, g_count_signals_received + 1, __ATOMIC_RELAXED);
}

static void StopHandler(int signum)
{
    (void)signum;

    /*
    if wd gets a sig he send to app.
    so if also app got it means both process got, so change the flage
    */
    if (FALSE == g_is_wd) /* is app */
    {
        /*write(STDOUT_FILENO,"App received SIGUSR2\n",10);*/
        __atomic_store_n(&g_both_pro_received_sig2, TRUE, __ATOMIC_RELAXED);
    }
    else /* is wd */
    {
        /*write(STDOUT_FILENO,"WD received SIGUSR2\n",9);*/
        if (FAILURE == kill(g_pid_other_process, SIGUSR2))
        {
            perror("kill error: ");
            exit(1);
        }
    }
}

/* task1 */
static int SendSigTask(void *param)
{
    (void)param;
    
    if (FALSE == g_is_wd)
    {
        printf(YEL "Send Sig from APP  to pid %d" RESET "\n\n", g_pid_other_process);
    }
    else
    {
        printf(GRN "Send Sig from WD  to pid %d" RESET "\n\n", g_pid_other_process);
    }

    if (FAILURE == kill(g_pid_other_process, SIGUSR1))
    {
        perror("kill error: ");
    }

    return 1;
}

/* task2 */
static int CheckSignalReceivedTask(void *param)
{
    (void)param;

    if (0 == g_count_signals_received)
    {
        printf(RED "%d received no sig!!" RESET "\n" , g_is_wd);
        return Restart();
    }

    g_count_signals_received = 0;

    return 5;
}


static wd_status_t Restart(void)
{
    if (SUCCESS != CreatOtherProcess(g_argc, g_argv))
    {
        return CREATE_FAIL;
    }

    if (FAILURE == sem_wait(g_sem_other_pro_ready))
    {
        return CREATE_FAIL;
    }

    return SUCCESS;
}

void *RunAppSched(void *param)
{
    (void)param;

    if (SUCCESS != SchedRun(g_sched))
    {
        return NULL;
    }

    SchedDestroy(g_sched);

    return param;
}

static void SleepOneSecond(void)
{
    unsigned int time_left_to_sleep = sleep(1);

    while (time_left_to_sleep)
    {
        time_left_to_sleep = sleep (time_left_to_sleep);
    }
}

static void *InitArgs(int argc, char *argv[])
{
    int i = 0;

    g_argc = argc;
    g_argv = (char **)malloc(sizeof(char *) * (argc + 1));
    if (NULL == g_argv)
    {
        return NULL;
    }

    if (!g_is_wd)
    {
        g_argv[0] = "./watchdog.out";
    }
    else
    {
        g_argv[0] = "./user_app.out";
    }

    for (i = 1; i < argc; ++i)
    {
        g_argv[i] = argv[i];
    }

    g_argv[i] = NULL;

    return g_argv;
}

static void CleanTask(void *param)
{
    (void)param;
}

static void *ExecutStopWd(void  *arg)
{
    (void)arg;

    g_sem_wd_need_to_stop = sem_open("/WD_NEED_TO_STOP", O_CREAT, 0666, 0);
    g_sem_wd_finish_stop = sem_open("/WD_FINISH_TO_STOP", O_CREAT, 0666, 0);

    sem_wait(g_sem_wd_need_to_stop);

    printf("\nstop WD\n");

    if (FAILURE == pthread_join(g_wd_stop_thread, NULL))
    {
        perror("join error: ");
        exit(1);
    }

    /*pthread_exit(NULL);*/

    SchedStop(g_sched);

    if (FAILURE == sem_post(g_sem_wd_finish_stop))
    {
        perror("post error: ");
        exit(1);
    }

    if (FAILURE == sem_close(g_sem_wd_need_to_stop))
    {
        perror("close error: ");
        exit(1);
    }

    if (FAILURE == sem_close(g_sem_wd_finish_stop))
    {
        perror("close error: ");
        exit(1);
    }

    if (FAILURE == sem_close(g_sem_other_pro_ready))
    {
        perror("close error: ");
        exit(1);
    }

    free(g_argv);
    g_argv = NULL;

    return NULL;
}


