/* lock.h - isbadsem */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define	NLOCKS		50	/* number of locks, if not defined	*/
#endif

#define	LFREE	'\005'		/* this semaphore is free		*/
#define	LUSED	'\008'		/* this semaphore is used		*/

#define READ '\001'
#define WRITE '\002'

extern unsigned long ctr1000;
struct	lentry	{		/* semaphore table entry		*/
	char	lstate;		/* the state SFREE or SUSED		*/
	int	lockcnt;		/* count for this semaphore		*/
	int	lqhead;		/* q index of head of the queue of the current waiting processes.		*/
	int	lqtail;		/* q index of tail of list		*/
	//int cqhead;			//the processes which have this lock.
	//int cqtail;		//represents the current processes which have acquired the lock and are executing.
	int currentlock;	//describes the process which recently acquired lock was of type READ or WRITE.
	int prioritytype[50];	//represents the priority of the process in waiting with. either a READ or WRITE.
	int waittype[50];		//represents was the wait type of process for which process went to Wait..
	int processforlock[50];	// repsents all the processes runing on the queue.
};
extern	struct	lentry	locktab[];
extern	int	nextlock;

#define	isbadlock(s)	(s<0 || s>=NLOCKS)

#endif

