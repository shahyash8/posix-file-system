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

#include "config.h"
#include "globals.h"

#include "errno.h"

#include "util/init.h"
#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"

#include "proc/kthread.h"
#include "proc/proc.h"
#include "proc/sched.h"

#include "mm/slab.h"
#include "mm/page.h"

kthread_t *curthr; /* global */
static slab_allocator_t *kthread_allocator = NULL;

#ifdef __MTP__
/* Stuff for the reaper daemon, which cleans up dead detached threads */
static proc_t *reapd = NULL;
static kthread_t *reapd_thr = NULL;
static ktqueue_t reapd_waitq;
static list_t kthread_reapd_deadlist; /* Threads to be cleaned */

static void *kthread_reapd_run(int arg1, void *arg2);
#endif

void
kthread_init()
{
        kthread_allocator = slab_allocator_create("kthread", sizeof(kthread_t));
        KASSERT(NULL != kthread_allocator);
}

/**
 * Allocates a new kernel stack.
 *
 * @return a newly allocated stack, or NULL if there is not enough
 * memory available
 */
static char *
alloc_stack(void)
{
        /* extra page for "magic" data */
        char *kstack;
        int npages = 1 + (DEFAULT_STACK_SIZE >> PAGE_SHIFT);
        kstack = (char *)page_alloc_n(npages);

        return kstack;
}

/**
 * Frees a stack allocated with alloc_stack.
 *
 * @param stack the stack to free
 */
static void
free_stack(char *stack)
{
        page_free_n(stack, 1 + (DEFAULT_STACK_SIZE >> PAGE_SHIFT));
}

void
kthread_destroy(kthread_t *t)
{
        KASSERT(t && t->kt_kstack);
        free_stack(t->kt_kstack);
        if (list_link_is_linked(&t->kt_plink))
                list_remove(&t->kt_plink);

        slab_obj_free(kthread_allocator, t);
}

/*
 * Allocate a new stack with the alloc_stack function. The size of the
 * stack is DEFAULT_STACK_SIZE.
 *
 * Don't forget to initialize the thread context with the
 * context_setup function. The context should have the same pagetable
 * pointer as the process.
 */
kthread_t *
kthread_create(struct proc *p, kthread_func_t func, long arg1, void *arg2)
{
      	/*  //Cody Modifying
        //to refer the structure of kthread and context, go to ~kernel/include/proc/kthread.h or context.h
        //alloc new thread */
        KASSERT(NULL != p);
        dbg(DBG_PRINT,"(GRADING1A 3.a)\n");

	       kthread_t* new=(kthread_t*)slab_obj_alloc(kthread_allocator);
      	/*  //alloc its stack */
	/*    ini context */

	       new->kt_kstack=alloc_stack();
        context_setup(&new->kt_ctx,func,arg1,arg2,new->kt_kstack,DEFAULT_STACK_SIZE, p->p_pagedir);

        /* this thread's context */

        new->kt_retval=NULL;      /* this thread's return value */
        new->kt_errno=0;       /* error no. of most recent syscall */
        new->kt_proc=p;        /* the thread's process */
        new->kt_cancelled=0;   /* 1 if this thread has been cancelled */
        new->kt_wchan=NULL;       /* The queue that this thread is blocked on */
      /*  NOT_YET_IMPLEMENTED("PROCS: kthread_create");*/
        new->kt_state=KT_NO_STATE;       /* this thread's state,runnable when created */
        list_link_init(&new->kt_qlink);       /* link on ktqueue */
        list_link_init(&new->kt_plink);       /* link on proc thread list */
        list_insert_tail(&p->p_threads,&new->kt_plink); /*insert the plink at the end of p Process
                                                        In fact, we only have one thread as it's single
                                                        thread OS*/
        return new;
}

/*
 * If the thread to be cancelled is the current thread, this is
 * equivalent to calling kthread_exit. Otherwise, the thread is
 * sleeping and we need to set the cancelled and retval fields of the
 * thread.
 *
 * If the thread's sleep is cancellable, cancelling the thread should
 * wake it up from sleep.
 *
 * If the thread's sleep is not cancellable, we do nothing else here.
 */
void
kthread_cancel(kthread_t *kthr, void *retval)
{
        KASSERT(NULL != kthr);
        dbg(DBG_PRINT,"(GRADING1A 3.b)\n");
        dbg(DBG_PRINT,"(GRADING1C)\n");

        if(kthr==curthr){
            dbg(DBG_PRINT,"(GRADING1C)\n");

            kthread_exit(retval);
        }
        else{
          dbg(DBG_PRINT,"(GRADING1C)\n");
            kthr->kt_retval=retval;
            sched_cancel(kthr);
          }

          /*  if(kthr->kt_state==) */
          /*  NOT_YET_IMPLEMENTED("PROCS: kthread_cancel"); */


}

/*
 * You need to set the thread's retval field, set its state to
 * KT_EXITED, and alert the current process that a thread is exiting
 * via proc_thread_exited.
 *
 * It may seem unneccessary to push the work of cleaning up the thread
 * over to the process. However, if you implement MTP, a thread
 * exiting does not necessarily mean that the process needs to be
 * cleaned up.
 */
void
kthread_exit(void *retval)
{
        curthr->kt_retval=retval;
        curthr->kt_state=KT_EXITED;
        KASSERT(!curthr->kt_wchan);
        dbg(DBG_PRINT,"(GRADING1A 3.c)\n");
        KASSERT(!curthr->kt_qlink.l_next && !curthr->kt_qlink.l_prev);
        dbg(DBG_PRINT,"(GRADING1A 3.c)\n");
        KASSERT(curthr->kt_proc == curproc);
        proc_thread_exited(retval);
      /*  NOT_YET_IMPLEMENTED("PROCS: kthread_exit"); */

}

/*
 * The new thread will need its own context and stack. Think carefully
 * about which fields should be copied and which fields should be
 * freshly initialized.
 * You do not need to worry about this until VM.
 */
kthread_t *
kthread_clone(kthread_t *thr)
{
        NOT_YET_IMPLEMENTED("VM: kthread_clone");
        return NULL;
}

/*
 * The following functions will be useful if you choose to implement
 * multiple kernel threads per process. This is strongly discouraged
 * unless your weenix is perfect.
 */
#ifdef __MTP__
int
kthread_detach(kthread_t *kthr)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_detach");
        return 0;
}

int
kthread_join(kthread_t *kthr, void **retval)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_join");
        return 0;
}

/* ------------------------------------------------------------------ */
/* -------------------------- REAPER DAEMON ------------------------- */
/* ------------------------------------------------------------------ */
static __attribute__((unused)) void
kthread_reapd_init()
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_init");
}
init_func(kthread_reapd_init);
init_depends(sched_init);

void
kthread_reapd_shutdown()
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_shutdown");
}

static void *
kthread_reapd_run(int arg1, void *arg2)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_run");
        return (void *) 0;
}
#endif
