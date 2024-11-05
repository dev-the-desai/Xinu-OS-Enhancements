/* lock.h */

#define LOCK_AVAILABLE  (-1)

#define NSPINLOCKS  20      /* maximum number of spinlocks  */
#define NLOCKS      20      /* maximum number of locks      */
#define NALOCKS     20      /* maximum number of activelocks*/
#define NPILOCKS    20      /* maximum number of pilocks    */

/* Spinlock primitive */
struct sl_lock_t {
    uint32  flag;
    pid32   owner;
};

/* Lock primitive */
struct lock_t {
    uint32  flag;
    uint32  guard;
    pid32   owner;
    qid16   lqueue;
};

/* Active Lock primitive */
struct al_lock_t {
    uint32  flag;
    uint32  guard;
    pid32   owner;
    qid16   lqueue;
};

/* PI Lock primitive */
struct pi_lock_t {
    uint32  flag;
    uint32  guard;
    pid32   owner;
    qid16   lqueue;
    uint32  id;
};

typedef struct sl_lock_t sl_lock_t;
typedef struct lock_t lock_t;
typedef struct al_lock_t al_lock_t;
typedef struct pi_lock_t pi_lock_t;

pid32 pi_lock_owners_pid[NPILOCKS+1];
pid32 pi_lock_highest_prio_waiting_pid[NPILOCKS+1];