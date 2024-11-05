/* clock.h */

/* Intel 8254-2 clock chip constants */

#define	CLOCKBASE	0x40		/* I/O base port of clock chip	*/
#define	CLOCK0		CLOCKBASE
#define	CLKCNTL		(CLOCKBASE+3)	/* chip CSW I/O port		*/


#define CLKTICKS_PER_SEC  1000		/* clock timer resolution	*/

extern	uint32	clktime;		/* second since system boot	*/
extern  uint32	ctr1000;        /* Milliseconds since boot	*/
extern  uint32	count1000;		/* ticks since clktime		*/
extern  uint32  priority_boost_timer;   /* priority boost timer*/

extern	qid16	sleepq;			/* queue for sleeping processes	*/
extern	uint32	preempt;		/* preemption counter		*/

