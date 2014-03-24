#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

unsigned long ctr1000;
/*Locks the process has the following functions:

queueempty(int lockid) returns 0 if the queue is empty i.e, no process is waiting in queue, 1 otherwise.
AllowRun() allows a process to run and does all the background process: including:
1. put in process table that the lock is associated with this proces.
2. put this process in the array held by locktable that this process currently holds the lock.
3. sets the current lock type.
4.  bus

ToBlock()
does the same function except removes the thing.

writepriority return 0 if no process with wait has more priority than this lock priority.

*/
LOCAL int priochanger(int prio,int processid);
LOCAL int queueempty(int ldesc);
LOCAL void AllowRun(int ldesc,int type, int priority);
LOCAL void ToBlock(int ldesc, int type, int priority);
LOCAL int writepriority(int ldesc, int priority);


int lock(int lockid,int type, int priority){
	STATWORD ps;    
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	pptr = &proctab[currpid];
	lptr = &locktab[lockid];
	disable(ps);
	if (isbadlock(lockid) || (lptr= &locktab[lockid])->lstate==LFREE) {
		restore(ps);
		
		return(SYSERR);
	}
	if(type==READ){
		//if the type is read
		//kprintf("READ with PID %d   %d",currpid,lockid);
		if(queueempty(lockid)==0 || lptr->currentlock==READ){
			//if either the queue is emptr or all processes accessing lock are READ
			//give process the lock and make updating entries.
			
			if(writepriority(lockid,priority)==1){
			//kprintf("\nMAX priority lock");
				ToBlock(lockid,type,priority);
				resched();
				restore(ps);
				return pptr->lockretvalue;
			
			}
			else{
				//kprintf("\n Allow Run: %d\n",lockid);
				AllowRun(lockid,type,priority);
				restore(ps);
				return OK;
				
			}
		} else if(lptr->currentlock==WRITE){
				
				ToBlock(lockid,type,priority);
				resched();
				restore(ps);
				return pptr->lockretvalue;
		
		}
	
	
	}
	else if(type==WRITE){
	//kprintf("\nWrite with PID %d   %d     %d\n",currpid,queueempty(lockid),lockid);
	
		if(queueempty(lockid)==0){
		
				AllowRun(lockid,type,priority);
				restore(ps);
				return OK;
		}else{
				//kprintf("\nto block 3");
				ToBlock(lockid,type,priority);
				resched();
				restore(ps);
				return pptr->lockretvalue;
		
		
		}
	
	
	
	}

}

LOCAL void AllowRun(int ldesc, int type, int priority){
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	pptr = &proctab[currpid];
	lptr = &locktab[ldesc];
	pptr->acqlocks[ldesc] = type;
	//kprintf("\nlock: %d, %d, %d",pptr->acqlocks[ldesc],ldesc,currpid);
	//thus keeps track of the process and its type;
	lptr->processforlock[currpid]=type;
	//kprintf("making it 1 %d",ldesc);
	lptr->currentlock = type;
	lptr->lockcnt++;
	

}

LOCAL void ToBlock(int ldesc,int type, int priority){
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	struct pentry *rptr;
	int i;
	int flag=0;
	int checker=0;
	pptr = &proctab[currpid];
	lptr = &locktab[ldesc];
	pptr->pstate=PRWAIT;
	insert(currpid, lptr->lqhead, priority);
	lptr->waittype[currpid] = type;
	//kprintf("\nMake %d with %d wait on %d",lptr->waittype[currpid],ldesc,currpid);
	lptr->prioritytype[currpid] = priority;//this priority is of the lock.
	pptr->lockretvalue = OK;
	pptr->waitonlock = ldesc;
	pptr->timeinqueue=ctr1000;
	
	//In here we have to implement priority inheritance, so we woill have a recursive thing. We call a function which will 
	//before calling recursive fn, 
	//initially I check here if wether the process which comes has highest priority than currently running process which has the lock.
	//if it is, then I call a recursive fn.
	if(pptr->pinh==0){
	checker = pptr->pprio;
	} else {
	checker = pptr->pinh;
	}
	//weather the inherited priority is set or not.
	for(i=0;i<NLOCKS;i++){
	//if for all the process that are waiting for the lock you have the max priority
		if(lptr->processforlock[i]==READ || lptr->processforlock[i]==WRITE){
		rptr=&proctab[i];
		if( rptr->pinh==-1){
				if(rptr->pprio>checker){
					flag=1;
				}
			}
			else if(rptr->pinh>0){
				if(rptr->pinh>checker){
					flag=1;
				}
			
			}
		}
	}
	//checks weather the priority is max of all the locks, which are in wait. 
	//now by value of flag we know what is happenning., if flag == 1, then we do not do priority inheritance.
	//here we are putting this guys priority to max.
	if(flag==0){
		//now we have to do priority inheritance. 
		//we make priority of all the process which are running pinh, and then what do we do?, then we check the lock in which those processes are waiting.
		for(i=0;i<NLOCKS;i++){
	//if for all the process that are waiting for the lock you have the max priority
		if(lptr->processforlock[i]==READ || lptr->processforlock[i]==WRITE){
		rptr=&proctab[i];
			
			if(rptr->pinh==0){
			if(checker>rptr->pprio){
				rptr->pinh = checker;
				priochanger(checker,i);
			}} else {
				if(checker>rptr->pinh){
					rptr->pinh = checker;
					priochanger(checker,i);
				
				}
			
			}
			
			
//if it is the max priority already we do not change the priority of that process			//we pass the new priority of process and the processid
		//this makes inherited of all the processes as pinh.., now we should check all the issues that this created.
			}
		}
	}
	
}
//and because priort of this proicess us changed
//I need to put this function in a file
LOCAL int priochanger(int prio,int processid){
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
				priochanger(prio,i);
		//this makes inherited of all the processes as pinh.., now we should check all the issues that this created.
			}
		}else if(rptr->pinh>0){
		
			if(rptr->pinh < prio)
			{
				rptr->pinh = prio;
				priochanger(prio,i);
		//this makes inherited of all the processes as pinh.., now we should check all the issues that this created.
			}
		
		
		}


}
}
}
}
LOCAL int queueempty(int ldesc){
struct	lentry	*lptr;
lptr = &locktab[ldesc];
int flag = 0;
int i=0;
for(i=0;i<NLOCKS;i++){
if(lptr->processforlock[i]==READ||lptr->processforlock[i]==WRITE){
	//kprintf("\nfor process %d   %d\n",i,lptr->processforlock[i]);
	flag = 1;

	}
}
return flag;
}

LOCAL int writepriority(int ldesc, int priority){
struct	lentry	*lptr;
lptr = &locktab[ldesc];
int flag = 0;
int i=0;
for(i=0;i<NLOCKS;i++){
	if(lptr->waittype[i]==WRITE)
		if(lptr->prioritytype[i] > priority){
			//as the priority of write process is more, we should put the proces in wait queue.
			flag=1;
		}

}
return flag;

}