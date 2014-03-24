/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
 LOCAL int priochanger2(int prio,int processid);
 
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	struct pentry *sptr; //represents the second process variable, used for checking.
	struct lentry *lptr;
	struct pentry *tptr;
	struct pentry *dptr;
	int checker=0;
	int	dev;
	//kprintf("\nkill called %d\n",pid);
	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	
	/*we need to check what is to be done here. For a process which is leaving, if I am holding any locks then release them., next any wait queues I am in,
	I recalculate what should be the priority of the process. which it is in wait queue. so
	1. change the priority of the process which is holding lock which I am in wait queue. There will be only one process in whose wait queue I will be in. so we fing that.*/
	//
	int processinlock = pptr->waitonlock;
	//kprintf("\n\n%d\n\n",pptr->waitonlock);
	//this gives us the lockid in which the process is waiting. 
	//we check in all the processes waiting 
	if(processinlock!=0){
	//kprintf("\nProcess is in lock lock descreptor :  %d\n", processinlock);
		//now, as we know that process is in some lock's wait queue., we go to that process and we check and change priority and call priority changer.
		lptr = &locktab[processinlock];
		dequeue(pid);//we delete the process from waiting lck. we should also delete other things wh
		//also make anyother entries of holding this lock as null
		lptr->waittype[pid] = 0;
		lptr->prioritytype[pid] = 0;
		pptr->acqlocks[pid]=0;
		pptr->waitonlock=0;
		if(pptr->pinh==0){
	checker = pptr->pprio;
	} else {
	checker = pptr->pinh;
	}
	int iterator2=0;
	
	//weather the inherited priority is set or not.
	//if (checker  = priority of all the process which are holding the lock, 
	//then we take which is the max priority process holding the lock and 
	//next maximum one which is in wait queue, and assign this priority to all the process currently holding the lock and do the same thing as in lock.c
	//1. check if process which have the lock are equal to him or not(imp point all)
	int secondflag=0;
	int iterator=0;

	int i=0;
	lptr = &locktab[processinlock];//lock descriptor for 48
	
	for(iterator=0;iterator<NPROC;iterator++){
			
		if(lptr->processforlock[iterator]==READ || lptr->processforlock[iterator]==WRITE){
			//if any locks has less priority than me, then I do recursion.
			//kprintf("\nlock %d is holded by process %d, \n", processinlock, iterator);
			sptr = &proctab[iterator];
			if(sptr->pinh==-1){
				if(sptr->pprio>checker){
					secondflag=1;
				}
			
			}else{
				if(sptr->pinh>checker)
					secondflag=1;
			}
		
		
		//is there any process which is running with priority greater than me, so no priority inheritance. from me.,		
		if(secondflag==0){
			//kprintf("\n\nFor %d process, others %d", currpid,iterator);
			//this guy has the max priority i.e, this person increased the priority. 
			//noone's priority is less than this guys, so now what to be done.
			//1. find max prio of waiting processes
			
			int maxofwait=0;
			int iterator3=0;
			tptr = &proctab[iterator];
			for(iterator3=0;iterator3<NLOCKS;iterator3++){
				for(iterator2=0;iterator2<NPROC;iterator2++){
				dptr = &proctab[iterator3];
					if(tptr->acqlocks[iterator2]==READ||tptr->acqlocks[iterator2]==WRITE){
					if(dptr->waitonlock == iterator2){
					sptr = &proctab[iterator3];
					//kprintf("\n111Process on lock %d are %d \t %d\n",iterator3,iterator2, maxofwait);
					if(sptr->pinh==0){
						//kprintf("\n\nmax of wait: %d     priority:    %d",maxofwait,sptr->pprio);
						if(maxofwait<sptr->pprio)
							maxofwait = sptr->pprio;
					
					} else{
						if(maxofwait<sptr->pinh)
							maxofwait = sptr->pinh;
					}
				}
				
				}
			}
			
			
			
			
			
			/*
			for(iterator2=0;iterator2<NPROC;iterator2++){
				kprintf("\nProcess: %d\t%d\t%d  on lock %d",iterator2, lptr->waittype[iterator2],lptr->prioritytype[iterator2],iterator3);
				if(lptr->waittype[iterator2]==READ ||lptr->waittype[iterator2]==WRITE){
					sptr = &proctab[iterator2];
					kprintf("\n111Process on lock %d are %d\n",iterator3,iterator2);
					if(sptr->pinh==0){
						if(maxofwait<sptr->pprio)
							maxofwait = sptr->pprio;
					
					} else{
						if(maxofwait<sptr->pinh)
							maxofwait = sptr->pinh;
					}
				
				}
			
			}*/}//after this iteration most probably we will have max priority on waiting queue.
				
				sptr = &proctab[iterator];
				//kprintf("\n\nmax of wait %d   %d\n\n",maxofwait,sptr->pprio);
					if(sptr->pprio<maxofwait){
						sptr->pinh = maxofwait;
						priochanger2(sptr->pinh,iterator);
						//I should change all the processe which inherited priority with this
					} else{
						sptr->pinh=0;
						priochanger2(sptr->pprio,iterator);
						
					}				

		}
		}
		}
	
	}
	
	//2. if all process equal priority to him then we think of other process otherwise not.
	
	
	
	//I am not in any of locks so, let us release all the locks which I am holding.
	int iterator=0;
	for(iterator=0;iterator<NLOCKS;iterator++){
		pptr->callfromkill=1;
		if(pptr->acqlocks[iterator]==READ || pptr->acqlocks[iterator]==WRITE){
			releaseall(1,iterator);
			//releases all the locks which I am holding. one by one, this is one lock release and then acquire other lock. 
			//will rescheduling be a problem?
		}
	
	}
	
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;//if a process is waiting in a queue, then lets remove it and check if this was the best one

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}


LOCAL int priochanger2(int prio,int processid){
//we come here with a process id and a priority
//kprintf("\n\nThere are some more things that are to be done: %d and pid %d", prio,processid);
struct	pentry	*pptr;
int i=0;
	struct	lentry	*lptr;
pptr=&proctab[processid];
if(pptr->waitonlock==-1)
	return OK;
else{
	struct	lentry	*lptr;
	struct pentry *rptr;
	lptr = &locktab[pptr->waitonlock];
	//now we have the lock descreptor for this, and we go through all the process which have this lock currentlly, comparing their priorities and calling this function again.
	for(i=0;i<NLOCKS;i++){
	//if for all the process that are waiting for the lock you have the max priority
		if(lptr->processforlock[i]==READ || lptr->processforlock[i]==WRITE){
		rptr=&proctab[i];
		if(rptr->pinh==0){
			if(rptr->pprio < prio)
			{
				rptr->pinh = prio;
				priochanger2(prio,i);
		//this makes inherited of all the processes as pinh.., now we should check all the issues that this created.
			}
		}else if(rptr->pinh>0){
		
			if(rptr->pinh < prio)
			{
				rptr->pinh = prio;
				priochanger2(prio,i);
		//this makes inherited of all the processes as pinh.., now we should check all the issues that this created.
			}
		
		
		}


}
}
}
}