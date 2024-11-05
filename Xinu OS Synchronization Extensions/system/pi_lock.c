/* pi_lock.c - pi_initlock, pi_lock, pi_unlock */

#include <xinu.h>

void pi_detect_deadlock(pi_lock_t *l);

/*------------------------------------------------------------------------
 *  pi_initlock  -  Initialize lock
 *------------------------------------------------------------------------
 */
syscall pi_initlock(
    pi_lock_t *l
)
{
    static uint32 lock_count = 0;
    static uint32 lock_id = 0;

    if (++lock_count > NPILOCKS)
    {
        return SYSERR;
    }

    l->flag = 0;
    l->guard = 0;
    l->owner = LOCK_AVAILABLE;
    l->lqueue = newqueue();
    l->id = ++lock_id;


    return OK;
}

/*------------------------------------------------------------------------
 *  pi_lock  -  Acquire the lock
 *------------------------------------------------------------------------
 */
syscall pi_lock(
    pi_lock_t *l
)
{
    struct	procent *currptr;
    struct  procent *ownerptr;

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);

    if (l->flag == 0) {
        l->flag = 1;
        l->owner = currpid;
        
        pi_lock_owners_pid[l->id] = currpid;
        pi_lock_highest_prio_waiting_pid[l->id] = 0;

        l->guard = 0;
    }
    else {
        /* enqueue in waiting list */
        enqueue(currpid, l->lqueue);

        //pi_detect_deadlock(l);

        currptr = &proctab[currpid];
        ownerptr = &proctab[l->owner];

        /* Check if waiting process has higher priority than owner */
        if (currptr->prprio > ownerptr->prprio) {
            kprintf("priority_change=P%d::%d-%d\n", l->owner, ownerptr->prprio, currptr->prprio);
            ownerptr->prprio = currptr->prprio;
        }
        
        if (currptr->prprio > pi_lock_highest_prio_waiting_pid[l->id]) {
            pi_lock_highest_prio_waiting_pid[l->id] = currptr->prprio;
        }
        
        /* park */
        pi_setpark();
        l->guard = 0;
        pi_park();
        
        l->owner = currpid;

        if (pi_lock_highest_prio_waiting_pid[l->id] > proctab[l->owner].prprio) {
            kprintf("priority_change=P%d::%d-%d\n", l->owner, proctab[l->owner].prprio, pi_lock_highest_prio_waiting_pid[l->id]);
            proctab[l->owner].prprio = pi_lock_highest_prio_waiting_pid[l->id];
        }
        
    }
    
    return OK;
}

/*------------------------------------------------------------------------
 *  pi_unlock  -  Release the lock
 *------------------------------------------------------------------------
 */
syscall pi_unlock(
    pi_lock_t *l
)
{
    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    
    /* If the owner holds more than 1 lock */
    int i;
    for (i = 1; i < NPILOCKS+1; i++) {
        if ((pi_lock_owners_pid[i] == l->owner) && (i != l->id)) {
            if ((pi_lock_highest_prio_waiting_pid[i] > proctab[currpid].prOldPrio) && (pi_lock_highest_prio_waiting_pid[i] != proctab[currpid].prprio)) {
                kprintf("priority_change=P%d::%d-%d\n", currpid, proctab[currpid].prprio, pi_lock_highest_prio_waiting_pid[i]);
                proctab[currpid].prprio = pi_lock_highest_prio_waiting_pid[i];
                break;
            }
        }
    }
    l->guard = 0;
    
    /* Reset Priority */
    if (i == NPILOCKS+1) {
        if (proctab[currpid].prprio != proctab[currpid].prOldPrio) {
            kprintf("priority_change=P%d::%d-%d\n", currpid, proctab[currpid].prprio, proctab[currpid].prOldPrio);
            proctab[currpid].prprio = proctab[currpid].prOldPrio;
        }
    }
    
    if (l->owner == currpid) {
        
        if (isempty(l->lqueue)) {
            l->flag = 0;
            l->owner = LOCK_AVAILABLE;
        }
        else {
            pi_unpark(dequeue(l->lqueue));
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

void pi_setpark()
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
void pi_park()
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
void pi_unpark(
    pid32 pid
)
{
    intmask 	mask;    	/* Interrupt mask */
    mask = disable();       /* disable interrupts for atomic-like operation */

    struct	procent *prptr;
    prptr = &proctab[pid];
    prptr->prTopark = 0;

    ready(pid);

    restore(mask);          /* enable interrupts */
}

void pi_detect_deadlock(
    pi_lock_t *l
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