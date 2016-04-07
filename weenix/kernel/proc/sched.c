/******************************************************************************/
/* Important Fall 2015 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "globals.h"
#include "errno.h"

#include "main/interrupt.h"

#include "proc/sched.h"
#include "proc/kthread.h"

#include "util/init.h"
#include "util/debug.h"

static ktqueue_t kt_runq;

static __attribute__((unused)) void
sched_init(void)
{
        sched_queue_init(&kt_runq);
}
init_func(sched_init);



/*** PRIVATE KTQUEUE MANIPULATION FUNCTIONS ***/
/**
 * Enqueues a thread onto a queue.
 *
 * @param q the queue to enqueue the thread onto
 * @param thr the thread to enqueue onto the queue
 */
static void
ktqueue_enqueue(ktqueue_t *q, kthread_t *thr)
{
        KASSERT(!thr->kt_wchan);
        list_insert_head(&q->tq_list, &thr->kt_qlink);
        thr->kt_wchan = q;
        q->tq_size++;
}

/**
 * Dequeues a thread from the queue.
 *
 * @param q the queue to dequeue a thread from
 * @return the thread dequeued from the queue
 */
static kthread_t *
ktqueue_dequeue(ktqueue_t *q)
{
        kthread_t *thr;
        list_link_t *link;

        if (list_empty(&q->tq_list))
                return NULL;

        link = q->tq_list.l_prev;
        thr = list_item(link, kthread_t, kt_qlink);
        list_remove(link);
        thr->kt_wchan = NULL;

        q->tq_size--;

        return thr;
}

/**
 * Removes a given thread from a queue.
 *
 * @param q the queue to remove the thread from
 * @param thr the thread to remove from the queue
 */
static void
ktqueue_remove(ktqueue_t *q, kthread_t *thr)
{
        KASSERT(thr->kt_qlink.l_next && thr->kt_qlink.l_prev);
        list_remove(&thr->kt_qlink);
        thr->kt_wchan = NULL;
        q->tq_size--;
}

/*** PUBLIC KTQUEUE MANIPULATION FUNCTIONS ***/
void
sched_queue_init(ktqueue_t *q)
{
        list_init(&q->tq_list);
        q->tq_size = 0;
}

int
sched_queue_empty(ktqueue_t *q)
{
        return list_empty(&q->tq_list);
}

/*
 * Updates the thread's state and enqueues it on the given
 * queue. Returns when the thread has been woken up with wakeup_on or
 * broadcast_on.
 *
 * Use the private queue manipulation functions above.
 */
void
sched_sleep_on(ktqueue_t *q)
{
        /*NOT_YET_IMPLEMENTED("PROCS: sched_sleep_on");*/
        /*dbg(DBG_PRINT,"*****************************************Entering sched_sleep_on\n");*/

	dbg(DBG_PRINT,"(GRADING1C)\n");
	curthr->kt_state=KT_SLEEP;
        ktqueue_enqueue(q,curthr);
        sched_switch();
        /*dbg(DBG_PRINT,"*****************************************Leaving sched_sleep_on\n");*/

}


/*
 * Similar to sleep on, but the sleep can be cancelled.
 *
 * Don't forget to check the kt_cancelled flag at the correct times.
 *
 * Use the private queue manipulation functions above.
 */
int
sched_cancellable_sleep_on(ktqueue_t *q)
{
        /*NOT_YET_IMPLEMENTED("PROCS: sched_cancellable_sleep_on");*/
	dbg(DBG_PRINT,"(GRADING1C)\n");
	/*dbg(DBG_PRINT,"*****************************************Entering sched_cancellable_sleep_on\n");*/
	if (curthr->kt_cancelled == 1)
        {
          dbg(DBG_PRINT,"(GRADING1C)\n");
          return -EINTR;
        }
        curthr->kt_state=KT_SLEEP_CANCELLABLE;
        ktqueue_enqueue(q,curthr);
        sched_switch();
        if (curthr->kt_cancelled == 1)
        {
          dbg(DBG_PRINT,"(GRADING1C)\n");
          return -EINTR;
        }
        else
            return 0;

	/* dbg(DBG_PRINT,"*****************************************Leaving sched_cancellable_sleep_on\n");*/

        return 0;
}

kthread_t *
sched_wakeup_on(ktqueue_t *q)
{
        /*NOT_YET_IMPLEMENTED("PROCS: sched_wakeup_on");*/
	/*dbg(DBG_PRINT,"*****************************************Entering sched_wakeup_on\n");*/
	kthread_t * thr;
/*
	if(sched_queue_empty(q))
        {
		dbg(DBG_PRINT,"(GRADING1E 4.5 sched_wakeup_on(): Queue empty)\n");
		return NULL;
	}
        else
	{ */
        thr=ktqueue_dequeue(q);
		if(thr!=NULL)
		{
			KASSERT((thr->kt_state == KT_SLEEP) || (thr->kt_state == KT_SLEEP_CANCELLABLE));
			dbg(DBG_PRINT,"(GRADING1A 4.a)\n");
			sched_make_runnable(thr);
        	}

  /*}*/
	return thr;
}

void
sched_broadcast_on(ktqueue_t *q)
{
        /*  NOT_YET_IMPLEMENTED("PROCS: sched_broadcast_on"); */
        /*wake up all threads in q queue*/
        dbg(DBG_PRINT,"(GRADING1C)\n");

	kthread_t * thr;
  	/*dbg(DBG_PRINT,"*****************************************Entering sched_broadcast_on\n");*/

	while(!sched_queue_empty(q))
        sched_wakeup_on(q);
       /*dbg(DBG_PRINT,"*****************************************Leaving sched_broadcast_on\n");*/
}

/*
 * If the thread's sleep is cancellable, we set the kt_cancelled
 * flag and remove it from the queue. Otherwise, we just set the
 * kt_cancelled flag and leave the thread on the queue.
 *
 * Remember, unless the thread is in the KT_NO_STATE or KT_EXITED
 * state, it should be on some queue. Otherwise, it will never be run
 * again.
 */
void
sched_cancel(struct kthread *kthr)
{
        /*NOT_YET_IMPLEMENTED("PROCS: sched_cancel");*/
        /*Cody comments: when we call sched_cancel, it means the thread MUST NOT be
        the current thread, and it MUST be in a queue.
        In either two conditions of the comments above, we should set the cancelled flag to 1*/
        /*dbg(DBG_PRINT,"*****************************************Entering sched_cancel\n");*/


	kthr->kt_cancelled=1;
	dbg(DBG_PRINT,"(GRADING1C)\n");

	if(kthr->kt_state==KT_SLEEP_CANCELLABLE){
	 dbg(DBG_PRINT,"(GRADING1C)\n");
         ktqueue_remove(kthr->kt_wchan,kthr);
         sched_make_runnable(kthr);
        }
        /*dbg(DBG_PRINT,"*****************************************Leaving sched_cancel_cancel\n");*/

}

/*
 * In this function, you will be modifying the run queue, which can
 * also be modified from an interrupt context. In order for thread
 * contexts and interrupt contexts to play nicely, you need to mask
 * all interrupts before reading or modifying the run queue and
 * re-enable interrupts when you are done. This is analagous to
 * locking a mutex before modifying a data structure shared between
 * threads. Masking interrupts is accomplished by setting the IPL to
 * high.
 *
 * Once you have masked interrupts, you need to remove a thread from
 * the run queue and switch into its context from the currently
 * executing context.
 *
 * If there are no threads on the run queue (assuming you do not have
 * any bugs), then all kernel threads are waiting for an interrupt
 * (for example, when reading from a block device, a kernel thread
 * will wait while the block device seeks). You will need to re-enable
 * interrupts and wait for one to occur in the hopes that a thread
 * gets put on the run queue from the interrupt context.
 *
 * The proper way to do this is with the intr_wait call. See
 * interrupt.h for more details on intr_wait.
 *
 * Note: When waiting for an interrupt, don't forget to modify the
 * IPL. If the IPL of the currently executing thread masks the
 * interrupt you are waiting for, the interrupt will never happen, and
 * your run queue will remain empty. This is very subtle, but
 * _EXTREMELY_ important.
 *
 * Note: Don't forget to set curproc and curthr. When sched_switch
 * returns, a different thread should be executing than the thread
 * which was executing when sched_switch was called.
 *
 * Note: The IPL is process specific.
 */
void
sched_switch(void)
{
    /*    NOT_YET_IMPLEMENTED("PROCS: sched_switch");*/
        /*dbg(DBG_PRINT,"*****************************************Entering sched_switch\n");*/

	dbg(DBG_PRINT,"(GRADING1C)\n");

	uint8_t oldipl=intr_getipl();
        kthread_t *oldkthread;
        intr_setipl(IPL_HIGH);
        while(sched_queue_empty(&kt_runq)){
          intr_setipl(IPL_LOW);
          intr_wait();
          intr_setipl(IPL_HIGH);
        }
        oldkthread = curthr;
        /*dbg(DBG_PRINT,"Run queue empty %d\n",sched_queue_empty(&kt_runq));*/
        curthr=ktqueue_dequeue(&kt_runq);
        curproc=curthr->kt_proc;
        context_switch(&oldkthread->kt_ctx,&curthr->kt_ctx);
        intr_setipl(oldipl);
        /*dbg(DBG_PRINT,"*****************************************Leaving sched_switch\n");*/
}

/*
 * Since we are modifying the run queue, we _MUST_ set the IPL to high
 * so that no interrupts happen at an inopportune moment.

 * Remember to restore the original IPL before you return from this
 * function. Otherwise, we will not get any interrupts after returning
 * from this function.
 *
 * Using intr_disable/intr_enable would be equally as effective as
 * modifying the IPL in this case. However, in some cases, we may want
 * more fine grained control, making modifying the IPL more
 * suitable. We modify the IPL here for consistency.
 */
void
sched_make_runnable(kthread_t *thr)
{

        /*NOT_YET_IMPLEMENTED("PROCS: sched_make_runnable");*/
      /* Cody: We use disable/enable in our case. I think it works. We could also set IPL high and restore it.*/

	/*dbg(DBG_PRINT,"*****************************************Entering sched_make_runnable\n");*/
	KASSERT(thr!=NULL);
	KASSERT(&kt_runq != thr->kt_wchan);
	dbg(DBG_PRINT,"(GRADING1A 4.b)\n");

        intr_disable();
        thr->kt_state=KT_RUN;
        ktqueue_enqueue(&(kt_runq),thr); /*run queue?*/
        intr_enable();
  	/*dbg(DBG_PRINT,"*****************************************Leaving sched_make_runnable\n");*/
}
