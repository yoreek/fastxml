#include "fastxml.h"

#include "xh_config.h"
#include "xh_core.h"

void Init_fastxml();

VALUE xh_module;
VALUE xh_parse_error_class;
ID    xh_id_next;
ID    xh_id_initialize;

typedef struct {
    int           argc;
    VALUE        *argv;
    xh_h2x_ctx_t *ctx;
} xh_h2x_arg_t;

typedef struct {
    int           argc;
    VALUE        *argv;
    xh_x2h_ctx_t *ctx;
} xh_x2h_arg_t;

static VALUE
hash2xml_exec(VALUE a) {
    xh_h2x_arg_t *arg = (xh_h2x_arg_t *) a;

    xh_h2x_init_ctx(arg->ctx, arg->argc, arg->argv);

    return xh_h2x(arg->ctx);
}

static VALUE
hash2xml(int argc, VALUE *argv, VALUE self) {
    xh_h2x_ctx_t  ctx;
    VALUE         result;
    int           state;
    xh_h2x_arg_t  arg;

    arg.argc = argc;
    arg.argv = argv;
    arg.ctx  = &ctx;

    result = rb_protect(hash2xml_exec, (VALUE) &arg, &state);

    if (state) {
        xh_h2x_destroy_ctx(&ctx);
        rb_exc_raise(rb_errinfo());
    }

    if (ctx.opts.output != Qnil) result = Qnil;

    xh_h2x_destroy_ctx(&ctx);

    return result;
}

static VALUE
xml2hash_exec(VALUE a) {
    xh_x2h_arg_t *arg = (xh_x2h_arg_t *) a;

    xh_x2h_init_ctx(arg->ctx, arg->argc, arg->argv);

    return xh_x2h(arg->ctx);
}

static VALUE
xml2hash(int argc, VALUE *argv, VALUE self) {
    xh_x2h_ctx_t  ctx;
    VALUE         result;
    int           state;
    xh_x2h_arg_t  arg;

    arg.argc = argc;
    arg.argv = argv;
    arg.ctx  = &ctx;

    result = rb_protect(xml2hash_exec, (VALUE) &arg, &state);

    if (state) {
        xh_x2h_destroy_ctx(&ctx);
        rb_exc_raise(rb_errinfo());
    }

    xh_x2h_destroy_ctx(&ctx);

    return result;
}

void Init_fastxml(void) {
    xh_module = rb_define_module("FastXML");
    xh_parse_error_class = rb_const_get_at(xh_module, rb_intern("ParseError"));

    rb_define_module_function(xh_module, "hash2xml", hash2xml, -1);
    rb_define_module_function(xh_module, "xml2hash", xml2hash, -1);

    xh_id_next = rb_intern("next");
    xh_id_initialize = rb_intern("initialize");
}
