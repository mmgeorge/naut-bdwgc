/*
 * Copyright (c) 1994 by Xerox Corporation.  All rights reserved.
 * Copyright (c) 1996 by Silicon Graphics.  All rights reserved.
 * Copyright (c) 1998 by Fergus Henderson.  All rights reserved.
 * Copyright (c) 2000-2009 by Hewlett-Packard Development Company.
 * All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */


#include <nautilus/scheduler.h>
#include "private/pthread_support.h"


#ifdef NAUT_THREADS


GC_INNER void GC_stop_world(void)
{
  BDWGC_DEBUG("Stopping the world from %p\n", get_cur_thread());
  //cli();
}


/* Caller holds allocation lock, and has held it continuously since     */
/* the world stopped.                                                   */
GC_INNER void GC_start_world(void)
{
  BDWGC_DEBUG("Starting the world from %p\n", get_cur_thread());
  //  sti();
}


static void push_thread_stack(nk_thread_t *t)
{
  ptr_t lo, hi;
  struct GC_traced_stack_sect_s *traced_stack_sect;
  word total_size = 0;
  
  //if (!GC_thr_initialized) GC_thr_init();
  
  traced_stack_sect = t -> gc_state -> traced_stack_sect;

  lo = t -> stack;
  hi = nk_thread_getstackaddr_np(t);
  
  if (traced_stack_sect != NULL
      && traced_stack_sect->saved_stack_ptr == lo) {
    /* If the thread has never been stopped since the recent  */
    /* GC_call_with_gc_active invocation then skip the top    */
    /* "stack section" as stack_ptr already points to.        */
    //    traced_stack_sect = traced_stack_sect->prev;
  }
  
  BDWGC_DEBUG("Pushing stack for thread (%p, tid=%u), range = [%p,%p)\n", t, t->tid, lo, hi);
      
  if (0 == lo) panic("GC_push_all_stacks: sp not set!");
  //GC_push_all_stack_sections(lo, hi, traced_stack_sect);
  GC_push_all_stack_sections(lo, hi, NULL);
  total_size += hi - lo; /* lo <= hi */
      
}

/* We hold allocation lock.  Should do exactly the right thing if the   */
/* world is stopped.  Should not fail if it isn't.                      */
GC_INNER void GC_push_all_stacks(void)
{
  BDWGC_DEBUG("Pushing stacks from thread %p\n", get_cur_thread());
  nk_sched_map_threads(0, push_thread_stack);
}


GC_INNER void GC_stop_init(void)
{
  BDWGC_DEBUG("GC_stop_init ... does nothing..\n");
}

#endif // NAUT_THREADS



