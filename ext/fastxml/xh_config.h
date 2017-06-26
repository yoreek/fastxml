#ifndef _XH_CONFIG_H_
#define _XH_CONFIG_H_

#include "ruby.h"
#include "ruby/io.h"
#include "ruby/re.h"
#if HAVE_RUBY_ENCODING_H
#include "ruby/encoding.h"
#endif
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/mman.h>
#endif

#if __GNUC__ >= 3
# define expect(expr,value)         __builtin_expect ((expr), (value))
# define XH_INLINE                  static inline
# define XH_UNUSED(v)               x __attribute__((unused))
#else
# define expect(expr,value)         (expr)
# define XH_INLINE                  static
# define XH_UNUSED(v)               v
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#define expect_false(expr) expect ((expr) != 0, 0)
#define expect_true(expr)  expect ((expr) != 0, 1)

typedef uintptr_t xh_bool_t;
typedef uintptr_t xh_uint_t;
typedef intptr_t  xh_int_t;
typedef u_char    xh_char_t;

#define XH_CHAR_CAST    (xh_char_t *)
#define XH_EMPTY_STRING (XH_CHAR_CAST "")

#if defined(XH_HAVE_ICONV) || defined(XH_HAVE_ICU)
#define XH_HAVE_ENCODER
#endif

#ifdef HAVE_RUBY_ENCODING_H
#define XH_FORCE_UTF8(s) rb_enc_set_index(s, rb_utf8_encindex())
#else
#define XH_FORCE_UTF8(s)
#endif

#if defined(XH_HAVE_XML2) && defined(XH_HAVE_XML__LIBXML)
#define XH_HAVE_DOM
#endif

#ifdef XH_HAVE_DOM
#include <libxml/parser.h>
#endif

#define XH_HAVE_MMAP

extern VALUE xh_module;
extern VALUE xh_parse_error_class;
extern ID    xh_id_next;
extern ID    xh_id_initialize;

#endif /* _XH_CONFIG_H_ */
