#ifndef _XH_RUBY_BUFFER_H_
#define _XH_RUBY_BUFFER_H_

#include "xh_config.h"
#include "xh_core.h"

typedef struct _xh_ruby_buffer_t xh_ruby_buffer_t;
struct _xh_ruby_buffer_t {
    VALUE      scalar;
    xh_char_t *start;
    xh_char_t *cur;
    xh_char_t *end;
};

void xh_ruby_buffer_init(xh_ruby_buffer_t *buf, size_t size);
void xh_ruby_buffer_grow(xh_ruby_buffer_t *buf, size_t inc);
void xh_ruby_buffer_sync(xh_ruby_buffer_t *buf);

#define xh_ruby_buffer_avail(b)    ((b)->end - (b)->cur)
#define xh_ruby_buffer_start(b)    ((b)->start)
#define xh_ruby_buffer_pos(b)      ((b)->cur)
#define xh_ruby_buffer_end(b)      ((b)->end)
#define xh_ruby_buffer_size(b)     ((b)->end - (b)->start)
#define xh_ruby_buffer_use(b)      ((b)->cur - (b)->start)
#define xh_ruby_buffer_seek(b, p)  (b)->cur = p
#define xh_ruby_buffer_seek_eof(b) (b)->cur = (b)->end
#define xh_ruby_buffer_seek_top(b) (b)->cur = (b)->start
#define xh_ruby_buffer_grow50(b)   xh_ruby_buffer_grow((b), xh_ruby_buffer_size(b) / 2)

#endif /* _XH_RUBY_BUFFER_H_ */
