#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>


LOCAL int newlock();

/*------------------------------------------------------------------------
 * screate  --  create and initialize a semaphore, returning its id
 *------------------------------------------------------------------------
 */
SYSCALL lcreate()
{
	STATWORD ps;    
	int	lockid;

	disable(ps);
	if ((lockid=newlock())==SYSERR || isbadlock(lockid)) {
		restore(ps);
		return(SYSERR);
	}
	locktab[lockid].lockcnt = 0;
	/* sqhead and sqtail were initialized at system startup */
	restore(ps);
	return(lockid);
}

/*------------------------------------------------------------------------
 * newsem  --  allocate an unused semaphore and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlock()
{
	int	lockid;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		lockid=nextlock--;
			if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (locktab[lockid].lstate==LFREE) {
			//kprintf("\n\n%d\n\n", lockid);
			locktab[lockid].lstate = LUSED;
			return(lockid);
		}
	}
	return(SYSERR);
}
