#ifndef _XH_H_
#define _XH_H_

#include "xh_config.h"
#include "xh_core.h"

#define XH_INTERNAL_ENCODING "utf-8"

typedef enum {
    XH_METHOD_NATIVE = 0,
    XH_METHOD_LX
} xh_method_t;

typedef struct {
    xh_method_t            method;
    xh_bool_t              use_attr;

    /* native options */
    xh_char_t              version[XH_PARAM_LEN];
    xh_char_t              encoding[XH_PARAM_LEN];
    xh_char_t              root[XH_PARAM_LEN];
    xh_bool_t              utf8;
    xh_bool_t              xml_decl;
    xh_bool_t              keep_root;
    xh_bool_t              canonical;
    xh_char_t              content[XH_PARAM_LEN];
    xh_int_t               indent;
    VALUE                  output;
#ifdef XH_HAVE_DOM
    xh_bool_t              doc;
#endif
    xh_int_t               max_depth;
    xh_int_t               buf_size;
    xh_pattern_t           force_array;
    xh_bool_t              force_content;
    xh_bool_t              merge_text;
    xh_pattern_t           filter;
    xh_bool_t              block_given;

    /* LX options */
    xh_char_t              attr[XH_PARAM_LEN];
    size_t                 attr_len;
    xh_char_t              text[XH_PARAM_LEN];
    xh_bool_t              trim;
    xh_char_t              cdata[XH_PARAM_LEN];
    xh_char_t              comm[XH_PARAM_LEN];
} xh_opts_t;

xh_opts_t *xh_create_opts(void);
void xh_destroy_opts(xh_opts_t *opts);
xh_bool_t xh_init_opts(xh_opts_t *opts);
void xh_parse_args(xh_opts_t *opts, xh_int_t *nparam, xh_int_t argc, VALUE *argv);
void xh_copy_opts(xh_opts_t *dst, xh_opts_t *src);
void *xh_get_obj_param(xh_int_t *nparam, xh_int_t argc, VALUE *argv, const char *class);
VALUE xh_get_hash_param(xh_int_t *nparam, xh_int_t argc, VALUE *argv);
VALUE xh_get_str_param(xh_int_t *nparam, xh_int_t argc, VALUE *argv);
void xh_merge_opts(xh_opts_t *ctx_opts, xh_opts_t *opts, xh_int_t *nparam, xh_int_t argc, VALUE *argv);

#endif /* _XH_H_ */
