/*
Name: Jonathan
Reviewer: Mutty
Description: uid
*/

#include <stdlib.h> /* malloc */
#include <assert.h> /* assert */
#include <unistd.h> /* getpid */
#include <signal.h> /* sig_atomic_t */

#include "uid.h"

const uid129_t g_bad_uid = {0, 0, 0};

uid129_t UIDGet(void)
{
    static sig_atomic_t counter = 0;
    
    uid129_t new_uid = {0, 0, 0};
    
    new_uid.time = time(NULL);
    if (new_uid.time == (time_t)(-1))
    {
        return g_bad_uid;
    }
    /* counter will start from 1 */
    __atomic_store_n (&counter, counter + 1, __ATOMIC_RELAXED);

    new_uid.counter = counter;
    new_uid.pid = getpid();
    
   return new_uid;
}

int UIDIsSame(uid129_t uid1, uid129_t uid2)
{
    return (uid1.time == uid2.time &&
            uid1.counter == uid2.counter && 
            uid1.pid == uid2.pid);
}

