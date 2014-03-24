#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <assert.h>
#define DEFAULT_LOCK_PRIO 20

int number = 0;
#define assert(condition)	if (!(condition)) \
kprintf("%d: ASSERT FAIL\n", ++number); \
else \
kprintf("%d: ASSERT PASS\n", ++number)


int     lck,lck2;
int 	sem,sem2;
/*SEMAPHORE*/
void readersem3 (char *msg, int sem)
{
        int     ret;

        kprintf ("  %s: to acquire lock %d  \n", msg,currpid);
		
        //lock (lck, READ, DEFAULT_LOCK_PRIO);
		wait(sem);
        kprintf ("  %s: acquired lock\n", msg);
		sleep(5);
        kprintf ("  %s: to release lock\n", msg);
        //releaseall (1, lck);
		signal(sem);
		
}

void writersem3 (char *msg, int sem)
{
        kprintf ("  %s: to acquire lock  %d     \n", msg,currpid);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
		
		//lock(lck2, WRITE, 30);
		wait(sem);
		kprintf("\n\nAcquired both locks, lck2 %d\n\n", lck2);
        sleep (5);
        kprintf ("  %s: to release lock\n", msg);
       // releaseall (1, lck);
	   signal(sem);
		
}

/*SEMAPHORE*/

void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock %d  \n", msg,currpid);
		
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
		sleep(5);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
		
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock  %d     %d\n", msg,currpid, lck);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
		
		lock(lck2, WRITE, 30);
		
		kprintf("\n\nAcquired both locks, lck2 %d\n\n", lck2);
        sleep (5);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
		
}

void test3 ()
{
        
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate();
        lck2 = lcreate();
		kprintf("lck1: %d    \t  lck2: %d",lck,lck2);
        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck2);
        wr1 = create(writer3, 2000, 40, "writer3", 2, "writer C", lck);
		 kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
		sleep (1);
        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
		assert(getprio(rd2)==40);
		assert(getprio(rd1)==25);
		assert(getprio(wr1)==40);
	   
	    sleep (8);
        kprintf ("Test 3 OK\n");
		/*semaphore*/
		
       
		kprintf("\n\nSEMAPHORE	\n\n");
        kprintf("\nTest 3: test the basic priority inheritence\n");
        sem  = screate(2);
        sem2 = screate(1);
		kprintf("lck1: %d    \t  lck2: %d",lck,lck2);
        rd1 = create(readersem3, 2000, 25, "readersem3", 2, "reader A", sem);
        rd2 = create(readersem3, 2000, 30, "readersem3", 2, "reader B", sem2);
        wr1 = create(writersem3, 2000, 40, "writersem3", 2, "writer C", sem);
		 kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
		sleep (1);
        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
		assert(getprio(rd2)==40);
		assert(getprio(rd1)==25);
		assert(getprio(wr1)==40);
	   
	    sleep (8);
        kprintf ("Test 3 OK\n");
		
		
}

