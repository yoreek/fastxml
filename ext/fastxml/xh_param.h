#ifndef _XH_PARAM_H_
#define _XH_PARAM_H_

#include "xh_config.h"
#include "xh_core.h"

typedef struct {
    xh_bool_t              enable;
    xh_bool_t              always;
    VALUE                  expr;
} xh_pattern_t;

void xh_param_assign_string(xh_char_t param[], VALUE value);
void xh_param_assign_int(xh_char_t *name, xh_int_t *param, VALUE value);
xh_bool_t xh_param_assign_bool(VALUE value);
void xh_param_assign_pattern(xh_pattern_t *param, VALUE value);
void xh_param_assign_filter(xh_pattern_t *param, VALUE value);
VALUE xh_param_assign_cb(VALUE value);

#define XH_PARAM_LEN 32

#define XH_PARAM_READ_INIT                              \
    VALUE      value;                                   \
    xh_char_t *str;

#define XH_PARAM_READ_STRING(var, name)                 \
    value = rb_iv_get(xh_module, name);                 \
    if ( RTEST(value) ) {                               \
        str = XH_CHAR_CAST StringValueCStr(value);      \
        xh_str_copy(var, str, XH_PARAM_LEN);            \
    }                                                   \
    else {                                              \
        var[0] = '\0';                                  \
    }

#define XH_PARAM_READ_BOOL(var, name)                   \
    value = rb_iv_get(xh_module, name);                 \
    if ( RTEST(value) ) {                               \
        var = TRUE;                                     \
    }                                                   \
    else {                                              \
        var = FALSE;                                    \
    }

#define XH_PARAM_READ_INT(var, name)                    \
    value = rb_iv_get(xh_module, name);                 \
    var = FIX2INT(value);

#define XH_PARAM_READ_REF(var, name)                    \
    var = rb_iv_get(xh_module, name);

#define XH_PARAM_READ_PATTERN(var, name)                \
    value = rb_iv_get(xh_module, name);                 \
    xh_param_assign_pattern(&(var), value);

#endif /* _XH_PARAM_H_ */
