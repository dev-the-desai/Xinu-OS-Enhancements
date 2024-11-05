/*  main.c  - main */

#include <xinu.h>

#define N 4

// Set to 1 to test that part
int32 P1 = 0;
int32 P2 = 0;
int32 P3 = 0;
int32 P4 = 0;

pid32   pid[N+1];
al_lock_t mutex[N+1];

syscall sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}

void run_for_ms(uint32 time){
	uint32 start = proctab[currpid].runtime;
	while ((proctab[currpid].runtime-start) < time);
}

process p2(al_lock_t *l1, al_lock_t *l2){
    //sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
	al_lock(l1);
	run_for_ms(1000);
	al_lock(l2);		
	run_for_ms(1000);
	al_unlock(l1);
	run_for_ms(1000);
	al_unlock(l2);		
	run_for_ms(1000);
	return OK;
}

process p2_trylock(al_lock_t *l1, al_lock_t *l2){
    //sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
	al_trylock(l1);
	run_for_ms(1000);
	al_trylock(l2);		
	run_for_ms(1000);
	al_unlock(l1);
	run_for_ms(1000);
	al_unlock(l2);		
	run_for_ms(1000);
	return OK;
}

process p2_no_circular_wait(al_lock_t *l1, al_lock_t *l2){
    //sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);

    if (l1 > l2) {
        al_trylock(l2);
	    run_for_ms(1000);
	    al_trylock(l1);		
	    run_for_ms(1000);
	    al_unlock(l2);
	    run_for_ms(1000);
	    al_unlock(l1);		
	    run_for_ms(1000);
    }

    else {	
	    al_trylock(l1);
	    run_for_ms(1000);
	    al_trylock(l2);		
	    run_for_ms(1000);
	    al_unlock(l1);
	    run_for_ms(1000);
	    al_unlock(l2);		
	    run_for_ms(1000);
    }

	return OK;
}

process	main(void)
{
    uint32 i;

    intmask test_mask;


    // Part 1
    if (P1) {
        
        /* initialize al_locks */
	    for (i=0; i<N+1; i++) al_initlock(&mutex[i]);

	    kprintf("\n\n================= PART 1 = Deadlock Detection ===================\n");

        // Circular dependence
        pid[0] = create((void *)p2, INITSTK, 4, "p2", 2, &mutex[0], &mutex[1]);
        pid[1] = create((void *)p2, INITSTK, 4, "p2", 2, &mutex[1], &mutex[2]);
        pid[2] = create((void *)p2, INITSTK, 4, "p2", 2, &mutex[2], &mutex[0]);
        
        // Run all the processes
        for (i = 0; i < 3; i++) {
            kprintf("Creating process with pid: %d\n", pid[i]);
            resume(pid[i]);
        }

        sleepms(5000);
        
        kprintf("\n");
        pid[N] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[N], &mutex[N+1]);
        resume(pid[N]);

        kprintf("P%d completed\n", receive());
    }

    // Part 2
	if (P2) {

        /* initialize al_locks */
	    for (i=0; i<N+1; i++) al_initlock(&mutex[i]);

        kprintf("\n\n================= PART 2 = Hold and Wait Prevention ===================\n");

        for (i = N; i < N+3; i++)
            pid[i] = create((void *)p2, INITSTK, 4,"p2", 2, &mutex[i], &mutex[i+1]);
        pid[N-1] = create((void *)p2, INITSTK, 4,"p2", 2, &mutex[N-1], &mutex[0]);

        // Begin atomic acquisition
        test_mask = disable();

        // Run all the processes
        for (i = N; i < N+3; i++) {
            kprintf("Creating process with pid: %d\n", pid[i]);
            resume(pid[i]);
            sleepms(10);
        }

        // End atomic acquisition
        restore(test_mask);

        kprintf("Deadlock avoided by Hold and Wait prevention\n");

        kprintf("P%d completed\n", receive());
    }

    // Part 3
	if (P3) {

        /* initialize al_locks */
	    for (i=0; i<N+1; i++) al_initlock(&mutex[i]);

        kprintf("\n\n================= PART 3 = Allowing Preemption ===================\n");

        for (i = N; i < N+3; i++)
            pid[i] = create((void *)p2_trylock, INITSTK, 4,"p2", 2, &mutex[i], &mutex[i+1]);
        pid[N-1] = create((void *)p2_trylock, INITSTK, 4,"p2", 2, &mutex[N-1], &mutex[0]);

        // Run all the processes
        for (i = N; i < N+3; i++) {
            kprintf("Creating process with pid: %d\n", pid[i]);
            resume(pid[i]);
            sleepms(10);
        }

        kprintf("Deadlock avoided by allowing preemption\n");

        kprintf("P%d completed\n", receive());
    }

    // Part 4
	if (P4) {

        /* initialize al_locks */
	    for (i=0; i<N+1; i++) al_initlock(&mutex[i]);

        kprintf("\n\n================= PART 4 = Avoiding Circular Wait ===================\n");

        for (i = N; i < N+3; i++)
            pid[i] = create((void *)p2_trylock, INITSTK, 4,"p2", 2, &mutex[i], &mutex[i+1]);
        pid[N-1] = create((void *)p2_trylock, INITSTK, 4,"p2", 2, &mutex[N-1], &mutex[0]);

        // Run all the processes
        for (i = N; i < N+3; i++) {
            kprintf("Creating process with pid: %d\n", pid[i]);
            resume(pid[i]);
            sleepms(10);
        }

        kprintf("Deadlock avoided by avoiding circular wait\n");

        kprintf("P%d completed\n", receive());
    }

    kprintf("TEST OVER");
    
    return OK;
}