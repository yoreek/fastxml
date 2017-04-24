#ifndef _XH_SORT_H_
#define _XH_SORT_H_

#include "xh_config.h"
#include "xh_core.h"

typedef struct {
    xh_char_t        *key;
    size_t            key_len;
    VALUE             value;
} xh_sort_hash_elt_t;

typedef struct {
    xh_sort_hash_elt_t *elts;
    size_t              nelts;
} xh_sort_hash_t;

void xh_sort_hash(xh_sort_hash_t *sorted_hash, VALUE hash);

#endif /* _XH_SORT_H_ */
