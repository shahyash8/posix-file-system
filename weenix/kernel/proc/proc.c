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

#include "kernel.h"
#include "config.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "proc/kthread.h"
#include "proc/proc.h"
#include "proc/sched.h"
#include "proc/proc.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/mm.h"
#include "mm/mman.h"

#include "vm/vmmap.h"

#include "fs/vfs.h"
#include "fs/vfs_syscall.h"
#include "fs/vnode.h"
#include "fs/file.h"

proc_t *curproc = NULL; /* global */
static slab_allocator_t *proc_allocator = NULL;

static list_t _proc_list;
static proc_t *proc_initproc = NULL; /* Pointer to the init process (PID 1) */

void
proc_init()
{
        list_init(&_proc_list);
        proc_allocator = slab_allocator_create("proc", sizeof(proc_t));
        KASSERT(proc_allocator != NULL);
}

proc_t *
proc_lookup(int pid)
{
        proc_t *p;
        list_iterate_begin(&_proc_list, p, proc_t, p_list_link) {
                if (p->p_pid == pid) {
                        return p;
                }
        } list_iterate_end();
        return NULL;
}

list_t *
proc_list()
{
        return &_proc_list;
}

static pid_t next_pid = 0;

/**
 * Returns the next available PID.
 *
 * Note: Where n is the number of running processes, this algorithm is
 * worst case O(n^2). As long as PIDs never wrap around it is O(n).
 *
 * @return the next available PID
 */
static int
_proc_getid()
{
        proc_t *p;
        pid_t pid = next_pid;
        while (1) {
failed:
                list_iterate_begin(&_proc_list, p, proc_t, p_list_link) {
                        if (p->p_pid == pid) {
                                if ((pid = (pid + 1) % PROC_MAX_COUNT) == next_pid) {
                                        return -1;
                                } else {
                                        goto failed;
                                }
                        }
                } list_iterate_end();
                next_pid = (pid + 1) % PROC_MAX_COUNT;
                return pid;
        }
}

/*
 * The new process, although it isn't really running since it has no
 * threads, should be in the PROC_RUNNING state.
 *
 * Don't forget to set proc_initproc when you create the init
 * process. You will need to be able to reference the init process
 * when reparenting processes to the init process.
 */
proc_t *
proc_create(char *name)
{
      /*  //Garry Locked to implement proc_create*/
      /*fixed some errors*/
      /* NOT_YET_IMPLEMENTED("PROCS: proc_create");*/
        /*
           Setting attributes of the process
        */
        proc_t *new_proc = (proc_t*)slab_obj_alloc(proc_allocator);
        pid_t pid = _proc_getid();

      /*if(pid != -1)
      {
      */
        KASSERT(PID_IDLE != pid || list_empty(&_proc_list)); /* pid can only be PID_IDLE if this is the first process */

        dbg(DBG_PRINT,"(GRADING1A 2.a)\n");

        KASSERT(PID_INIT != pid || PID_IDLE == curproc->p_pid); /* pid can only be PID_INIT when creating from idle process */

        dbg(DBG_PRINT,"(GRADING1A 2.a)\n");

        new_proc->p_pid = pid;

        strncpy(new_proc->p_comm,name,sizeof(new_proc->p_comm)-1);/* process name*/

      /*   curproc=new_proc; */

        /*Init proc global variable is set*/
        if(new_proc->p_pid == PID_INIT)
        {
        dbg(DBG_PRINT,"(GRADING1C)\n");
        proc_initproc=new_proc;
        }

        /* Initializing the parent proc with the curr_proc*/
        if(new_proc->p_pid==PID_IDLE)
        {
          dbg(DBG_PRINT,"(GRADING1C)\n");
          new_proc->p_pproc = NULL;
        }
        else
        {
         dbg(DBG_PRINT,"(GRADING1C)\n");
         new_proc->p_pproc = curproc;
        }

        /*exit status not sure*/
        new_proc->p_status = 0;

        /*Process state is RUNNING*/
        new_proc->p_state = PROC_RUNNING;


        /*
          Initialize wait queue implementation
        */
        sched_queue_init(&(new_proc->p_wait));

        /* initialise list of threads */
        list_init(&(new_proc->p_threads));

        /* initialize list of child processes */
        list_init(&(new_proc->p_children));



        /*initialize the link to connect to parent child processes*/
        list_link_init(&(new_proc->p_list_link));

        /*Insert new proc in list of processes */
        list_insert_tail(&_proc_list,&(new_proc->p_list_link));



        if (new_proc->p_pproc != NULL)
        {
        dbg(DBG_PRINT,"(GRADING1C)\n");
        /*initialize the link to connect to parent child processes*/
        list_link_init(&(new_proc->p_child_link));

        /* Insert new process in the parent thread child processes */

        list_insert_tail(&(new_proc->p_pproc->p_children),&(new_proc->p_child_link));
       }


        /* Initialize the page table directory for the process*/
        new_proc->p_pagedir = pt_create_pagedir();
       return new_proc;

	/* VFS related initialization */
	int i = 0;
	for (i = 0; i < NFILES; i++)
		new_proc->p_files[i] = NULL; 
	
	/*new_proc->p_cwd = NULL;*/ 

	/*set it to vfs_root_n*/		
	new_proc->p_cwd = vfs_root_vn;
	
	/*//new_proc->p_files = NULL;
	//new_proc->p_cwd = (vnode_t*)slab_obj_alloc(vnode_allocator);	

  /*   }
     dbg(DBG_PRINT,"(GRADING1E 2.5 proc_create(): No new pids could be added to the system)\n");
    return NULL;
*/
}

/**
 * Cleans up as much as the process as can be done from within the
 * process. This involves:
 *    - Closing all open files (VFS)
 *    - Cleaning up VM mappings (VM)
 *    - Waking up its parent if it is waiting
 *    - Reparenting any children to the init process
 *    - Setting its status and state appropriately
 *
 * The parent will finish destroying the process within do_waitpid (make
 * sure you understand why it cannot be done here). Until the parent
 * finishes destroying it, the process is informally called a 'zombie'
 * process.
 *
 * This is also where any children of the current process should be
 * reparented to the init process (unless, of course, the current
 * process is the init process. However, the init process should not
 * have any children at the time it exits).
 *
 * Note: You do _NOT_ have to special case the idle process. It should
 * never exit this way.
 *
 * @param status the status to exit the process with
 */
void
proc_cleanup(int status)
{
        /*NOT_YET_IMPLEMENTED("PROCS: proc_cleanup");
        */
        proc_t *p;
        KASSERT(NULL != proc_initproc); /* should have an "init" process */
        dbg(DBG_PRINT,"(GRADING1A 2.b)\n");

        KASSERT(1 <= curproc->p_pid); /* this process should not be idle process */
        dbg(DBG_PRINT,"(GRADING1A 2.b)\n");

        KASSERT(NULL != curproc->p_pproc); /* this process should have parent process */
        dbg(DBG_PRINT,"(GRADING1A 2.b)\n");


        /* Closing all open files*/

        /* Cleaning up VM mappings*/

        /*
           Waking up parent if its waiting
           in its own p_wait queue
        */
        if(!sched_queue_empty(&(curproc->p_pproc->p_wait)))
        {
          dbg(DBG_PRINT,"(GRADING1C)\n");
          sched_wakeup_on(&(curproc->p_pproc->p_wait));
        }

        /*
            Reparenting the child processes of the current process
        */
      /*  if(curproc != proc_initproc)
        {
        */

                list_iterate_begin(&curproc->p_children, p, proc_t, p_child_link){

                       list_remove(&(p->p_child_link));
                       list_link_init(&(p->p_child_link));
                       list_insert_tail(&(proc_initproc->p_children),&(p->p_child_link));
                       p->p_pproc = proc_initproc;
                } list_iterate_end();
        /*}
        */

       curproc->p_status = status;
       curproc->p_state = PROC_DEAD;
       KASSERT(NULL != curproc->p_pproc); /* this process should have parent process */
       dbg(DBG_PRINT,"(GRADING1A 2.b)\n");

}

/*
 * This has nothing to do with signals and kill(1).
 *
 * Calling this on the current process is equivalent to calling
 * do_exit().
 *
 * In Weenix, this is only called from proc_kill_all.
 */
void
proc_kill(proc_t *p, int status)
{
      /*  NOT_YET_IMPLEMENTED("PROCS: proc_kill"); */
      /*As we are implementing Single thread mode, dont conder MTP here*/
      kthread_t *t;
      if (curproc == p)
      {
           dbg(DBG_PRINT,"(GRADING1C)\n");
           do_exit(status);
      }
      else
      {
          dbg(DBG_PRINT,"(GRADING1C)\n");
          list_iterate_begin(&(p->p_threads),t,kthread_t,kt_plink)
         {
              if(curthr != t)
              {
                dbg(DBG_PRINT,"(GRADING1C)\n");
                kthread_cancel(t,&status);
              }
         }list_iterate_end();
      }

}

/*
 * Remember, proc_kill on the current process will _NOT_ return.
 * Don't kill direct children of the idle process.
 *
 * In Weenix, this is only called by sys_halt.
 */
void
proc_kill_all()
{
        proc_t *p;
        /*NOT_YET_IMPLEMENTED("PROCS: proc_kill_all");*/

          list_iterate_begin(proc_list(), p, proc_t, p_list_link){

                  if(p->p_pid != PID_IDLE && p->p_pproc->p_pid != PID_IDLE && p != curproc && p->p_pid!=PID_INIT){

                      dbg(DBG_PRINT,"(GRADING1C)\n");
                       proc_kill(p,p->p_status);

                  }

          }
          list_iterate_end();
       if(p->p_pid != PID_IDLE && p->p_pproc->p_pid != PID_IDLE)
       {
           dbg(DBG_PRINT,"(GRADING1C)\n");
           proc_kill(curproc,curproc->p_status);
        }


}

/*
 * This function is only called from kthread_exit.
 *
 * Unless you are implementing MTP, this just means that the process
 * needs to be cleaned up and a new thread needs to be scheduled to
 * run. If you are implementing MTP, a single thread exiting does not
 * necessarily mean that the process should be exited.
 */
void
proc_thread_exited(void *retval)
{
        /*proc_cleanup()*/
      /*  NOT_YET_IMPLEMENTED("PROCS: proc_thread_exited");*/
      dbg(DBG_PRINT,"(GRADING1C)\n");
      proc_cleanup(curthr->kt_cancelled);
      sched_switch();

}

/* If pid is -1 dispose of one of the exited children of the current
 * process and return its exit status in the status argument, or if
 * all children of this process are still running, then this function
 * blocks on its own p_wait queue until one exits.
 *
 * If pid is greater than 0 and the given pid is a child of the
 * current process then wait for the given pid to exit and dispose
 * of it.
 *
 * If the current process has no children, or the given pid is not
 * a child of the current process return -ECHILD.
 *
 * Pids other than -1 and positive numbers are not supported.
 * Options other than 0 are not supported.
 */
pid_t
do_waitpid(pid_t pid, int options, int *status)
{
      /*  NOT_YET_IMPLEMENTED("PROCS: do_waitpid");*/

    proc_t *p = NULL;
    proc_t *child = NULL;

    if(list_empty(&curproc->p_children)){

      dbg(DBG_PRINT,"(GRADING1C)\n");
      return -ECHILD;
    }

    /*if((pid == 0) || (pid <-1)){

        dbg(DBG_PRINT,"(GRADING1E 2.14 do_waitpid(): pid %d is not supported)\n",pid);
        return pid;
      }
    if(options!=0){
        dbg(DBG_PRINT,"(GRADING1E 2.15 do_waitpid(): options %d is not supported)\n",options);
        return pid;
      }
      */
      /*if pid is -1, if one of its child exited, return exit status; otherwise block in p_wait and wait for one child process exit*/

      if(pid==-1){
         dbg(DBG_PRINT,"(GRADING1C)\n");
               while(1){
                        list_iterate_begin(&curproc->p_children, p, proc_t, p_child_link) {
                        if(p->p_state==PROC_DEAD){

                                  KASSERT(-1 == pid || p->p_pid == pid); /*should be able to find a valid process ID for the process */
                                  dbg(DBG_PRINT,"(GRADING1A 2.c)\n");

                                  KASSERT(NULL != p); /* the dead child process should not be NULL */
                                  dbg(DBG_PRINT,"(GRADING1A 2.c)\n");
                                  pid_t childpid;
                                  childpid = p->p_pid;
                                  if(status != NULL)
                                  {
                                    dbg(DBG_PRINT,"(GRADING1C)\n");
                                    *status = p->p_status;
                                  }

                                   kthread_t *thr;
                         /*thr = list_head(&(p->p_threads),kthread_t,kt_plink);*/
                         /* thr points to a thread to be destroyed */

              	                  list_iterate_begin(&(p->p_threads),thr,kthread_t,kt_plink) {
                                         KASSERT(KT_EXITED == thr->kt_state);
                                         dbg(DBG_PRINT,"(GRADING1A 2.c)\n");
                                         kthread_destroy(thr);
              	                  }list_iterate_end();

                                  KASSERT(NULL != p->p_pagedir); /* this process should have a valid pagedir */
                                  dbg(DBG_PRINT,"(GRADING1A 2.c)\n");
                                  pt_destroy_pagedir(p->p_pagedir);
                                  list_remove(&(p->p_list_link));
                                  list_remove(&(p->p_child_link));
                                  slab_obj_free(proc_allocator,(void *)p);
                                  return childpid;

                              }
                            }list_iterate_end();
                 /*wait until one of its child proc exit*/
                          dbg(DBG_PRINT,"(GRADING1C)\n");
                          sched_sleep_on(&curproc->p_wait);
                        }

      }
      if (pid > 0){ /*pid is positive*/


        list_iterate_begin(&curproc->p_children, p, proc_t, p_child_link) {
               if(p->p_pid == pid)
               {
                 child = p;
                 dbg(DBG_PRINT,"(GRADING1C)\n");
               }
        }list_iterate_end();

        if( child == NULL )
        {
          dbg(DBG_PRINT,"(GRADING1C)\n");
          return -ECHILD;
        }

        p = child;
        KASSERT(-1 == pid || p->p_pid == pid); /* should be able to find a valid process ID for the process */
        dbg(DBG_PRINT,"(GRADING1A 2.c)\n");
        /*if (p->p_pproc != curproc)
        {
          dbg(DBG_PRINT,"*****************************************Leaving do_waitpid\n");
           return -ECHILD;
         }
         */

        while(1){

          if (p->p_state == PROC_DEAD)
          {
               pid_t childpid;
               KASSERT(NULL != p); /* the dead child process should not be NULL */
               dbg(DBG_PRINT,"(GRADING1A 2.c)\n");
               childpid = p->p_pid;
               *status = p->p_status;

               kthread_t *thr;
               /*thr = list_head(&(p->p_threads),kthread_t,kt_plink);*/
               /* thr points to a thread to be destroyed */

	             list_iterate_begin(&(p->p_threads),thr,kthread_t,kt_plink) {
                     KASSERT(KT_EXITED == thr->kt_state);
                     dbg(DBG_PRINT,"(GRADING1A 2.c)\n");
                     kthread_destroy(thr);

	             }list_iterate_end();



               KASSERT(NULL != p->p_pagedir); /* this process should have a valid pagedir */
               dbg(DBG_PRINT,"(GRADING1A 2.c)\n");
               pt_destroy_pagedir(p->p_pagedir);

               list_remove(&(p->p_list_link));
               list_remove(&(p->p_child_link));
               slab_obj_free(proc_allocator,(void *)p);
	             return childpid;
          }
          else
          {
               dbg(DBG_PRINT,"(GRADING1C)\n");
               sched_sleep_on(&curproc->p_wait);
          }

        }

      }
   return NULL;
}

/*
 * Cancel all threads, join with them, and exit from the current
 * thread.
 *
 * @param status the exit status of the process
 */
void
do_exit(int status)
{
   /*  NOT_YET_IMPLEMENTED("PROCS: do_exit");*/
   dbg(DBG_PRINT,"(GRADING1C)\n");
   curproc->p_status = status;
   kthread_cancel(curthr, &status);
}

size_t
proc_info(const void *arg, char *buf, size_t osize)
{
        const proc_t *p = (proc_t *) arg;
        size_t size = osize;
        proc_t *child;

        KASSERT(NULL != p);
        KASSERT(NULL != buf);

        iprintf(&buf, &size, "pid:          %i\n", p->p_pid);
        iprintf(&buf, &size, "name:         %s\n", p->p_comm);
        if (NULL != p->p_pproc) {
                iprintf(&buf, &size, "parent:       %i (%s)\n",
                        p->p_pproc->p_pid, p->p_pproc->p_comm);
        } else {
                iprintf(&buf, &size, "parent:       -\n");
        }

#ifdef __MTP__
        int count = 0;
        kthread_t *kthr;
        list_iterate_begin(&p->p_threads, kthr, kthread_t, kt_plink) {
                ++count;
        } list_iterate_end();
        iprintf(&buf, &size, "thread count: %i\n", count);
#endif

        if (list_empty(&p->p_children)) {
                iprintf(&buf, &size, "children:     -\n");
        } else {
                iprintf(&buf, &size, "children:\n");
        }
        list_iterate_begin(&p->p_children, child, proc_t, p_child_link) {
                iprintf(&buf, &size, "     %i (%s)\n", child->p_pid, child->p_comm);
        } list_iterate_end();

        iprintf(&buf, &size, "status:       %i\n", p->p_status);
        iprintf(&buf, &size, "state:        %i\n", p->p_state);

#ifdef __VFS__
#ifdef __GETCWD__
        if (NULL != p->p_cwd) {
                char cwd[256];
                lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
                iprintf(&buf, &size, "cwd:          %-s\n", cwd);
        } else {
                iprintf(&buf, &size, "cwd:          -\n");
        }
#endif /* __GETCWD__ */
#endif

#ifdef __VM__
        iprintf(&buf, &size, "start brk:    0x%p\n", p->p_start_brk);
        iprintf(&buf, &size, "brk:          0x%p\n", p->p_brk);
#endif

        return size;
}

size_t
proc_list_info(const void *arg, char *buf, size_t osize)
{
        size_t size = osize;
        proc_t *p;

        KASSERT(NULL == arg);
        KASSERT(NULL != buf);

#if defined(__VFS__) && defined(__GETCWD__)
        iprintf(&buf, &size, "%5s %-13s %-18s %-s\n", "PID", "NAME", "PARENT", "CWD");
#else
        iprintf(&buf, &size, "%5s %-13s %-s\n", "PID", "NAME", "PARENT");
#endif

        list_iterate_begin(&_proc_list, p, proc_t, p_list_link) {
                char parent[64];
                if (NULL != p->p_pproc) {
                        snprintf(parent, sizeof(parent),
                                 "%3i (%s)", p->p_pproc->p_pid, p->p_pproc->p_comm);
                } else {
                        snprintf(parent, sizeof(parent), "  -");
                }

#if defined(__VFS__) && defined(__GETCWD__)
                if (NULL != p->p_cwd) {
                        char cwd[256];
                        lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
                        iprintf(&buf, &size, " %3i  %-13s %-18s %-s\n",
                                p->p_pid, p->p_comm, parent, cwd);
                } else {
                        iprintf(&buf, &size, " %3i  %-13s %-18s -\n",
                                p->p_pid, p->p_comm, parent);
                }
#else
                iprintf(&buf, &size, " %3i  %-13s %-s\n",
                        p->p_pid, p->p_comm, parent);
#endif
        } list_iterate_end();
        return size;
}
