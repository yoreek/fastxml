#ifndef _XH_CORE_H_
#define _XH_CORE_H_

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define XH_PPCAT_NX(A, B) A ## B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define XH_PPCAT(A, B) XH_PPCAT_NX(A, B)

/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define XH_STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define XH_STRINGIZE(A) XH_STRINGIZE_NX(A)

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE  (1)
#endif

#include "xh_log.h"
#include "xh_string.h"
#include "xh_sort.h"
#include "xh_stack.h"
#include "xh_param.h"
#include "xh_buffer_helper.h"
#include "xh_buffer.h"
#include "xh_ruby_buffer.h"
#include "xh_encoder.h"
#include "xh_reader.h"
#include "xh_writer.h"
#include "xh.h"
#include "xh_h2x.h"
#include "xh_xml.h"
#include "xh_x2h.h"
#endif /* _XH_CORE_H_ */
