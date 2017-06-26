#include "xh_config.h"
#include "xh_core.h"

xh_bool_t
xh_init_opts(xh_opts_t *opts)
{
    xh_char_t method[XH_PARAM_LEN];

    XH_PARAM_READ_INIT

    /* native options */
    XH_PARAM_READ_STRING (opts->root,          "@root");
    XH_PARAM_READ_STRING (opts->version,       "@version");
    XH_PARAM_READ_STRING (opts->encoding,      "@encoding");
    XH_PARAM_READ_INT    (opts->indent,        "@indent");
    XH_PARAM_READ_BOOL   (opts->canonical,     "@canonical");
    XH_PARAM_READ_STRING (opts->content,       "@content");
    XH_PARAM_READ_BOOL   (opts->utf8,          "@utf8");
    XH_PARAM_READ_BOOL   (opts->xml_decl,      "@xml_decl");
    XH_PARAM_READ_BOOL   (opts->keep_root,     "@keep_root");
#ifdef XH_HAVE_DOM
    XH_PARAM_READ_BOOL   (opts->doc,           "@doc");
#endif
    XH_PARAM_READ_INT    (opts->max_depth,     "@max_depth");
    XH_PARAM_READ_INT    (opts->buf_size,      "@buf_size");
    XH_PARAM_READ_PATTERN(opts->force_array,   "@force_array");
    XH_PARAM_READ_BOOL   (opts->force_content, "@force_content");
    XH_PARAM_READ_BOOL   (opts->merge_text,    "@merge_text");

    /* XML::Hash::LX options */
    XH_PARAM_READ_STRING (opts->attr,          "@attr");
    opts->attr_len = xh_strlen(opts->attr);
    XH_PARAM_READ_STRING (opts->text,          "@text");
    XH_PARAM_READ_BOOL   (opts->trim,          "@trim");
    XH_PARAM_READ_STRING (opts->cdata,         "@cdata");
    XH_PARAM_READ_STRING (opts->comm,          "@comm");

    /* method */
    XH_PARAM_READ_STRING (method,              "@method");
    XH_PARAM_READ_BOOL   (opts->use_attr,      "@use_attr");
    if (xh_strcmp(method, XH_CHAR_CAST "LX") == 0) {
        opts->method = XH_METHOD_LX;
    }
    else {
        opts->method = XH_METHOD_NATIVE;
    }

    /* output, NULL - to string */
    XH_PARAM_READ_REF    (opts->output,        "@output");

    return TRUE;
}

xh_opts_t *
xh_create_opts(void)
{
    xh_opts_t *opts;

    if ((opts = malloc(sizeof(xh_opts_t))) == NULL) {
        return NULL;
    }
    memset(opts, 0, sizeof(xh_opts_t));

    if (! xh_init_opts(opts)) {
        xh_destroy_opts(opts);
        return NULL;
    }

    return opts;
}

void
xh_destroy_opts(xh_opts_t *opts)
{
    /* nothing */
}

void
xh_copy_opts(xh_opts_t *dst, xh_opts_t *src)
{
    memcpy(dst, src, sizeof(xh_opts_t));
}

static int
xh_parse_arg(VALUE key, VALUE value, VALUE ctx)
{
    xh_opts_t *opts = (xh_opts_t *) ctx;
    xh_char_t *keyptr, *valueptr;
    size_t     keylen, valuelen;

    if (SYMBOL_P(key)) {
        key = rb_sym2str(key);
    }

    keyptr = XH_CHAR_CAST RSTRING_PTR(key);
    keylen = RSTRING_LEN(key);

    switch (keylen) {
#ifdef XH_HAVE_DOM
        case 3:
            if (xh_str_equal3(keyptr, 'd', 'o', 'c')) {
                opts->doc = xh_param_assign_bool(value);
                break;
            }
            goto error;
#endif
        case 4:
            if (xh_str_equal4(keyptr, 'a', 't', 't', 'r')) {
                xh_param_assign_string(opts->attr, value);
                if (opts->attr[0] == '\0') {
                    opts->attr_len = 0;
                }
                else {
                    opts->attr_len = xh_strlen(opts->attr);
                }
                break;
            }
            if (xh_str_equal4(keyptr, 'c', 'o', 'm', 'm')) {
                xh_param_assign_string(opts->comm, value);
                break;
            }
            if (xh_str_equal4(keyptr, 'r', 'o', 'o', 't')) {
                xh_param_assign_string(opts->root, value);
                break;
            }
            if (xh_str_equal4(keyptr, 't', 'r', 'i', 'm')) {
                opts->trim = xh_param_assign_bool(value);
                break;
            }
            if (xh_str_equal4(keyptr, 't', 'e', 'x', 't')) {
                xh_param_assign_string(opts->text, value);
                break;
            }
            if (xh_str_equal4(keyptr, 'u', 't', 'f', '8')) {
                opts->utf8 = xh_param_assign_bool(value);
                break;
            }
            goto error;
        case 5:
            if (xh_str_equal5(keyptr, 'c', 'd', 'a', 't', 'a')) {
                xh_param_assign_string(opts->cdata, value);
                break;
            }
            goto error;
        case 6:
            if (xh_str_equal6(keyptr, 'i', 'n', 'd', 'e', 'n', 't')) {
                xh_param_assign_int(keyptr, &opts->indent, value);
                break;
            }
            if (xh_str_equal6(keyptr, 'm', 'e', 't', 'h', 'o', 'd')) {
                if (value == Qnil) {
                    rb_raise(rb_eArgError, "Parameter '%s' is undefined", StringValueCStr(key));
                }
                valueptr = XH_CHAR_CAST RSTRING_PTR(value);
                valuelen = RSTRING_LEN(value);
                switch  (valuelen) {
                    case 6:
                        if (xh_str_equal6(valueptr, 'N', 'A', 'T', 'I', 'V', 'E')) {
                            opts->method = XH_METHOD_NATIVE;
                            break;
                        }
                        goto error_value;
                    case 2:
                        if (valueptr[0] == 'L' && valueptr[1] == 'X') {
                            opts->method = XH_METHOD_LX;
                            break;
                        }
                        goto error_value;
                    default:
                        goto error_value;
                }
                break;
            }
            if (xh_str_equal6(keyptr, 'o', 'u', 't', 'p', 'u', 't')) {
                if ( RTEST(value) ) {
                    opts->output = value;
                }
                else {
                    opts->output = Qnil;
                }
                break;
            }
            if (xh_str_equal6(keyptr, 'f', 'i', 'l', 't', 'e', 'r')) {
                xh_param_assign_filter(&opts->filter, value);
                break;
            }
            goto error;
        case 7:
            if (xh_str_equal7(keyptr, 'c', 'o', 'n', 't', 'e', 'n', 't')) {
                xh_param_assign_string(opts->content, value);
                break;
            }
            if (xh_str_equal7(keyptr, 'v', 'e', 'r', 's', 'i', 'o', 'n')) {
                xh_param_assign_string(opts->version, value);
                break;
            }
            goto error;
        case 8:
            if (xh_str_equal8(keyptr, 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g')) {
                xh_param_assign_string(opts->encoding, value);
                break;
            }
            if (xh_str_equal8(keyptr, 'u', 's', 'e', '_', 'a', 't', 't', 'r')) {
                opts->use_attr = xh_param_assign_bool(value);
                break;
            }
            if (xh_str_equal8(keyptr, 'x', 'm', 'l', '_', 'd', 'e', 'c', 'l')) {
                opts->xml_decl = xh_param_assign_bool(value);
                break;
            }
            if (xh_str_equal8(keyptr, 'b', 'u', 'f', '_', 's', 'i', 'z', 'e')) {
                xh_param_assign_int(keyptr, &opts->buf_size, value);
                break;
            }
            goto error;
        case 9:
            if (xh_str_equal9(keyptr, 'c', 'a', 'n', 'o', 'n', 'i', 'c', 'a', 'l')) {
                opts->canonical = xh_param_assign_bool(value);
                break;
            }
            if (xh_str_equal9(keyptr, 'm', 'a', 'x', '_', 'd', 'e', 'p', 't', 'h')) {
                xh_param_assign_int(keyptr, &opts->max_depth, value);
                break;
            }
            if (xh_str_equal9(keyptr, 'k', 'e', 'e', 'p', '_', 'r', 'o', 'o', 't')) {
                opts->keep_root = xh_param_assign_bool(value);
                break;
            }
            goto error;
        case 10:
            if (xh_str_equal10(keyptr, 'm', 'e', 'r', 'g', 'e', '_', 't', 'e', 'x', 't')) {
                opts->merge_text = xh_param_assign_bool(value);
                break;
            }
        case 11:
            if (xh_str_equal11(keyptr, 'f', 'o', 'r', 'c', 'e', '_', 'a', 'r', 'r', 'a', 'y')) {
                xh_param_assign_pattern(&opts->force_array, value);
                break;
            }
        case 13:
            if (xh_str_equal13(keyptr, 'f', 'o', 'r', 'c', 'e', '_', 'c', 'o', 'n', 't', 'e', 'n', 't')) {
                opts->force_content = xh_param_assign_bool(value);
                break;
            }
        default:
            goto error;
    }

    return ST_CONTINUE;

error_value:
    rb_raise(rb_eArgError, "Invalid parameter value for '%s': %s", StringValueCStr(key), StringValueCStr(value));

error:
    rb_raise(rb_eArgError, "Invalid parameter '%s'", StringValueCStr(key));
}

void
xh_parse_args(xh_opts_t *opts, xh_int_t *nparam, xh_int_t argc, VALUE *argv)
{
    VALUE hash;

    if (*nparam >= argc)
        return;

    hash = argv[*nparam];
    if (rb_cHash != rb_obj_class(hash))
        rb_raise(rb_eArgError, "Parameter is not a hash");

    (*nparam)++;

    rb_hash_foreach(hash, xh_parse_arg, (VALUE) opts);

    opts->block_given = xh_param_assign_block();
}

void *
xh_get_obj_param(xh_int_t *nparam, xh_int_t argc, VALUE *argv, const char *class)
{
    void *obj = NULL;
/*
    SV   *param;

    if (*nparam >= items)
        rb_raise(rb_eArgError, "Invalid parameters");

    param = ST(*nparam);
    if ( sv_derived_from(param, class) ) {
        if ( sv_isobject(param) ) {
            // reference to object
            IV tmp = SvIV((SV *) SvRV(param));
            obj = INT2PTR(xh_opts_t *, tmp);
        }
        (*nparam)++;
    }
*/
    return obj;
}

VALUE
xh_get_hash_param(xh_int_t *nparam, xh_int_t argc, VALUE *argv)
{
    VALUE param;

    if (*nparam >= argc)
        rb_raise(rb_eArgError, "Invalid parameters");

    param = argv[*nparam];
    if (rb_cHash != rb_obj_class(param))
        rb_raise(rb_eArgError, "Parameter is not a hash");

    (*nparam)++;

    return param;
}

VALUE
xh_get_str_param(xh_int_t *nparam, xh_int_t argc, VALUE *argv)
{
    VALUE param;

    if (*nparam >= argc)
        rb_raise(rb_eArgError, "Invalid parameters");

    param = argv[*nparam];
    if (!RB_TYPE_P(param, RUBY_T_STRING))
        rb_raise(rb_eArgError, "Parameter is not a string");

    (*nparam)++;

    return param;
}

void
xh_merge_opts(xh_opts_t *ctx_opts, xh_opts_t *opts, xh_int_t *nparam, xh_int_t argc, VALUE *argv)
{
    if (opts == NULL) {
        /* read global options */
        xh_init_opts(ctx_opts);
    }
    else {
        /* copy options from object */
        xh_copy_opts(ctx_opts, opts);
    }
    if (*nparam < argc) {
        xh_parse_args(ctx_opts, nparam, argc, argv);
    }
}

