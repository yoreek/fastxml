#include "xh_config.h"
#include "xh_core.h"

static VALUE
xh_h2x_exec(VALUE arg)
{
    xh_h2x_ctx_t *ctx = (xh_h2x_ctx_t *) arg;

    xh_writer_init(&ctx->writer, ctx->opts.encoding, ctx->opts.output, ctx->opts.buf_size, ctx->opts.indent, ctx->opts.trim);

    if (ctx->opts.xml_decl) {
        xh_xml_write_xml_declaration(&ctx->writer, ctx->opts.version, ctx->opts.encoding);
    }

    switch (ctx->opts.method) {
        case XH_METHOD_NATIVE:
            if (ctx->opts.use_attr) {
                xh_h2x_native_attr(ctx, ctx->opts.root, xh_strlen(ctx->opts.root), ctx->hash, XH_H2X_F_COMPLEX);
            }
            else {
                xh_h2x_native(ctx, ctx->opts.root, xh_strlen(ctx->opts.root), ctx->hash);
            }
            break;
        case XH_METHOD_LX:
            //xh_h2x_lx(ctx, ctx->hash, NULL, 0, XH_H2X_F_NONE);
            break;
        default:
            rb_raise(rb_eArgError, "Invalid method");
    }

    return Qnil;
}

VALUE
xh_h2x(xh_h2x_ctx_t *ctx)
{
    VALUE result;
    int   state;

    result = rb_protect(xh_h2x_exec, (VALUE) ctx, &state);

    if (state) {
        result = xh_writer_flush(&ctx->writer);
        xh_writer_destroy(&ctx->writer);
        rb_exc_raise(rb_errinfo());
    }

    result = xh_writer_flush(&ctx->writer);
    if (result != Qnil && ctx->opts.utf8) {
#ifdef XH_HAVE_ENCODER
        if (ctx->writer.encoder == NULL) {
            XH_FORCE_UTF8(result);
        }
#else
        XH_FORCE_UTF8(result);
#endif
    }

    xh_writer_destroy(&ctx->writer);

    return result;
}
