#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <mem.h>
#include <io.h>
#include <stdio.h>
#include <q.h>


LOCAL int release(int lockid);
LOCAL int queueempty(int ldesc);
LOCAL int makeready(int processid,int lockid,int type);
/*remember to include disable and other stuff.*/


SYSCALL releaseall(nargs, args){
	unsigned long	*a;		/* points to list of args	*/
	STATWORD ps;    	
	struct pentry *pptr;
	disable(ps);
	//kprintf("\n in release.");
	a = (unsigned long *)(&args) -1; /* last argument	*/
	int arr[*a];
	int i=0;
	
	 for ( ; nargs > 0 ; nargs--)	{/* machine dependent; copy args	*/
		*a++;	/* onto created process' stack	*/
		 //kprintf("\n Lock to release: %d",*(a+i));
		 arr[i] = release(*(a+i));//1 returns then syserr., 
		 i++;
	}
	
	int flag = 0;
	int j=0;
	for(j=0;j<i;j++){
		if(arr[j]!=0)
			flag = 1;

	}
	//kprintf("\nAbout to reschedule");
	if((pptr=&proctab[currpid])->callfromkill==0)
		resched();
	
	restore(ps);
	//kprintf("\n\nall released: %d", flag);
	if(flag==1)
		return(SYSERR);
	return OK;
}

LOCAL int release(int lockid){
//first we check 
	//kprintf("\nin release with lockid %d", lockid);
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	pptr = &proctab[currpid];
	lptr = &locktab[lockid];
	//let us first check weather this lock is held by this process or not.
	int flag = 1;
	int i=0;
	// for( i =0;i<NLOCKS;i++){
		// if((pptr->acqlocks[i] == READ || pptr->acqlocks[i]==WRITE) && i==lockid)
			// flag = 0;
	// }
	if((pptr->acqlocks[lockid] == READ || pptr->acqlocks[lockid]==WRITE))
			 flag = 0;
		//kprintf("\n value: %d %d  %d\n",pptr->acqlocks[lockid],lockid,currpid);
	//kprintf("\nFlag: %d",flag);
	if(flag==1)
		return flag;
	else{
		//kprintf("\nMatched the process.");
		//so now we have checked weather the lock is valid or not, now we release this lock.
		//we take one/other process  should be given the lock.
		
			pptr->acqlocks[lockid] = 0;
			//thus keeps track of the process and its type;
			lptr->processforlock[currpid]=0;//makes it 0, setting that this process is not executing.
			lptr->lockcnt--;
		if(isempty(lptr->lqhead))
		{
			//kprintf("\nqueue is empty");
			//if the queue is empty then we should make the entries of that process as null, as this process should release this lock.
			
			////we set it neither to read nor write.
			//it can also happen that the wait queue is empty but there are current ready process holding the lock
			if(lptr->lockcnt==0){
				
				lptr->currentlock = 0;
				return 0;
				}
			else{
			//kprintf("\nfirst %d",lptr->lockcnt);
				return 0;
				
			}
		}else{
		//kprintf("\nNOT empty");
			//if the queue is nonempty there is/ are other proces which want to have the lock, we schedule them and take them out of the queue.
			//2 cases in this scenario:
			//1. the current process was write, so we should give lock to other processes in the queue.
			//2. the current process was read, and there are other processes waiting in queue. implying this process is write., so we give lock to this write proces based on things.
			if(lptr->currentlock == READ||lptr->currentlock==WRITE){
				//then there is going to be either one writer and there might be processes whic are higher priories.
				//firstly are there equal priority processes, and if there are then how much.
				
				//lets check what is priority of our head.
				int maxpriority = lastkey(lptr->lqtail);//if this returns the process id
				int iterator=0;
				int j=0;
				int samepriorityprocess=0;
				int priorityarr[50];
				//now, we check how many process with this priority are in the queue.
				for(iterator=0;iterator<50;iterator++){
					if(iterator!=maxpriority){
						
						if(lptr->prioritytype[i]==lptr->prioritytype[maxpriority]){
							samepriorityprocess++;
								priorityarr[j] = i;
								j++;
							}
					
					}
				
				}
				int rcompare=0;
				int rtemp=0;
				int wcompare=0;
				int wtemp=0;
				int witerator=0;
				int riterator=0;
				//after this we know the number of processes with same priority.
				if(samepriorityprocess==0){
					//there are no process with this max prioroty, we start this process.
					//make this process ready and return OK, saying that the lock was released.
					/*We need to change a lot of things here.*/
					makeready(maxpriority,lockid,lptr->waittype[maxpriority]);
					return 0;
				} else if(samepriorityprocess>0){
					//we check the time of running. for all same priority process
					//We difrenciate between read and write or not?
					for(iterator=0;iterator<j;iterator++){
						pptr = &proctab[iterator];
						if(lptr->waittype[iterator]==READ){
							if((rtemp = ctr1000-(pptr->timeinqueue))>rcompare){
								rcompare = rtemp;
								riterator = iterator;
							}
						} else if(lptr->waittype[iterator]==WRITE){
							if((wtemp = ctr1000-(pptr->timeinqueue))>wcompare){
								wcompare = wtemp;
								witerator=iterator;
							}
						
						}
					}
					/*to implement this.*/
					//after this we have the max time the process has been in priority.
					if(wcompare-rcompare>=-1){
						//we give write process the right to run, with max priority.
						makeready(witerator,lockid,lptr->waittype[witerator]);
						return 0;
					}
					else if(wcompare-rcompare<-1){
						//as read process waited for more time than write, we ready all read process with max priority.
						for(iterator=0;iterator<j;iterator++){
							if(lptr->waittype[iterator]==READ){
								makeready(iterator,lockid,lptr->waittype[iterator]);
								
							}
						}
						return 0;
					} 
				
				}
				
				
			 }
		
		}
	
	
	
	
	}
}
LOCAL int makeready(int processid,int lockid,int type){
	//before rescheduling we should erase write footprint from array.
	//kprintf("\nCall to make ready\n");
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	pptr = &proctab[processid];
	lptr = &locktab[lockid];
	lptr->waittype[processid] = 0;
	lptr->prioritytype[processid] = 0;
	pptr->lockretvalue = OK;
	pptr->timeinqueue=0;
	//kprintf("\nPID %d",processid);
	ready(dequeue(processid), RESCHNO); //we cannot have duplicate entries in queues.
	pptr->acqlocks[lockid] = type;
	//thus keeps track of the process and its type;
	lptr->processforlock[processid]=type;
	lptr->currentlock = type;
	lptr->lockcnt++;
	//makes the proicess ready with no reschedule
	//we call reschedule once all is done.

}



LOCAL int queueempty(int ldesc){
struct	lentry	*lptr;
lptr = &locktab[ldesc];
int flag = 0;
int i=0;
for(i=0;i<NLOCKS;i++){
if(lptr->processforlock[i]!=0)
	flag = 1;

}
return flag;
}