#include "xh_config.h"
#include "xh_core.h"

static int
xh_h2x_native_hash_iter(VALUE key, VALUE value, VALUE arg)
{
    xh_h2x_ctx_t *ctx = (xh_h2x_ctx_t *) arg;

    if (SYMBOL_P(key)) {
        key = rb_sym2str(key);
    }
    else {
        key = rb_obj_as_string(key);
    }

    xh_h2x_native(ctx, XH_CHAR_CAST RSTRING_PTR(key), RSTRING_LEN(key), value);

    return ST_CONTINUE;
}

static VALUE
xh_h2x_native_call_next(VALUE obj)
{
    return rb_funcall(obj, xh_id_next, 0);
}

static VALUE
xh_h2x_native_call_stop(VALUE obj)
{
    return Qnil;
}

void
xh_h2x_native(xh_h2x_ctx_t *ctx, xh_char_t *key, size_t key_len, VALUE value)
{
    xh_uint_t       type;
    size_t          i, len;
    VALUE           next_value;
    xh_sort_hash_t  sorted_hash;

    value = xh_h2x_resolve_value(ctx, value, &type);

    if (rb_obj_is_kind_of(value, rb_cEnumerator)) {
        while (1) {
            next_value = rb_rescue2(xh_h2x_native_call_next, value, xh_h2x_native_call_stop, value, rb_eStopIteration, (VALUE) 0);
            if (next_value == Qnil) break;
            xh_h2x_native(ctx, key, key_len, next_value);
        }
        goto FINISH;
    }

    if (type & XH_H2X_T_SCALAR) {
        xh_xml_write_node(&ctx->writer, key, key_len, value, type & XH_H2X_T_RAW);
    }
    else if (type & XH_H2X_T_HASH) {
        len = RHASH_SIZE(value);
        if (len == 0) goto ADD_EMPTY_NODE;

        xh_xml_write_start_node(&ctx->writer, key, key_len);

        if (len > 1 && ctx->opts.canonical) {
            sorted_hash.elts = alloca(sizeof(xh_sort_hash_elt_t) * len);

            xh_sort_hash(&sorted_hash, value);

            for (i = 0; i < sorted_hash.nelts; i++) {
                xh_h2x_native(ctx, sorted_hash.elts[i].key, sorted_hash.elts[i].key_len, sorted_hash.elts[i].value);
            }
        }
        else {
            rb_hash_foreach(value, xh_h2x_native_hash_iter, (VALUE) ctx);
        }

        xh_xml_write_end_node(&ctx->writer, key, key_len);
    }
    else if (type & XH_H2X_T_ARRAY) {
        len = RARRAY_LEN(value);
        for (i = 0; i < len; i++) {
            xh_h2x_native(ctx, key, key_len, RARRAY_AREF(value, i));
        }
    }
    else {
ADD_EMPTY_NODE:
        xh_xml_write_empty_node(&ctx->writer, key, key_len);
    }

FINISH:
    ctx->depth--;
}
