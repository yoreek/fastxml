#ifndef _XH_RUBY_HASH_H_
#define _XH_RUBY_HASH_H_

#include "xh_config.h"
#include "xh_core.h"
#if RUBY_API_VERSION_MAJOR == 2 && RUBY_API_VERSION_MINOR == 2
#include "ruby-2.2.7/xh_ruby_st.h"
#elif RUBY_API_VERSION_MAJOR == 2 && RUBY_API_VERSION_MINOR == 3
#include "ruby-2.3.4/xh_ruby_st.h"
#elif RUBY_API_VERSION_MAJOR == 2 && RUBY_API_VERSION_MINOR == 4
#include "ruby-2.4.1/xh_ruby_st.h"
#endif

static VALUE *
hash_store(VALUE hash, const char *key , size_t keylen, VALUE val)
{
    VALUE key_val = rb_utf8_str_new(key, keylen);
    return st_store(RHASH(hash)->ntbl, (st_data_t) key_val, val, TRUE);
}

static VALUE *
hash_fetch(VALUE hash, const char *key , size_t keylen, VALUE val)
{
    VALUE key_val = rb_utf8_str_new(key, keylen);
    VALUE *lval;
    lval = st_store(RHASH(hash)->ntbl, (st_data_t) key_val, val, FALSE);

    return lval;
}

static VALUE
hash_new(void)
{
    VALUE hash = rb_hash_new();
    rb_funcall(hash, xh_id_initialize, 0);
    return hash;
}

static int
hash_first_value_i(VALUE key, VALUE value, VALUE first)
{
    *((VALUE *) first) = value;
    return ST_STOP;
}

static VALUE
hash_first_value(VALUE hash)
{
    VALUE value = Qnil;
    rb_hash_foreach(hash, hash_first_value_i, (VALUE) &value);
    return value;
}

#endif /* _XH_RUBY_HASH_H_ */
