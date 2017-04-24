#ifndef _XH_H2X_H_
#define _XH_H2X_H_

#include "xh_config.h"
#include "xh_core.h"

#define XH_H2X_F_NONE                   0
#define XH_H2X_F_SIMPLE                 1
#define XH_H2X_F_COMPLEX                2
#define XH_H2X_F_CONTENT                4
#define XH_H2X_F_ATTR_ONLY              8

#define XH_H2X_T_SCALAR                 1
#define XH_H2X_T_HASH                   2
#define XH_H2X_T_ARRAY                  4
//#define XH_H2X_T_BLESSED                8
#define XH_H2X_T_RAW                    16
#define XH_H2X_T_NOT_NULL               (XH_H2X_T_SCALAR | XH_H2X_T_ARRAY | XH_H2X_T_HASH)

#define XH_H2X_STASH_SIZE               16

typedef struct {
    xh_opts_t    opts;
    xh_int_t     depth;
    xh_writer_t  writer;
    xh_stack_t   stash;
    VALUE        hash;
} xh_h2x_ctx_t;

XH_INLINE VALUE
xh_h2x_resolve_value(xh_h2x_ctx_t *ctx, VALUE value, xh_uint_t *type)
{
    *type = 0;

    while (RB_TYPE_P(value, RUBY_T_DATA) || RB_TYPE_P(value, RUBY_T_OBJECT)) {
        if (rb_obj_is_kind_of(value, rb_cProc) || rb_obj_is_kind_of(value, rb_cMethod)) {
            value = rb_funcall(value, rb_intern("call"), 0);
        }
        else {
            break;
        }

        if (++ctx->depth > ctx->opts.max_depth)
            rb_raise(xh_parse_error_class, "Maximum recursion depth exceeded");
    }

    if (rb_cHash == rb_obj_class(value)) {
        *type |= XH_H2X_T_HASH;
    }
    else if (rb_cArray == rb_obj_class(value)) {
        *type |= XH_H2X_T_ARRAY;
    }
    else if (!RTEST(value)) {
        *type = 0;
    }
    else {
        *type |= XH_H2X_T_SCALAR;
    }

/*
    if (SvOBJECT(value))
        *type |= XH_H2X_T_BLESSED;
*/
    return value;
}

VALUE xh_h2x(xh_h2x_ctx_t *ctx);
void xh_h2x_native(xh_h2x_ctx_t *ctx, xh_char_t *key, size_t key_len, VALUE value);
xh_int_t xh_h2x_native_attr(xh_h2x_ctx_t *ctx, xh_char_t *key, size_t key_len, VALUE value, xh_int_t flag);
/*
void xh_h2x_lx(xh_h2x_ctx_t *ctx, SV *value, xh_char_t *key, I32 key_len, xh_int_t flag);
*/

XH_INLINE void
xh_h2x_destroy_ctx(xh_h2x_ctx_t *ctx)
{
    xh_destroy_opts(&ctx->opts);
}

XH_INLINE void
xh_h2x_init_ctx(xh_h2x_ctx_t *ctx, xh_int_t argc, VALUE *argv)
{
    xh_opts_t *opts = NULL;
    xh_int_t   nparam = 0;

    memset(ctx, 0, sizeof(xh_h2x_ctx_t));

    //opts = (xh_opts_t *) xh_get_obj_param(&nparam, ax, items, "XML::Hash::XS");
    ctx->hash = xh_get_hash_param(&nparam, argc, argv);
    xh_merge_opts(&ctx->opts, opts, &nparam, argc, argv);
}

#endif /* _XH_H2X_H_ */
