/*  main.c  - main */

#include <xinu.h>

uint32 sum2 (uint32 a, uint32 b)
{
	kprintf("Working...");
	return a+b;
}

uint32 sum3 (uint32 a, uint32 b, uint32 c)
{
	kprintf("Working...");
	return sum2(a, b) + c;
}

process	main(void)
{
	pid32	shpid;		/* Shell process ID */
	
	pid32	t1pid;		/* Test process 1*/
	pid32	t2pid;		/* Test process 2*/
	pid32	t3pid;		/* Test process 3*/
	pid32	t4pid;		/* Test process 4*/
	pid32	t5pid;		/* Test process 5*/
	pid32	t6pid;		/* Test process 6*/
	pid32	t7pid;		/* Test process 7*/
	pid32	t8pid;		/* Test process 8*/
	pid32	t9pid;		/* Test process 9*/
	pid32	t10pid;		/* Test process 10*/
	pid32	t11pid;		/* Test process 11*/
	pid32	t12pid;		/* Test process 12*/
	pid32	t13pid;		/* Test process 13*/
	pid32	t14pid;		/* Test process 14*/
	pid32	t15pid;		/* Test process 15*/

	uint32	i;			/* iterator */

	printf("\n\n");

	/* Create a local file system on the RAM disk */

	lfscreate(RAM0, 40, 20480);

	/* Run the Xinu shell */

	recvclr();
	resume(shpid = create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Creating process for testing */
	t1pid = create(sum3, 8192, 50, "Test1", 3, 2, 3, 4);
	proctab[t1pid].user_process = TRUE;

	t2pid = create(sum2, 8192, 50, "Test2", 2, 2, 3);
	proctab[t2pid].user_process = TRUE;
	proctab[t2pid].prparent = t1pid;

	t3pid = create(sum2, 8192, 50, "Test3", 2, 2, 3);
	proctab[t3pid].user_process = TRUE;
	proctab[t3pid].prparent = t1pid;

	t4pid = create(sum2, 8192, 50, "Test4", 2, 2, 3);
	proctab[t4pid].user_process = TRUE;
	proctab[t4pid].prparent = t2pid;

	t5pid = create(sum2, 8192, 50, "Test5", 2, 2, 3);
	proctab[t5pid].user_process = TRUE;
	proctab[t5pid].prparent = t2pid;

	t6pid = create(sum2, 8192, 50, "Test6", 2, 2, 3);
	proctab[t6pid].user_process = TRUE;
	proctab[t6pid].prparent = t3pid;

	t7pid = create(sum2, 8192, 50, "Test7", 2, 2, 3);
	proctab[t7pid].user_process = TRUE;
	proctab[t7pid].prparent = t4pid;

	t8pid = create(sum2, 8192, 50, "Test8", 2, 2, 3);
	proctab[t8pid].user_process = TRUE;
	proctab[t8pid].prparent = t4pid;

	t9pid = create(sum2, 8192, 50, "Test9", 2, 2, 3);
	proctab[t9pid].user_process = TRUE;
	proctab[t9pid].prparent = t6pid;

	t10pid = create(sum2, 8192, 50, "Test10", 2, 2, 3);
	proctab[t10pid].user_process = TRUE;
	proctab[t10pid].prparent = t8pid;

	t11pid = create(sum2, 8192, 50, "Test11", 2, 2, 3);
	proctab[t11pid].user_process = TRUE;
	proctab[t11pid].prparent = t10pid;

	t12pid = create(sum2, 8192, 50, "Test12", 2, 2, 3);
	proctab[t12pid].user_process = TRUE;
	proctab[t12pid].prparent = t10pid;

	t13pid = create(sum2, 8192, 50, "Test13", 2, 2, 3);
	proctab[t13pid].user_process = TRUE;
	proctab[t13pid].prparent = t12pid;

	t14pid = create(sum2, 8192, 50, "Test14", 2, 2, 3);
	proctab[t14pid].user_process = TRUE;
	proctab[t14pid].prparent = t12pid;

	t15pid = create(sum2, 8192, 50, "Test15", 2, 2, 3);
	proctab[t15pid].user_process = TRUE;
	proctab[t15pid].prparent = t14pid;

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
	    if (receive() == shpid) {
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(shpid = create(shell, 4096, 20, "shell", 1, CONSOLE));
	    }
	}
	return OK;
}
