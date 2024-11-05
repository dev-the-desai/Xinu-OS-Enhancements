/* fork.c - fork, forks new process */

#include <xinu.h>

pid32   fork()
{
    struct procent *childptr;           /* Pointer to child table entry  */
    struct procent *parentptr;          /* Pointer to parent table entry */
    uint32      ssize;                  /* Stack size       */
    uint32      savsp, *pushsp;         /* Stack pointers   */
    uint32      *saddr;                 /* Stack address   */
    pid32       childpid;               /* Child PID        */
    intmask     mask;                   /* Interrupt mask   */
    int32       i;                      /* Iterator         */
    unsigned long   *parent_ebp, *parent_sp;    /* Acts as ESP and EBP pointers */

    /* Assign parent pointer */
    parentptr = &proctab[currpid];
    ssize = parentptr->prstklen;

    /* Stops interrupts from this point onwards */
    mask = disable();

    /* Stack memory assignment */
    if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( (parentptr->prprio < 1) || ((childpid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}

    /* Increment process counter */
    prcount++;

    /* Assign child pointer */
    childptr = &proctab[childpid];

    /* Initialize child parameters */
    childptr->prstate = PR_SUSP;            /* Start in suspended state*/
    childptr->prprio = parentptr->prprio;   /* Get parent's priority   */
    childptr->prstklen = ssize;             /* Get parent's stack size */
    childptr->prstkbase = (char *)saddr;    /* Set base pointer         */
    childptr->prname[PNMLEN-1] = NULLCH;    /* Set the NULL terminator */
    for (i = 0 ; i < PNMLEN-1 && (childptr->prname[i] = parentptr->prname[i] != NULLCH); i++);
    childptr->prsem = -1;                   /* Set default */
    childptr->prparent = (pid32)getpid();   /* Set caller as parent */
    childptr->prhasmsg = FALSE;             /* Set default */
    childptr->user_process = TRUE;          /* User created process */
    /* Set child device descriptors */
    childptr->prdesc[0] = CONSOLE;          /* STDIN  */
	childptr->prdesc[1] = CONSOLE;          /* STDOUT */
	childptr->prdesc[2] = CONSOLE;          /* STDERR */

    /* Beginning stack copying from parent to child */
    savsp = (uint32)saddr;                      /* Save the base of the child's stack to savsp */

    asm("movl %%ebp, %0\n" :"=r"(parent_ebp));  /* Copy ebp from reg for access */

    pushsp = (uint32)(parentptr->prstkbase);    /* Save the value of the base   */
                                                /* of the parent's stack        */

    parent_sp = (unsigned long *)(parentptr->prstkbase);    /* Iterator for looping through */
                                                            /* parent's stack               */

    /* Starting the main loop of copying stack from parent to child */
    /* We start from the base of the parent's stack and copy stack  */
    /* until we reach the EBP, which means we have reached the end  */
    /* of the 2nd last frame i.e. frame just above the fork frame   */
    while (parent_sp >= parent_ebp)
    {   
        /* Check if the data pointed by the parent_sp is same as the location of pushsp */
        if (*parent_sp == pushsp)
        {
            /* savsp->saddr->stackbase */
            pushsp = parent_sp;     /* Store the pointer value  */
            *saddr = savsp;         /* Sets reg EBP's value during exit */
            savsp = saddr;          /* Save the saddr           */
            parent_sp--;            /* Move up in the stack     */
            saddr--;                /* Move up in the stack     */
        }
        /* Or else simply copy the data */
        else
        {
            *saddr = *parent_sp;
            parent_sp--;
            saddr--;
        }
    }

    /* The copying process is over, now we get the child ready for ctxsw    */
    *saddr = 0x00000200;		/* New process runs with interrupts enabled	*/
    
    /* pushal */
    *--saddr = NPROC;	    /* %eax */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (childptr->prstkptr = (char *)saddr);

    /* Readying the child */
    childptr->prstate = PR_READY;                   /* Ready the child      */
    insert(childpid, readylist, childptr->prprio);  /* Insert into the queue*/

    /* Restoring interrupts */
    restore(mask);

    /* Return values */
    if (childpid == SYSERR)         /* Check for errors */
        return SYSERR;
    else if (currpid == childpid)   /* Check if current process is child */
        return NPROC;

    return childpid;    /* Return value for parent */
}