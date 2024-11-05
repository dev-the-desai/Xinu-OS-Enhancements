/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	mlfq_high;			/* Index of high MLFQ		*/
qid16	mlfq_med;			/* Index of middle MLFQ		*/
qid16	mlfq_low;			/* Index of low MLFQ		*/

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
		//kprintf("\nShould be user pid: %d\n", pid);
		/* If in high */
		if(prptr->prprio == 10) {
			/* If it still has time to run*/
			if(prptr->queue_time < TIME_ALLOTMENT) {
				enqueue(pid, mlfq_high);
			}
			/* If it's time is exhausted*/
			else {
				prptr->downgrades++;
				prptr->prprio = 20;
				prptr->queue_time = 0;
				enqueue(pid, mlfq_med);
			}
			//print_ready_list(mlfq_high);
		}
		/* If in medium */	
		else if(prptr->prprio == 20) {
			/* If it still has time to run*/
			if(prptr->queue_time < TIME_ALLOTMENT*2) {
				enqueue(pid, mlfq_med);
			}
			/* If it's time is exhausted*/
			else {
				prptr->downgrades++;
				prptr->prprio = 30;
				prptr->queue_time = 0;
				enqueue(pid, mlfq_low);
			}
			//print_ready_list(mlfq_med);
		}
		/* If in low */
		else {
			prptr->prprio = 30;
			enqueue(pid, mlfq_low);
			//print_ready_list(mlfq_low);
		}
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
		kprintf("Element: %d , Key: %d \n", curr, queuetab[curr].qkey);
		curr = queuetab[curr].qnext;
	}

	return OK;
}
