#ifndef _XH_WRITER_H_
#define _XH_WRITER_H_

#include "xh_config.h"
#include "xh_core.h"

#define XH_WRITER_RESIZE_BUFFER(w, b, l)                                \
    if (((l) + 1) > (size_t) xh_buffer_avail(b)) {                      \
        xh_writer_resize_buffer(w, (l) + 1);                            \
    }

typedef struct _xh_writer_t xh_writer_t;
struct _xh_writer_t {
#ifdef XH_HAVE_ENCODER
    xh_encoder_t          *encoder;
    xh_ruby_buffer_t       enc_buf;
#endif
    VALUE                  ruby_io;
    xh_ruby_buffer_t       main_buf;
    xh_int_t               indent;
    xh_int_t               indent_count;
    xh_bool_t              trim;
};

VALUE xh_writer_flush_buffer(xh_writer_t *writer, xh_ruby_buffer_t *buf);
VALUE xh_writer_flush(xh_writer_t *writer);
void xh_writer_resize_buffer(xh_writer_t *writer, size_t inc);
void xh_writer_destroy(xh_writer_t *writer);
void xh_writer_init(xh_writer_t *writer, xh_char_t *encoding, VALUE output, size_t size, xh_uint_t indent, xh_bool_t trim);

XH_INLINE void
xh_writer_flush_ruby_io(xh_ruby_buffer_t *buf, VALUE ruby_io, size_t use)
{
    rb_io_write(ruby_io, buf->scalar);
    xh_buffer_reset(buf);
}

XH_INLINE size_t
xh_writer_flush_ruby_buffer(xh_ruby_buffer_t *buf)
{
    size_t use = xh_buffer_use(buf);

    *buf->cur = '\0';
    rb_str_set_len(buf->scalar, use);

    return use;
}

#endif /* _XH_WRITER_H_ */
