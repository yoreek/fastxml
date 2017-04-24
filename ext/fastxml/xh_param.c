#include "xh_config.h"
#include "xh_core.h"

void
xh_param_assign_string(xh_char_t param[], VALUE value)
{
    xh_char_t *str;

    if ( RTEST(value) ) {
        str = (xh_char_t *) StringValueCStr(value);
        xh_str_copy(param, str, XH_PARAM_LEN);
    }
    else {
        *param = 0;
    }
}

void
xh_param_assign_int(xh_char_t *name, xh_int_t *param, VALUE value)
{
    Check_Type(value, T_FIXNUM);
    *param = FIX2INT(value);
}

xh_bool_t
xh_param_assign_bool(VALUE value)
{
    if ( RTEST(value) )
        return TRUE;

    return FALSE;
}

void
xh_param_assign_pattern(xh_pattern_t *patt, VALUE value)
{
    if ( RTEST(value) ) {
        patt->enable = TRUE;
        if ( RB_TYPE_P(value, T_REGEXP) || RB_TYPE_P(value, T_ARRAY) ) {
            patt->always = FALSE;
            patt->expr   = value;
        }
        else {
            patt->always = TRUE;
        }
    }
    else {
        patt->enable = FALSE;
    }
}

void
xh_param_assign_filter(xh_pattern_t *patt, VALUE value)
{
    if ( RTEST(value) ) {
        patt->enable = TRUE;
        patt->always = FALSE;
        patt->expr   = value;
    }
    else {
        patt->enable = FALSE;
    }
}

VALUE
xh_param_assign_cb(VALUE value)
{
    if ( !RTEST(value) )
        return Qnil;

    if (rb_cProc != rb_class_of(value) && rb_cMethod != rb_class_of(value))
      rb_raise(rb_eArgError, "Expected Proc or Method callback");

    //rb_funcall(value, rb_intern("call"), 0);

    return value;
}
