/*
 * Adds a mutex implementation based on c++ mutex to harfbuzz.
 */
#include "include/private/SkMutex.h"

using hb_mutex_impl_t = SkMutex;
#define hb_mutex_impl_init(M)   HB_STMT_START { new (M) hb_mutex_impl_t;  } HB_STMT_END
#define hb_mutex_impl_lock(M)   (M)->acquire ()
#define hb_mutex_impl_unlock(M) (M)->release ()
#define hb_mutex_impl_finish(M) HB_STMT_START { (M)->~hb_mutex_impl_t(); } HB_STMT_END
