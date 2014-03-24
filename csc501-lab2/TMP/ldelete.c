#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>


SYSCALL ldelete(int lockid)
{
	STATWORD ps;    
	int	pid;
	struct	lentry	*lptr;

	disable(ps);
	//checks if the lock is Free or not., if it is free then returns ERRor
	if (isbadlock(lockid) || locktab[lockid].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locktab[lockid];
	//kprintf("\nWe delete the lock\n");
	lptr->lstate = LFREE;
	if (nonempty(lptr->lqhead)) {
	//all the processes which are waiting in the queue for this lock should be deleted from queue and processes which have this lock should return error as SFREE is there.
		while( (pid=getfirst(lptr->lqhead)) != EMPTY) // just for deleating we take from head.
		  {
		    proctab[pid].lockretvalue = DELETED;
			
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);
}
