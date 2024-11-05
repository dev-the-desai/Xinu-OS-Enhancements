/* active_lock.c - al_initlock, al_lock, al_unlock, al_trylock */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  al_initlock  -  Initialize active lock
 *------------------------------------------------------------------------
 */
syscall al_initlock(
    al_lock_t *l
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
 *  al_lock  -  Acquire the active lock
 *------------------------------------------------------------------------
 */
syscall al_lock(
    al_lock_t *l
)
{
    struct	procent *prptr;
    
    /* Capture the guard */
    while (test_and_set(&l->guard, 1) == 1);
    
    /* Capture the flag, if possible */
    if(l->flag == 0) {
        l->flag = 1;
        l->owner = currpid;
        l->guard = 0;
    }
    /* If not able to capture the lock, enqueue to lock queue and wait */
    else {
        /* update pcb params*/
        prptr = &proctab[currpid];
        prptr->prLockWaitOn = l->owner;
        prptr->prTopark = 1;

        /* Check for deadlocks */
        detect_deadlock(l);

        /* enqueue in waiting list */
        enqueue(currpid, l->lqueue);

        /* park */
        al_setpark();
        l->guard = 0;
        al_park();
    }

    return OK;
}

/*------------------------------------------------------------------------
 *  al_unlock  -  Release the active lock
 *------------------------------------------------------------------------
 */
syscall al_unlock(
    al_lock_t *l
)
{
    struct	procent *prptr;

    /* Capture the guard */
    while (test_and_set(&l->guard, 1) == 1);

    if (l->owner == currpid) {
        if (isempty(l->lqueue)) {
            l->flag = 0;
            l->owner = LOCK_AVAILABLE;
        }
        else {
            prptr = &proctab[currpid];
            prptr->prLockWaitOn = -1;
            al_unpark(dequeue(l->lqueue));
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
 *  al_trylock  -  Checks if lock can be obtained
 *------------------------------------------------------------------------
 */
bool8 al_trylock(
    al_lock_t *l
)
{
    /* Capture the guard */
    while (test_and_set(&l->guard, 0) == 1);

    /* If lock is available, capture it and return true */
    if (l->flag == 0) {
        l->flag = 1;
        l->owner = currpid;
        l->guard = 0;

        return TRUE;
    }
    /* Id lock is not available, return false */
    else {
        l->guard = 0;

        return FALSE;
    }
}   

/*------------------------------------------------------------------------
 *  al_setpark  -  Mark that a process is going to al_park
 *------------------------------------------------------------------------
 */
void al_setpark()
{
    intmask 	mask;    	/* Interrupt mask */
    mask = disable();       /* disable interrupts for atomic-like operation */

    struct	procent *prptr;
    prptr = &proctab[currpid];
    prptr->prTopark = 1;

    restore(mask);          /* enable interrupts */
}

/*------------------------------------------------------------------------
 *  al_park  -  Park a process
 *------------------------------------------------------------------------
 */
void al_park()
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
 *  al_unpark  -  Unpark a process
 *------------------------------------------------------------------------
 */
void al_unpark(
    pid32 pid
)
{
    intmask 	mask;    	/* Interrupt mask */
    mask = disable();       /* disable interrupts for atomic-like operation */

    struct	procent *prptr;
    prptr = &proctab[currpid];
    prptr->prTopark = 0;

    ready(pid);

    restore(mask);          /* enable interrupts */
}

/*------------------------------------------------------------------------------------------
 *  detect_deadlock  -  Detects deadlocks and prints the chain of pids leading to deadlock
 *------------------------------------------------------------------------------------------
 */
void detect_deadlock(
    al_lock_t *l
)
{
    intmask 	mask;    	/* Interrupt mask */
    mask = disable();       /* disable interrupts for atomic-like operation */

    struct  procent *prptr;
    pid32 check_pid = l->owner;
    uint32 deadlock_pid_chain[NALOCKS];
    uint32 deadlock_chain_length = 0;
    bool8  deadlock_detected = FALSE;
    uint32 i = 0;
    
    /* Initialize the deadlock chain with invalid PID */
    for (i = 0; i < NALOCKS; i++)
	{
		deadlock_pid_chain[i] = NPROC + 1;
	}

    /* Check for circular dependencies */

    while ((check_pid != -1))
    {
        prptr = &proctab[check_pid];
        
        //int i = 0;
        //kprintf("D2\n");
        //kprintf("\ncheck_pid: %d, prptr->prLockWaitOn: %d", check_pid, prptr->prLockWaitOn);
        if (prptr->prLockWaitOn == l->owner) {
            //kprintf("Deadlock!");
            deadlock_pid_chain[deadlock_chain_length++] = check_pid;
            deadlock_detected = TRUE;
            break;
        }

        /* Add the current process to the deadlock chain if unique */
        bool8 is_unique = TRUE;
        uint32 j;
        for (j = 0; j < deadlock_chain_length; j++) {
            if (deadlock_pid_chain[j] == check_pid) {
                is_unique = FALSE;
                break;
            }
        }

        if (is_unique) {
            deadlock_pid_chain[deadlock_chain_length++] = check_pid;
        }

        check_pid = prptr->prLockWaitOn;
    }

    /* Sort the deadlock chain if a deadlock was detected */
    if (deadlock_detected) {
        uint32 i;
        for (i = 0; i < deadlock_chain_length - 1; i++) {
            uint32 j;
            for (j = 0; j < deadlock_chain_length - i - 1; j++) {
                if (deadlock_pid_chain[j] > deadlock_pid_chain[j + 1]) {
                    uint32 temp = deadlock_pid_chain[j];
                    deadlock_pid_chain[j] = deadlock_pid_chain[j + 1];
                    deadlock_pid_chain[j + 1] = temp;
                }
            }
        }

        /* Print detected deadlock chain if any */
        kprintf("\ndeadlock_detected=");
        uint32 j;
        for (j = 0; j < deadlock_chain_length; j++) {
            kprintf("P%d", deadlock_pid_chain[j]);
            if (j < deadlock_chain_length - 1) {
                kprintf("-");
            }
        }
    }

    restore(mask);          /* enable interrupts */
}