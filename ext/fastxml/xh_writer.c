#include "xh_config.h"
#include "xh_core.h"

void
xh_writer_resize_buffer(xh_writer_t *writer, size_t inc)
{
    (void) xh_writer_flush(writer);

    xh_ruby_buffer_grow(&writer->main_buf, inc);
}

VALUE
xh_writer_flush_buffer(xh_writer_t *writer, xh_ruby_buffer_t *buf)
{
    size_t use = xh_writer_flush_ruby_buffer(buf);

    if (writer->ruby_io != Qnil) {
        if (use > 0) xh_writer_flush_ruby_io(buf, writer->ruby_io, use);
        return Qnil;
    }

    return buf->scalar;
}

#ifdef XH_HAVE_ENCODER
void
xh_writer_encode_buffer(xh_writer_t *writer, xh_ruby_buffer_t *main_buf, xh_ruby_buffer_t *enc_buf)
{
    size_t len;

    /* 1 char -> 4 chars and '\0' */
    len = xh_buffer_use(main_buf) * 4 + 1;

    if (len > (size_t) (enc_buf->end - enc_buf->cur)) {
        xh_writer_flush_buffer(writer, enc_buf);

        xh_ruby_buffer_grow(enc_buf, len);
    }

    xh_encoder_encode_ruby_buffer(writer->encoder, main_buf, enc_buf);
}
#endif

VALUE
xh_writer_flush(xh_writer_t *writer)
{
    xh_ruby_buffer_t *buf;

#ifdef XH_HAVE_ENCODER
    if (writer->encoder != NULL) {
        xh_writer_encode_buffer(writer, &writer->main_buf, &writer->enc_buf);
        buf = &writer->enc_buf;
    }
    else {
        buf = &writer->main_buf;
    }
#else
    buf = &writer->main_buf;
#endif

    return xh_writer_flush_buffer(writer, buf);
}

void
xh_writer_destroy(xh_writer_t *writer)
{
#ifdef XH_HAVE_ENCODER
    xh_encoder_destroy(writer->encoder);
#endif
}

void
xh_writer_init(xh_writer_t *writer, xh_char_t *encoding, VALUE output, size_t size, xh_uint_t indent, xh_bool_t trim)
{
    writer->indent = indent;
    writer->trim   = trim;

    xh_ruby_buffer_init(&writer->main_buf, size);

    if (encoding[0] != '\0' && xh_strcasecmp(encoding, XH_INTERNAL_ENCODING) != 0) {
#ifdef XH_HAVE_ENCODER
        writer->encoder = xh_encoder_create(encoding, XH_CHAR_CAST XH_INTERNAL_ENCODING);
        if (writer->encoder == NULL) {
            rb_raise(xh_parse_error_class, "Can't create encoder for '%s'", encoding);
        }

        xh_ruby_buffer_init(&writer->enc_buf, size * 4);
#else
        rb_raise(xh_parse_error_class, "Can't create encoder for '%s'", encoding);
#endif
    }

    writer->ruby_io = output;
}
