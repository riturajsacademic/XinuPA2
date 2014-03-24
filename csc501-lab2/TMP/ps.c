#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab2.h>
#include <stdio.h>
#include <q.h>


void ps(){

	
	struct pentry *pte; // process table entry
	int i;
	STATWORD ps;
	disable(ps);
	kprintf("\nPName  priority   inherited   pid    onlock");
	for(i=0;i<NPROC;i++){
		pte = &proctab[i];
		if(pte->pstate!=PRFREE)
		{
		printf("\n\n%s\t%d\t%d\t%d\t%d",pte->pname,pte->pprio, pte->pinh,i,pte->waitonlock);
		}

		}	
		//Now we sort the array:
		restore(ps);
		
	
	}

