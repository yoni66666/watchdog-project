/*
Name: Jonathan
Reviewer: Tzach
Description: WD
*/

#define _POSIX_C_SOURCE (200112L) /*setenv*/

#include <stdlib.h> /*setenv*/
#include <stdio.h> /* printf */

#include "wd.h"

#define TRUE (1)
#define FAILURE (-1)
#define OVERWRITE (1)

int main(int argc, char *argv[])
{
    wd_status_t status = SUCCESS;
    char *envval = "Y";
    g_is_wd = TRUE;

    status = setenv("/IS_FIRST_RUN", envval, OVERWRITE);
     /* setenv() function shall update or add a variable in 
       the environment of the calling process.  */
    if (SUCCESS != status)
    {
        return status;
    }

    status = WDStart(argc, argv);
    
    return status;;
}
