#include "xh_config.h"
#include "xh_core.h"

typedef struct {
    xh_h2x_ctx_t *ctx;
    xh_int_t      flag;
    size_t       *done;
} xh_iter_ctx_t;

static int
xh_h2x_native_attr_hash_iter(VALUE key, VALUE value, VALUE arg)
{
    xh_iter_ctx_t *iter_ctx = (xh_iter_ctx_t *) arg;

    if (SYMBOL_P(key)) {
        key = rb_sym2str(key);
    }
    else {
        key = rb_obj_as_string(key);
    }

    *iter_ctx->done += xh_h2x_native_attr(iter_ctx->ctx, XH_CHAR_CAST RSTRING_PTR(key), RSTRING_LEN(key), value, iter_ctx->flag);

    return ST_CONTINUE;
}

static VALUE
xh_h2x_native_attr_call_next(VALUE obj)
{
    return rb_funcall(obj, xh_id_next, 0);
}

static VALUE
xh_h2x_native_attr_call_stop(VALUE obj)
{
    return Qnil;
}

xh_int_t
xh_h2x_native_attr(xh_h2x_ctx_t *ctx, xh_char_t *key, size_t key_len, VALUE value, xh_int_t flag)
{
    xh_uint_t       type;
    size_t          len, i, nattrs, done;
    xh_sort_hash_t  sorted_hash;
    VALUE           next_value;
    xh_iter_ctx_t   iter_ctx;

    nattrs = 0;

    if (ctx->opts.content[0] != '\0' && xh_strcmp(key, ctx->opts.content) == 0)
        flag = flag | XH_H2X_F_CONTENT;

    value = xh_h2x_resolve_value(ctx, value, &type);

    if (rb_obj_is_kind_of(value, rb_cEnumerator)) {
        if (!(flag & XH_H2X_F_COMPLEX)) goto FINISH;

        while (1) {
            next_value = rb_rescue2(xh_h2x_native_attr_call_next, value, xh_h2x_native_attr_call_stop, value, rb_eStopIteration, (VALUE) 0);
            if (next_value == Qnil) break;
            (void) xh_h2x_native_attr(ctx, key, key_len, next_value, XH_H2X_F_SIMPLE | XH_H2X_F_COMPLEX);
        }

        nattrs++;

        goto FINISH;
    }

    if (type & XH_H2X_T_SCALAR) {
        if (flag & XH_H2X_F_COMPLEX && (flag & XH_H2X_F_SIMPLE || type & XH_H2X_T_RAW)) {
            xh_xml_write_node(&ctx->writer, key, key_len, value, type & XH_H2X_T_RAW);
        }
        else if (flag & XH_H2X_F_COMPLEX && flag & XH_H2X_F_CONTENT) {
            xh_xml_write_content(&ctx->writer, value);
        }
        else if (flag & XH_H2X_F_SIMPLE && !(flag & XH_H2X_F_CONTENT) && !(type & XH_H2X_T_RAW)) {
            xh_xml_write_attribute(&ctx->writer, key, key_len, value);
            nattrs++;
        }
    }
    else if (type & XH_H2X_T_HASH) {
        if (!(flag & XH_H2X_F_COMPLEX)) goto FINISH;

        len = RHASH_SIZE(value);
        if (len == 0) {
            xh_xml_write_empty_node(&ctx->writer, key, key_len);
            goto FINISH;
        }

        xh_xml_write_start_tag(&ctx->writer, key, key_len);

        done = 0;
        iter_ctx.ctx = ctx;
        iter_ctx.done = &done;

        if (len > 1 && ctx->opts.canonical) {
            sorted_hash.elts = alloca(sizeof(xh_sort_hash_elt_t) * len);

            xh_sort_hash(&sorted_hash, value);

            for (i = 0; i < sorted_hash.nelts; i++) {
                done += xh_h2x_native_attr(ctx, sorted_hash.elts[i].key, sorted_hash.elts[i].key_len, sorted_hash.elts[i].value, XH_H2X_F_SIMPLE);
            }

            if (done == len) {
                xh_xml_write_closed_end_tag(&ctx->writer);
            }
            else {
                xh_xml_write_end_tag(&ctx->writer);

                for (i = 0; i < sorted_hash.nelts; i++) {
                    (void) xh_h2x_native_attr(ctx, sorted_hash.elts[i].key, sorted_hash.elts[i].key_len, sorted_hash.elts[i].value, XH_H2X_F_COMPLEX);
                }

                xh_xml_write_end_node(&ctx->writer, key, key_len);
            }
        }
        else {
            iter_ctx.flag = XH_H2X_F_SIMPLE;
            rb_hash_foreach(value, xh_h2x_native_attr_hash_iter, (VALUE) &iter_ctx);

            if (done == len) {
                xh_xml_write_closed_end_tag(&ctx->writer);
            }
            else {
                xh_xml_write_end_tag(&ctx->writer);

                iter_ctx.flag = XH_H2X_F_COMPLEX;
                rb_hash_foreach(value, xh_h2x_native_attr_hash_iter, (VALUE) &iter_ctx);

                xh_xml_write_end_node(&ctx->writer, key, key_len);
            }
        }

        nattrs++;
    }
    else if (type & XH_H2X_T_ARRAY) {
        if (!(flag & XH_H2X_F_COMPLEX)) goto FINISH;

        len = RARRAY_LEN(value);
        for (i = 0; i < len; i++) {
            (void) xh_h2x_native_attr(ctx, key, key_len, RARRAY_AREF(value, i), XH_H2X_F_SIMPLE | XH_H2X_F_COMPLEX);
        }

        nattrs++;
    }
    else {
        if (flag & XH_H2X_F_SIMPLE && flag & XH_H2X_F_COMPLEX) {
            xh_xml_write_empty_node(&ctx->writer, key, key_len);
        }
        else if (flag & XH_H2X_F_SIMPLE && !(flag & XH_H2X_F_CONTENT)) {
            xh_xml_write_attribute(&ctx->writer, key, key_len, Qnil);
            nattrs++;
        }
    }

FINISH:
    ctx->depth--;

    return nattrs;
}
