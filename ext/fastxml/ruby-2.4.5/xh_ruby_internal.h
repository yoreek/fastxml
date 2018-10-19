#ifndef _XH_RUBY_INTERNAL_H_
#define _XH_RUBY_INTERNAL_H_

#include "xh_config.h"
#include "xh_core.h"

struct RHash {
    struct RBasic basic;
    struct st_table *ntbl;      /* possibly 0 */
    int iter_lev;
    const VALUE ifnone;
};

#define RHASH(obj)   (R_CAST(RHash)(obj))

#ifdef RHASH_ITER_LEV
#undef RHASH_ITER_LEV
#undef RHASH_IFNONE
#undef RHASH_SIZE
#define RHASH_ITER_LEV(h) (RHASH(h)->iter_lev)
#define RHASH_IFNONE(h) (RHASH(h)->ifnone)
#define RHASH_SIZE(h) (RHASH(h)->ntbl ? (st_index_t)RHASH(h)->ntbl->num_entries : 0)
#endif

#endif /* _XH_RUBY_INTERNAL_H_ */
