/*
Name: Jonathan
Reviewer: Tzach
Description: WD
*/

#define _POSIX_C_SOURCE 200112L

#include <stdio.h> 
#include <unistd.h> /*sleep*/

#include "wd.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define FALSE (0)

static void SleepOneSecond(void);

int main(int argc, char *argv[])
{
    int status = SUCCESS;
    int i = 0;

    printf("I am user_app\n");

    g_is_wd = FALSE;

    status = WDStart(argc, argv);
    if (SUCCESS != status)
    {
        return status;
    }

    while (i < 10)
    {
        SleepOneSecond();
        ++i;
    }

    status = WDStop();

    return status;
}

static void SleepOneSecond(void)
{
    unsigned int time_left_to_sleep = sleep(1);

    while (time_left_to_sleep)
    {
        time_left_to_sleep = sleep(time_left_to_sleep);
    }
}