

#include <nautilus/thread.h>
#include <nautilus/bdwgc.h>
#include "../../src/bdwgc/include/gc.h"


typedef unsigned long word;

extern NK_LOCK_T GC_allocate_ml;
#define LOCK() NK_LOCK(&GC_allocate_ml)
#define UNLOCK() NK_UNLOCK(&GC_allocate_ml)
#define DCL_LOCK_STATE


//#define MALLOC(x) ({ void *p  = malloc((x)+2*PAD); if (!p) { panic("Failed to Allocate %d bytes\n",x); } memset(p,0,(x)+2*PAD); p+PAD; })

extern nk_tls_key_t GC_thread_key;
extern int  keys_initialized;


void GC_init_thread_local(GC_tlfs p); // included from thread_local_alloc.h

/* Called from GC_inner_start_routine().  Defined in this file to       */
/* minimize the number of include files in pthread_start.c (because     */
/* sem_t and sem_post() are not used that file directly).               */
nk_thread_t *
GC_start_rtn_prepare_thread(void *(**pstart)(void *),
                            void **pstart_arg,
                            struct GC_stack_base *sb, void *arg)
{
  struct start_info * si = arg;
  nk_thread_t * me = get_cur_thread();
  DCL_LOCK_STATE;

  BDWGC_DEBUG("Starting thread %p, sp = %p\n", me, &arg);

  LOCK();
  //me = GC_register_my_thread_inner(sb, me);
  me -> gc_state -> traced_stack_sect = NULL;
  me -> gc_state -> flags = si -> flags;

  GC_init_thread_local(&(me-> gc_state -> tlfs));

  UNLOCK();
  
  *pstart = si -> start_routine;

  //BDWGC_DEBUG("start_routine = %p\n", (void *)(signed_word)(*pstart));

  *pstart_arg = si -> arg;
  //  sem_post(&(si -> registered));      /* Last action on si.   */
  /* OK to deallocate.    */
  return me;
}


bdwgc_thread_state * bdwgc_thread_state_init()
{
  bdwgc_thread_state * s = (bdwgc_thread_state *)malloc(sizeof(bdwgc_thread_state));

  s -> traced_stack_sect = NULL;
  s -> thread_blocked = false;
  /* if (0 != nk_tls_key_create(&GC_thread_key, 0)) { */
  /*   panic("Failed to create key for local allocator"); */
  /* } */

  /* if (0 != nk_tls_set(GC_thread_key, tlfs)) { */
  /*   panic("Failed to set thread specific allocation pointers"); */
  /* } */

  int i;
  for (i = 1; i < GC_TINY_FREELISTS; ++i) {
    s -> tlfs.ptrfree_freelists[i] = (void *)(unsigned long)1;
    s -> tlfs.normal_freelists[i]  = (void *)(unsigned long)1;
  }
  /* Set up the size 0 free lists.    */
  /* We now handle most of them like regular free lists, to ensure    */
  /* That explicit deallocation works.  */
  s -> tlfs.ptrfree_freelists[0] = (void *)(unsigned long)1;
  s -> tlfs.normal_freelists[0]  = (void *)(unsigned long)1;
  
  return s;
}

