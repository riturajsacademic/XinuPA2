#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>


#define DEFAULT_LOCK_PRIO 20
#define READER_SLEEP 2
#define WRITER_SLEEP 2

int anum = 0;
#define assert(cond)	if (!(cond)) \
kprintf("%d: ASSERT FAIL\n", ++anum); \
else \
kprintf("%d: ASSERT PASS\n", ++anum)
/*--------------------------------Test 1--------------------------------*/
 
void reader1 (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void test1 ()
{
	int	lck;
	int	pid1;
	int	pid2;

	kprintf("\nTest 1: readers can share the rwlock\n");
	lck  = lcreate ();
	assert (lck != SYSERR);

	pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	resume(pid2);
	
	sleep (5);
	ldelete (lck);
	kprintf ("Test 1 finished!\n");
}

/*----------------------------------Test 2---------------------------*/
void reader2 (char *msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);
        kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void writer2 (char *msg, int lck, int lprio)
{
	kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test2 ()
{
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
		"lock acquisition is: reader A, reader B, reader D, writer C & E\n");
        lck  = lcreate ();
        assert (lck != SYSERR);

	rd1 = create(reader2, 2000, 20, "reader2", 3, "reader A", lck, 20);
	rd2 = create(reader2, 2000, 20, "reader2", 3, "reader B", lck, 30);
	rd3 = create(reader2, 2000, 20, "reader2", 3, "reader D", lck, 25);
	rd4 = create(reader2, 2000, 20, "reader2", 3, "reader E", lck, 20);
        wr1 = create(writer2, 2000, 20, "writer2", 3, "writer C", lck, 25);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);


        sleep (10);
        kprintf ("Test 2 finished, check order of acquisition!\n");
}

/*----------------------------------Test 3---------------------------*/
void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR);

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 25);

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	assert (getprio(wr1) == 30);
	
	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep (1);
	assert (getprio(wr1) == 25);

	kprintf("-kill reader A, then sleep 1s\n");
	kill (rd1);
	sleep(1);
	assert(getprio(wr1) == 20);

        sleep (8);
        kprintf ("Test 3 OK\n");
}

int main( )
{
	int i, s;
	int count = 0;
	char buf[8];


	kprintf("Please Input:\n");
	while ((i = read(CONSOLE, buf, sizeof(buf))) <1);
	buf[i] = 0;
	s = atoi(buf);
	switch (s)
	{
	case 1:
		test1();
		break;
	
	case 2:
		test2();
		break;
		
	case 3:
		test3();
		break;
	}
}
