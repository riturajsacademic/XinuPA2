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
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
 LOCAL int priochanger3(int prio,int processid);
 
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
int maxofwait=0;

	disable(ps);
	//kprintf("comming here %d",pid);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		//kprintf("inside");
		restore(ps);
		return(SYSERR);
	}
	//kprintf("commin here with %d and %d    %d    %d", newprio, pid,pptr->pprio,pptr->pinh);
	//here I have checked everything. now first we check if the new priority is greater than old one or not. 
	//if it is greater than the given priority, then we put the priority and we call rescheduling function.
	
	if(pptr->pinh==0){
		if(pptr->pprio<newprio){
			pptr->pprio = newprio;
			priochanger3(pptr->pprio,pid);
		}else if(pptr->pprio>newprio){
			pptr->pprio = newprio;
			//find max on stack and if maxonstack is greater than newprio then assign pinh to it.
					int iterator=0;
					int iterator2=0;
					struct pentry *sptr;
					struct lentry *lptr;
					sptr = &proctab[pid];
					int maxofwait=0;
					for(iterator=0;iterator<NPROC;iterator++){
			
						if(sptr->acqlocks[iterator]==READ || sptr->acqlocks[iterator]==WRITE){
							
							lptr = &locktab[iterator];
							for(iterator2=0;iterator2<NPROC;iterator2++){
							
								if(lptr->waittype[iterator2]==READ ||lptr->waittype[iterator2]==WRITE){
								
								sptr = &proctab[iterator2];
								if(sptr->pinh==0){
								
									if(maxofwait<sptr->pprio){
											//kprintf("\n\nyu should check this  %d   %d\n\n",maxofwait,sptr->pprio);
											maxofwait = sptr->pprio;
											//kprintf("\n\nmax of wait isisis :%d",maxofwait);
										}
								} else{
											if(maxofwait<sptr->pinh)
													maxofwait = sptr->pinh;
										}
				
									}
			
								}
				
						}
					}
					//kprintf("max of wait: %d",maxofwait);
					if(maxofwait>newprio){
						pptr->pinh=maxofwait;
						priochanger3(pptr->pinh, pid);
					} 
		
		}
	}else {
		//now here we have to be more careful as pinh is set.
		//if it is set then anytime it is greater than pprio. so.
		//first we check if inherited prio is >newprio, then assign pprio to newprio.
		if(pptr->pinh>newprio){
			pptr->pprio = newprio;
			//and I do not change anything
		} else if(pptr->pinh<newprio){
			pptr->pinh=0;
			pptr->pprio = newprio;
			priochanger3(pptr->pprio,pid);
		
		}
	
	
	}
	
	
	
	
	
	
	
	
	
	
	
	
			/*
			if(pptr->pprio<newprio || pptr->pinh<newprio){
				if(pptr->pinh<newprio){
				//kprintf("\n\nshould come here.");
				pptr->pinh=0;
				
				priochanger3(pptr->pprio, pid);
				}
			
			} else if(pptr->pprio>newprio || pptr->pinh>newprio){
				if(pptr->pinh==0||pptr->pinh<newprio){
					//this implies that the priority is not inherited. 
					//and what is the priority is of pprio. Now, we check the max priority on 
					//stack and assign it to pinh and change pprio to the low priority.
					
					//now find max priority in stack
					
					int iterator=0;
					int iterator2=0;
					struct pentry *sptr;
					struct lentry *lptr;
					sptr = &proctab[pid];
					for(iterator=0;iterator<NPROC;iterator++){
			
						if(sptr->acqlocks[iterator]==READ || sptr->acqlocks[iterator]==WRITE){
							int maxofwait=0;
							lptr = &locktab[iterator];
							for(iterator2=0;iterator2<NPROC;iterator2++){
								if(lptr->waittype[iterator2]==READ ||lptr->waittype[iterator2]==WRITE){
								sptr = &proctab[iterator2];
								if(sptr->pinh==0){
									if(maxofwait<sptr->pprio)
											maxofwait = sptr->pprio;
					
								} else{
											if(maxofwait<sptr->pinh)
													maxofwait = sptr->pinh;
										}
				
									}
			
								}
				
						}
					}
					if(maxofwait>newprio){
						pptr->pinh=maxofwait;
					} 
	}*/
	
	
	//pptr->pprio = newprio;
	if(pptr -> pstate == PRREADY) {
		dequeue(pid);
		insert(pid, rdyhead, pptr -> pprio);
	}
	restore(ps);
	return(newprio);

}


LOCAL int priochanger3(int prio,int processid){
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
				priochanger3(prio,i);
		//this makes inherited of all the processes as pinh.., now we should check all the issues that this created.
			}
		}else if(rptr->pinh>0){
		
			if(rptr->pinh < prio)
			{
				rptr->pinh = prio;
				priochanger3(prio,i);
		//this makes inherited of all the processes as pinh.., now we should check all the issues that this created.
			}
		
		
		}


}
}
}
}