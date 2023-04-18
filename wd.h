/*
Name: Jonathan
Reviewer: Tzach
Description: WD
*/

#ifndef __WD_H__
#define __WD_H__

typedef enum wd_status 
{
    SUCCESS = 0, 
    CREATE_FAIL, 
    SETUP_FAIL, 
    WD_FORCED_STOP
} wd_status_t;

extern int g_is_wd;

/*
	Desc:	a func that the user need to add to the code for be able
            to use Watchdog. this func will start it.
	Param:	arguments which passed to main
	Return:	if success return o, otherwise return status of failur
	Notes:	If App falls Watchdog will restart it, 
            and also when Watchdog Falls so App will restart Watchdog.
            the program use SIGUSR1 and SIGUSR2.
*/
wd_status_t WDStart(int argc, char *argv[]);

/*
    Desc: a func that the user need to add to end of the code
            for stop the Watchdog.
    Params: none
    Return value: if success return o, otherwise return status of failur
    Notes: app try to close Watchdog and do cleanup. if it faild 
           it will close whithout doing cleanup.
*/
wd_status_t WDStop(void);

#endif /* __WD_H__ */