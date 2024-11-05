/* lock.c - initlock, lock, unlock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  initlock  -  Initialize lock
 *------------------------------------------------------------------------
 */
syscall initlock(
    lock_t *l
)
{
    static uint32 lock_count = 0;
    

    if (lock_count > NLOCKS)
    {
        return SYSERR;
    }

    l->flag = 0;
    l->guard = 0;
    l->owner = LOCK_AVAILABLE;
    l->lqueue = newqueue();

    lock_count++;

    return OK;
}

/*------------------------------------------------------------------------
 *  lock  -  Acquire the lock
 *------------------------------------------------------------------------
 */
syscall lock(
    lock_t *l
)
{
    struct	procent *prptr;

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);

    if (l->flag == 0) {
        l->flag = 1;
        l->owner = currpid;
        l->guard = 0;
    }
    else {
        enqueue(currpid, l->lqueue);
        prptr = &proctab[currpid];
        prptr->prTopark = 1;
        setpark();
        l->guard = 0;
        park();
        l->owner = currpid;
    }

    return OK;
}

/*------------------------------------------------------------------------
 *  unlock  -  Release the lock
 *------------------------------------------------------------------------
 */
syscall unlock(
    lock_t *l
)
{
    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);

    if (l->owner == currpid) {
        if (isempty(l->lqueue)) {
            l->flag = 0;
            l->owner = LOCK_AVAILABLE;
        }
        else {
            unpark(dequeue(l->lqueue));
        }
        l->guard = 0;

        return OK;
    }
    else {
        l->guard = 0;

        return SYSERR;
    }
}

/*------------------------------------------------------------------------
 *  setpark  -  Mark that a process is going to park
 *------------------------------------------------------------------------
 */
void setpark()
{
    intmask 	mask;    	/* Interrupt mask */
    mask = disable();       /* disable interrupts for atomic-like operation */

    struct	procent *prptr;
    prptr = &proctab[currpid];
    prptr->prTopark = 1;

    restore(mask);          /* enable interrupts */
}

/*------------------------------------------------------------------------
 *  park  -  Park a process
 *------------------------------------------------------------------------
 */
void park()
{
    intmask 	mask;    	/* Interrupt mask */
    mask = disable();       /* disable interrupts for atomic-like operation */

    struct	procent *prptr;
    prptr = &proctab[currpid];

    if (prptr->prTopark == 1) {
        prptr->prstate = PR_WAIT;
        prptr->prTopark = 0;
        resched();
    }
    
    restore(mask);          /* enable interrupts */
}

/*------------------------------------------------------------------------
 *  unpark  -  Unpark a process
 *------------------------------------------------------------------------
 */
void unpark(pid32 pid)
{
    intmask 	mask;    	/* Interrupt mask */
    mask = disable();       /* disable interrupts for atomic-like operation */

    struct	procent *prptr;
    prptr = &proctab[currpid];
    prptr->prTopark = 0;

    ready(pid);

    restore(mask);          /* enable interrupts */
}