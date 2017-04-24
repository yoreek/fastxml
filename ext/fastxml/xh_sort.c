#include "xh_config.h"
#include "xh_core.h"

static int
xh_sort_hash_cmp(const void *p1, const void *p2)
{
    return xh_strcmp(((xh_sort_hash_elt_t *) p1)->key, ((xh_sort_hash_elt_t *) p2)->key);
}

static int
xh_sort_hash_iter(VALUE key, VALUE value, VALUE arg)
{
    xh_sort_hash_t *sorted_hash = (xh_sort_hash_t *) arg;

    if (SYMBOL_P(key)) {
        key = rb_sym2str(key);
    }
    else {
        key = rb_obj_as_string(key);
    }

    key = StringValue(key);

    sorted_hash->elts[sorted_hash->nelts].value   = value;
    sorted_hash->elts[sorted_hash->nelts].key     = XH_CHAR_CAST StringValueCStr(key);
    sorted_hash->elts[sorted_hash->nelts].key_len = RSTRING_LEN(key);
    sorted_hash->nelts++;

    return ST_CONTINUE;
}

void
xh_sort_hash(xh_sort_hash_t *sorted_hash, VALUE hash)
{
    sorted_hash->nelts = 0;

    rb_hash_foreach(hash, xh_sort_hash_iter, (VALUE) sorted_hash);

    qsort(sorted_hash->elts, sorted_hash->nelts, sizeof(xh_sort_hash_elt_t), xh_sort_hash_cmp);
}
