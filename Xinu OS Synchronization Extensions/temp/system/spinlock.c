/* spinlock.c - sl_initlock, sl_lock, sl_unlock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  sl_initlock  -  Initialize spinlock
 *------------------------------------------------------------------------
 */
syscall sl_initlock(
    sl_lock_t *l
)
{
    static uint32 lock_count = 0;
    
    if (lock_count > NSPINLOCKS)
    {
        return SYSERR;
    }
    
    l->flag = 0;
    l->owner = LOCK_AVAILABLE;

    lock_count++;

    return OK;
}

/*------------------------------------------------------------------------
 *  sl_lock  -  Acquire the spinlock
 *------------------------------------------------------------------------
 */
syscall sl_lock(
    sl_lock_t *l
)
{
    while (test_and_set(&l->flag, 1) == 1);
    l->owner = currpid;

    return OK;
}

/*------------------------------------------------------------------------
 *  sl_unlock  -  Release the spinlock
 *------------------------------------------------------------------------
 */
syscall sl_unlock(
    sl_lock_t *l
)
{
    if (l->owner == currpid) {
        l->flag = 0;
        l->owner = LOCK_AVAILABLE;
        return OK;
    }
    else {
        return SYSERR;
    }
}
