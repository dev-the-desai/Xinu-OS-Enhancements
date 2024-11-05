/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	userprocesslist;	/* Index of user process list */

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	
	if (prptr->user_process == FALSE) {
		insert(pid, readylist, prptr->prprio);
	}
	else {
		insert_to_user(pid, userprocesslist, prptr->tickets);
	}
	
	resched();

	return OK;
}

/*---------------------------------------------------------------------------------------------------------------
 *  print_ready_list  -  Prints queue contents but unlike the name suggests, 
 *			 			 it can be used to print any queue
 *--------------------------------------------------------------------------------------------------------------
 */
syscall print_ready_list(qid16 q)
{
	qid16 curr;			/* index iterator */
	
	if(isbadqid(q))
		return SYSERR;
	if (isempty(q)) {
		kprintf("Queue is empty...");
		return OK;
	}

	curr = firstid(q);
	
	while (curr != queuetail(q))
	{
		kprintf("Element %d: , Key: %d \n", curr, queuetab[curr].qkey);
		curr = queuetab[curr].qnext;
	}

	return OK;
}