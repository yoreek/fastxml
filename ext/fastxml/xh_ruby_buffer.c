#include "xh_config.h"
#include "xh_core.h"

void
xh_ruby_buffer_init(xh_ruby_buffer_t *buf, size_t size)
{
    buf->scalar = rb_str_new(NULL, size);

    buf->start = buf->cur = XH_CHAR_CAST RSTRING_PTR(buf->scalar);
    buf->end   = buf->start + size;

    xh_log_debug2("buf: %p size: %lu", buf->start, size);
}

void
xh_ruby_buffer_grow(xh_ruby_buffer_t *buf, size_t inc)
{
    size_t size, use;

    if (inc <= (size_t) (buf->end - buf->cur))
        return;

    size = xh_ruby_buffer_size(buf);
    use  = xh_ruby_buffer_use(buf);

    xh_log_debug2("old buf: %p size: %lu", buf->start, size);

    size += inc < size ? size : inc;

    rb_str_set_len(buf->scalar, use);
    rb_str_resize(buf->scalar, size);

    buf->start = XH_CHAR_CAST RSTRING_PTR(buf->scalar);
    buf->cur   = buf->start + use;
    buf->end   = buf->start + size;

    xh_log_debug2("new buf: %p size: %lu", buf->start, size);
}

void
xh_ruby_buffer_sync(xh_ruby_buffer_t *buf)
{
    size_t use  = buf->cur - buf->start;
    size_t size = RSTRING_LEN(buf->scalar);

    buf->start  = XH_CHAR_CAST RSTRING_PTR(buf->scalar);
    buf->end    = buf->start + size;
    buf->cur    = buf->start + use;

    xh_log_debug2("buf: %p size: %lu", buf->start, size);
}
