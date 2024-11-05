/* resched.h */

/* Used to prevent a process from being preempted or interrupted */

/* Constants and variables related to deferred rescheduling */

#define	DEFER_START	1	/* Start deferred rescehduling		*/
#define	DEFER_STOP	2	/* Stop  deferred rescehduling		*/


/* MLFQ parameters */
#define UPRIORITY_QUEUES 		3		/* 3 queues*/
#define TIME_ALLOTMENT 			100		/* Time allotment for top queue (lower queues double this amount each level) */
#define PRIORITY_BOOST_PERIOD 	10000000	/* Boost period */

/* Structure that collects items related to deferred rescheduling	*/

struct	defer	{
	int32	ndefers;	/* Number of outstanding defers 	*/
	bool8	attempt;	/* Was resched called during the	*/
						/*   deferral period?			*/
};

extern	struct	defer	Defer;
extern 	bool8	boost_flag;
