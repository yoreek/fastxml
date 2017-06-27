#include "xh_config.h"
#include "xh_core.h"
#include "xh_ruby_hash.h"

static const char DEF_CONTENT_KEY[] = "content";

#define _NEW_STRING(s, l, utf8)                                         \
    ((utf8) ? rb_utf8_str_new((const char *) (s), l) : rb_str_new((const char *) (s), l))

#define NEW_STRING(s, l)                                                \
    _NEW_STRING(s, l, ctx->opts.utf8)

#define CAT_STRING(v, s, l)                                             \
    rb_str_cat((v), (const char *) (s), (l));

#define SAVE_VALUE(lv, v , s, l)                                        \
    xh_log_trace2("save value: [%.*s]", (int) (l), s);                  \
    if ( RTEST(v) ) {                                                   \
        xh_log_trace0("add to array");                                  \
        /* get array if value is reference to array */                  \
        if ( RB_TYPE_P(v, RUBY_T_ARRAY) ) {                             \
            av = v;                                                     \
        }                                                               \
        /* create a new array and move value to array */                \
        else {                                                          \
            av = rb_ary_new();                                          \
            *(lv) = av;                                                 \
            rb_ary_push(av, v);                                         \
            (v) = *(lv);                                                \
        }                                                               \
        /* add value to array */                                        \
        rb_ary_push(av, NEW_STRING((s), (l)));                          \
        (lv) = (VALUE *) &RARRAY_CONST_PTR(av)[RARRAY_LEN(av) - 1];     \
    }                                                                   \
    else {                                                              \
        xh_log_trace0("set string");                                    \
        *(lv) = NEW_STRING(s, l);                                       \
    }                                                                   \

#define _OPEN_TAG(s, l)                                                 \
    val = *lval;                                                        \
    /* if content exists that move to hash with 'content' key */        \
    if ( RB_TYPE_P(val, RUBY_T_STRING) || RB_TYPE_P(val, RUBY_T_ARRAY)) {\
        *lval = hash_new();                                             \
        if (RB_TYPE_P(val, RUBY_T_HASH) || RB_TYPE_P(val, RUBY_T_ARRAY) || (RB_TYPE_P(val, RUBY_T_STRING) && RSTRING_LEN(val))) {\
            (void) hash_store(*lval, (const char *) content_key, content_key_len, val);\
        }                                                               \
        val = *lval;                                                    \
    }                                                                   \
    /* fetch existen or create empty hash entry */                      \
    lval = hash_fetch(val, (const char *) s, l, Qnil);                  \
    /* save as empty string */                                          \
    val = *lval;                                                        \
    xh_log_trace0("save as empty string");                              \
    SAVE_VALUE(lval, val, "", 0)                                        \
    if (++depth >= ctx->opts.max_depth) goto MAX_DEPTH_EXCEEDED;        \
    nodes[depth].lval = lval;                                           \
    nodes[depth].flags = XH_X2H_NODE_FLAG_NONE;                         \
    if (depth > 1 && ctx->opts.force_array.enable && !RB_TYPE_P(val, RUBY_T_ARRAY) \
        && (ctx->opts.force_array.always || xh_x2h_match_node(s, l, ctx->opts.force_array.expr))\
    ) {                                                                 \
        nodes[depth].flags |= XH_X2H_NODE_FLAG_FORCE_ARRAY;             \
    }                                                                   \
    (s) = NULL;

#define OPEN_TAG(s, l)                                                  \
    xh_log_trace2("new tag: [%.*s]", (int) (l), s);                     \
    if (real_depth == 0) {                                              \
        if (flags & XH_X2H_ROOT_FOUND) goto INVALID_XML;                \
        flags |= XH_X2H_ROOT_FOUND;                                     \
    }                                                                   \
    if (XH_X2H_FILTER_SEARCH(flags)) {                                  \
        xh_x2h_xpath_update(ctx->xpath, s, l);                          \
        if (xh_x2h_match_node(ctx->xpath, xh_strlen(ctx->xpath), ctx->opts.filter.expr)) {\
            xh_log_trace2("match node: [%.*s]", (int) (l), s);          \
            ctx->hash = hash_new();                                     \
            nodes[0].lval = lval = &ctx->hash;                          \
            depth = 0;                                                  \
            flags |= XH_X2H_FILTER_MATCHED;                             \
        }                                                               \
    }                                                                   \
    if (!XH_X2H_FILTER_SEARCH(flags)) {                                 \
        _OPEN_TAG(s, l)                                                 \
    }                                                                   \
    real_depth++;

#define _CLOSE_TAG                                                      \
    val = *nodes[depth].lval;                                           \
    if (ctx->opts.force_content && RB_TYPE_P(val, RUBY_T_STRING)) {     \
        lval = nodes[depth].lval;                                       \
        *lval = hash_new();                                             \
        (void) hash_store(*lval, (const char *) content_key, content_key_len, val);\
        val = *lval;                                                    \
    }                                                                   \
    if ((nodes[depth].flags & XH_X2H_NODE_FLAG_FORCE_ARRAY)             \
        && (RB_TYPE_P(val, RUBY_T_STRING) || !RB_TYPE_P(val, RUBY_T_ARRAY))\
    ) {                                                                 \
        lval = nodes[depth].lval;                                       \
        av = rb_ary_new();                                              \
        *lval = av;                                                     \
        rb_ary_push(av, val);                                           \
    }                                                                   \
    lval = nodes[--depth].lval;

#define CLOSE_TAG                                                       \
    xh_log_trace0("close tag");                                         \
    if (real_depth == 0) goto INVALID_XML;                              \
    if (!XH_X2H_FILTER_SEARCH(flags)) {                                 \
        _CLOSE_TAG                                                      \
    }                                                                   \
    if ((flags & XH_X2H_FILTER_MATCHED) && depth == 0) {                \
        xh_log_trace0("match node finished");                           \
        val = *nodes[0].lval;                                           \
        if (ctx->opts.block_given) {                                    \
            rb_yield(val);                                              \
        }                                                               \
        else {                                                          \
            rb_ary_push(ctx->result, val);                              \
        }                                                               \
        flags ^= XH_X2H_FILTER_MATCHED;                                 \
    }                                                                   \
    if ((flags & (XH_X2H_FILTER_ENABLED | XH_X2H_FILTER_MATCHED)) == XH_X2H_FILTER_ENABLED) {\
        xh_x2h_xpath_update(ctx->xpath, NULL, 0);                       \
    }                                                                   \
    real_depth--;

#define NEW_NODE_ATTRIBUTE(k, kl, v, vl)                                \
    if (!XH_X2H_FILTER_SEARCH(flags)) {                                 \
        _OPEN_TAG(k, kl)                                                \
        _NEW_TEXT(v, vl)                                                \
        _CLOSE_TAG                                                      \
    }

#define NEW_XML_DECL_ATTRIBUTE(k, kl, v, vl)                            \
    xh_log_trace4("new xml decl attr name: [%.*s] value: [%.*s]", (int) (kl), k, (int) (vl), v);\
    /* save encoding parameter to converter context if param found */   \
    if ((kl) == (sizeof("encoding") - 1) &&                             \
        xh_strncmp((k), XH_CHAR_CAST "encoding", sizeof("encoding") - 1) == 0) {\
        xh_str_range_copy(ctx->encoding, XH_CHAR_CAST (v), vl, XH_PARAM_LEN);\
    }                                                                   \
    (k) = (v) = NULL;

#define NEW_ATTRIBUTE(k, kl, v, vl) NEW_NODE_ATTRIBUTE(k, kl, v, vl)

#define _NEW_TEXT(s, l)                                                 \
    val = *lval;                                                        \
    if ( !RB_TYPE_P(val, RUBY_T_STRING) ) {                             \
        xh_log_trace0("add to array");                                  \
        /* add content to array*/                                       \
        if ( RB_TYPE_P(val, RUBY_T_ARRAY) ) {                           \
            av = val;                                                   \
            rb_ary_push(av, NEW_STRING(s, l));                          \
        }                                                               \
        /* save content to hash with "content" key */                   \
        else {                                                          \
            xh_log_trace0("save to hash");                              \
            lval = hash_fetch(val, (const char *) content_key, content_key_len, Qnil);\
            val = *lval;                                                \
            SAVE_VALUE(lval, val, s, l)                                 \
            lval = nodes[depth].lval;                                   \
        }                                                               \
    }                                                                   \
    else if (RSTRING_LEN(val) && !ctx->opts.merge_text) {               \
        xh_log_trace0("create a new array");                            \
        xh_log_trace1("create a new array val: %s", StringValueCStr(val));\
        /* content already exists, create a new array and move*/        \
        /* old and new content to array */                              \
        *lval = av = rb_ary_new();                                      \
        rb_ary_push(av, val);                                           \
        rb_ary_push(av, NEW_STRING(s, l));                              \
    }                                                                   \
    else {                                                              \
        xh_log_trace0("concat");                                        \
        /* concatenate with previous string */                          \
        CAT_STRING(val, s, l)                                           \
    }                                                                   \

#define NEW_TEXT(s, l)                                                  \
    xh_log_trace2("new text: [%.*s]", (int) (l), s);                    \
    if (real_depth == 0) goto INVALID_XML;                              \
    if (!XH_X2H_FILTER_SEARCH(flags)) {                                 \
        _NEW_TEXT(s, l)                                                 \
    }

#define NEW_COMMENT(s, l) (s) = NULL;

#define NEW_CDATA(s, l) NEW_TEXT(s, l)

#define CHECK_EOF_WITH_CHUNK(loop)                                      \
    if (cur >= eof || *cur == '\0') {                                   \
        eof = cur;                                                      \
        if (terminate) goto XH_PPCAT(loop, _FINISH);                    \
        ctx->state = XH_PPCAT(loop, _START);                            \
        goto CHUNK_FINISH;                                              \
    }                                                                   \

#define CHECK_EOF_WITHOUT_CHUNK(loop)                                   \
    if (cur >= eof || *cur == '\0') goto XH_PPCAT(loop, _FINISH);       \

#define CHECK_EOF(loop) CHECK_EOF_WITH_CHUNK(loop)

#define DO(loop)                                                        \
XH_PPCAT(loop, _START):                                                 \
    CHECK_EOF(loop)                                                     \
    c = *cur++;                                                         \
    xh_log_trace3("'%c'=[0x%X] %s start", c, c, XH_STRINGIZE(loop));    \
    switch (c) {

#define _DO(loop)                                                       \
XH_PPCAT(loop, _START):                                                 \
    CHECK_EOF_WITHOUT_CHUNK(loop)                                       \
    c = *cur++;                                                         \
    xh_log_trace3("'%c'=[0x%X] %s start", c, c, XH_STRINGIZE(loop));    \
    switch (c) {

#define END(loop)                                                       \
    }                                                                   \
    xh_log_trace1("           %s end", XH_STRINGIZE(loop));             \
    goto XH_PPCAT(loop, _START);                                        \
XH_PPCAT(loop, _FINISH):

#define EXPECT_ANY(desc)                                                \
    default: xh_log_trace3("'%c'=[0x%X] - %s expected", c, c, desc);

#define EXPECT_CHAR(desc, c1)                                           \
    case c1: xh_log_trace3("'%c'=[0x%X] - %s expected", c, c, desc);

#define EXPECT_BLANK_WO_CR(desc)                                        \
    case ' ': case '\t': case '\n':                                     \
        xh_log_trace3("'%c'=[0x%X] - %s expected", c, c, desc);

#define EXPECT_BLANK(desc)                                              \
    case ' ': case '\t': case '\n': case '\r':                          \
        xh_log_trace3("'%c'=[0x%X] - %s expected", c, c, desc);

#define EXPECT_DIGIT(desc)                                              \
    case '0': case '1': case '2': case '3': case '4':                   \
    case '5': case '6': case '7': case '8': case '9':                   \
        xh_log_trace3("'%c'=[0x%X] - %s expected", c, c, desc);

#define EXPECT_HEX_CHAR_LC(desc)                                        \
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':         \
        xh_log_trace3("'%c'=[0x%X] - %s expected", c, c, desc);

#define EXPECT_HEX_CHAR_UC(desc)                                        \
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':         \
        xh_log_trace3("'%c'=[0x%X] - %s expected", c, c, desc);

#define SKIP_BLANK                                                      \
    EXPECT_BLANK("skip blank") break;

#define SCAN2(loop, c1, c2)                                             \
    DO(XH_PPCAT(loop, _1)) EXPECT_CHAR(XH_STRINGIZE(c1), c1)            \
    DO(XH_PPCAT(loop, _2)) EXPECT_CHAR(XH_STRINGIZE(c2), c2)

#define END2(loop, stop)                                                \
    EXPECT_ANY("wrong character") goto stop;                            \
    END(XH_PPCAT(loop, _2))       goto stop;                            \
    EXPECT_ANY("wrong character") goto stop;                            \
    END(XH_PPCAT(loop, _1))

#define SCAN3(loop, c1, c2, c3)                                         \
    DO(XH_PPCAT(loop, _1)) EXPECT_CHAR(STRINGIZE(c1), c1)               \
    DO(XH_PPCAT(loop, _2)) EXPECT_CHAR(STRINGIZE(c2), c2)               \
    DO(XH_PPCAT(loop, _3)) EXPECT_CHAR(STRINGIZE(c3), c3)

#define END3(loop, stop)                                                \
    EXPECT_ANY("wrong character") goto stop;                            \
    END(XH_PPCAT(loop, _3))          goto stop;                         \
    EXPECT_ANY("wrong character") goto stop;                            \
    END(XH_PPCAT(loop, _2))          goto stop;                         \
    EXPECT_ANY("wrong character") goto stop;                            \
    END(XH_PPCAT(loop, _1))

#define SCAN5(loop, c1, c2, c3, c4, c5)                                 \
    SCAN3(XH_PPCAT(loop, _1), c1, c2, c3)                               \
    SCAN2(XH_PPCAT(loop, _2), c4, c5)

#define END5(loop, stop)                                                \
    END2(XH_PPCAT(loop, _2), stop)                                      \
    END3(XH_PPCAT(loop, _1), stop)

#define SCAN6(loop, c1, c2, c3, c4, c5, c6)                             \
    SCAN3(XH_PPCAT(loop, _1), c1, c2, c3)                               \
    SCAN3(XH_PPCAT(loop, _2), c4, c5, c6)

#define END6(loop, stop)                                                \
    END3(XH_PPCAT(loop, _2), stop)                                      \
    END3(XH_PPCAT(loop, _1), stop)

#define SEARCH_END_TAG                                                  \
    EXPECT_CHAR("end tag", '>')                                         \
        goto PARSE_CONTENT;                                             \
    EXPECT_CHAR("self closing tag", '/')                                \
        CLOSE_TAG                                                       \
        DO(SEARCH_END_TAG)                                              \
            EXPECT_CHAR("end tag", '>')                                 \
                goto PARSE_CONTENT;                                     \
            EXPECT_ANY("wrong character")                               \
                goto INVALID_XML;                                       \
        END(SEARCH_END_TAG)                                             \
        goto INVALID_XML;

#define SEARCH_NODE_ATTRIBUTE_VALUE(loop, top_loop, quot)               \
    EXPECT_CHAR("start attr value", quot)                               \
        content = cur;                                                  \
        flags &= ~XH_X2H_NEED_NORMALIZE;                                \
        DO(XH_PPCAT(loop, _END_ATTR_VALUE))                             \
            EXPECT_CHAR("attr value end", quot)                         \
                if (flags & XH_X2H_NEED_NORMALIZE) {                    \
                    NORMALIZE_TEXT(loop, content, cur - content - 1)    \
                    NEW_ATTRIBUTE(node, end - node, enc, enc_len)       \
                }                                                       \
                else {                                                  \
                    NEW_ATTRIBUTE(node, end - node, content, cur - content - 1)\
                }                                                       \
                goto top_loop;                                          \
            EXPECT_CHAR("CR", '\r')                                     \
                flags |= XH_X2H_NORMALIZE_LINE_FEED;                    \
                break;                                                  \
            EXPECT_CHAR("reference", '&')                               \
                flags |= XH_X2H_NORMALIZE_REF;                          \
                break;                                                  \
        END(XH_PPCAT(loop, _END_ATTR_VALUE))                            \
        goto INVALID_XML;

#define SEARCH_XML_DECL_ATTRIBUTE_VALUE(loop, top_loop, quot)           \
    EXPECT_CHAR("start attr value", quot)                               \
        content = cur;                                                  \
        DO(XH_PPCAT(loop, _END_ATTR_VALUE))                             \
            EXPECT_CHAR("attr value end", quot)                         \
                NEW_ATTRIBUTE(node, end - node, content, cur - content - 1)\
                goto top_loop;                                          \
        END(XH_PPCAT(loop, _END_ATTR_VALUE))                            \
        goto INVALID_XML;

#define SEARCH_ATTRIBUTE_VALUE(loop, top_loop, quot) SEARCH_NODE_ATTRIBUTE_VALUE(loop, top_loop, quot)

#define SEARCH_ATTRIBUTES(loop, search_end_tag)                         \
XH_PPCAT(loop, _SEARCH_ATTRIBUTES_LOOP):                                \
    DO(XH_PPCAT(loop, _SEARCH_ATTR))                                    \
        search_end_tag                                                  \
                                                                        \
        SKIP_BLANK                                                      \
                                                                        \
        EXPECT_ANY("start attr name")                                   \
            node = cur - 1;                                             \
                                                                        \
            DO(XH_PPCAT(loop, _PARSE_ATTR_NAME))                        \
                EXPECT_BLANK("end attr name")                           \
                    end = cur - 1;                                      \
                    xh_log_trace2("attr name: [%.*s]", (int) (end - node), node);\
                                                                        \
                    DO(XH_PPCAT(loop, _ATTR_SKIP_BLANK))                \
                        EXPECT_CHAR("search attr value", '=')           \
                            goto XH_PPCAT(loop, _SEARCH_ATTRIBUTE_VALUE);\
                        SKIP_BLANK                                      \
                        EXPECT_ANY("wrong character")                   \
                            goto INVALID_XML;                           \
                    END(XH_PPCAT(loop, _ATTR_SKIP_BLANK))               \
                    goto INVALID_XML;                                   \
                EXPECT_CHAR("end attr name", '=')                       \
                    end = cur - 1;                                      \
                    xh_log_trace2("attr name: [%.*s]", (int) (end - node), node);\
                                                                        \
XH_PPCAT(loop, _SEARCH_ATTRIBUTE_VALUE):                                \
                    DO(XH_PPCAT(loop, _PARSE_ATTR_VALUE))               \
                        SEARCH_ATTRIBUTE_VALUE(XH_PPCAT(loop, _1), XH_PPCAT(loop, _SEARCH_ATTRIBUTES_LOOP), '"')\
                        SEARCH_ATTRIBUTE_VALUE(XH_PPCAT(loop, _2), XH_PPCAT(loop, _SEARCH_ATTRIBUTES_LOOP), '\'')\
                        SKIP_BLANK                                      \
                        EXPECT_ANY("wrong character")                   \
                            goto INVALID_XML;                           \
                    END(XH_PPCAT(loop, _PARSE_ATTR_VALUE))              \
                    goto INVALID_XML;                                   \
            END(XH_PPCAT(loop, _PARSE_ATTR_NAME))                       \
            goto INVALID_XML;                                           \
    END(XH_PPCAT(loop, _SEARCH_ATTR))                                   \
    goto INVALID_XML;

#define PARSE_XML_DECLARATION                                           \
    SCAN3(XML_DECL, 'x', 'm', 'l')                                      \
        DO(XML_DECL_ATTR)                                               \
            EXPECT_BLANK("blank")                                       \
                SEARCH_ATTRIBUTES(XML_DECL_ATTR, SEARCH_END_XML_DECLARATION)\
                goto INVALID_XML;                                       \
            EXPECT_ANY("wrong character")                               \
                goto INVALID_XML;                                       \
        END(XML_DECL_ATTR)                                              \
        goto INVALID_XML;                                               \
    END3(XML_DECL, INVALID_XML)                                         \
    goto INVALID_XML;

#define SEARCH_END_XML_DECLARATION                                      \
    EXPECT_CHAR("end tag", '?')                                         \
        DO(XML_DECL_SEARCH_END_TAG2)                                    \
            EXPECT_CHAR("end tag", '>')                                 \
                goto XML_DECL_FOUND;                                    \
            EXPECT_ANY("wrong character")                               \
                goto INVALID_XML;                                       \
        END(XML_DECL_SEARCH_END_TAG2)                                   \
        goto INVALID_XML;

#define PARSE_COMMENT                                                   \
    DO(COMMENT1)                                                        \
        EXPECT_CHAR("-", '-')                                           \
            content = NULL;                                             \
            DO(END_COMMENT1)                                            \
                SKIP_BLANK                                              \
                EXPECT_CHAR("1st -", '-')                               \
                    if (content == NULL) content = end = cur - 1;       \
                    DO(END_COMMENT2)                                    \
                        EXPECT_CHAR("2nd -", '-')                       \
                            DO(END_COMMENT3)                            \
                                EXPECT_CHAR(">", '>')                   \
                                    NEW_COMMENT(content, end - content) \
                                    goto PARSE_CONTENT;                 \
                                EXPECT_CHAR("2nd -", '-')               \
                                    end = cur - 2;                      \
                                    goto END_COMMENT3_START;            \
                                EXPECT_ANY("any character")             \
                                    end = cur - 1;                      \
                                    goto END_COMMENT1_START;            \
                            END(END_COMMENT3)                           \
                        EXPECT_BLANK("skip blank")                      \
                            end = cur - 1;                              \
                            goto END_COMMENT1_START;                    \
                        EXPECT_ANY("any character")                     \
                            end = cur;                                  \
                            goto END_COMMENT1_START;                    \
                    END(END_COMMENT2)                                   \
                EXPECT_ANY("any char")                                  \
                    if (content == NULL) content = cur - 1;             \
                    end = cur;                                          \
            END(END_COMMENT1)                                           \
            goto INVALID_XML;                                           \
                                                                        \
        EXPECT_ANY("wrong character")                                   \
            goto INVALID_XML;                                           \
                                                                        \
    END(COMMENT1)                                                       \
    goto INVALID_XML;

#define PARSE_CDATA                                                     \
    SCAN6(CDATA, 'C', 'D', 'A', 'T', 'A', '[')                          \
        content = end = cur;                                            \
        DO(END_CDATA1)                                                  \
            EXPECT_CHAR("1st ]", ']')                                   \
                DO(END_CDATA2)                                          \
                    EXPECT_CHAR("2nd ]", ']')                           \
                        DO(END_CDATA3)                                  \
                            EXPECT_CHAR(">", '>')                       \
                                end = cur - 3;                          \
                                NEW_CDATA(content, end - content)       \
                                goto PARSE_CONTENT;                     \
                            EXPECT_CHAR("2nd ]", ']')                   \
                                goto END_CDATA3_START;                  \
                            EXPECT_ANY("any character")                 \
                                goto END_CDATA1_START;                  \
                        END(END_CDATA3)                                 \
                    EXPECT_ANY("any character")                         \
                        goto END_CDATA1_START;                          \
                END(END_CDATA2)                                         \
                ;                                                       \
        END(END_CDATA1)                                                 \
        goto INVALID_XML;                                               \
    END6(CDATA, INVALID_XML)

#define PARSE_CDATA_WITH_TRIM                                           \
    SCAN6(CDATA_WITH_TRIM, 'C', 'D', 'A', 'T', 'A', '[')                \
        content = NULL;                                                 \
        DO(END_CDATA_WITH_TRIM1)                                        \
            SKIP_BLANK                                                  \
            EXPECT_CHAR("1st ]", ']')                                   \
                if (content == NULL) content = end = cur - 1;           \
                DO(END_CDATA_WITH_TRIM2)                                \
                    EXPECT_CHAR("2nd ]", ']')                           \
                        DO(END_CDATA_WITH_TRIM3)                        \
                            EXPECT_CHAR(">", '>')                       \
                                NEW_CDATA(content, end - content)       \
                                goto PARSE_CONTENT;                     \
                            EXPECT_CHAR("2nd ]", ']')                   \
                                end = cur - 2;                          \
                                goto END_CDATA_WITH_TRIM3_START;        \
                            EXPECT_ANY("any character")                 \
                                end = cur - 1;                          \
                                goto END_CDATA_WITH_TRIM1_START;        \
                        END(END_CDATA_WITH_TRIM3)                       \
                    EXPECT_BLANK("skip blank")                          \
                        end = cur - 1;                                  \
                        goto END_CDATA_WITH_TRIM1_START;                \
                    EXPECT_ANY("any character")                         \
                        end = cur;                                      \
                        goto END_CDATA_WITH_TRIM1_START;                \
                END(END_CDATA_WITH_TRIM2)                               \
            EXPECT_ANY("any char")                                      \
                if (content == NULL) content = cur - 1;                 \
                end = cur;                                              \
        END(END_CDATA_WITH_TRIM1)                                       \
        goto INVALID_XML;                                               \
    END6(CDATA_WITH_TRIM, INVALID_XML)

#define NORMALIZE_REFERENCE(loop)                                       \
    _DO(XH_PPCAT(loop, _REFERENCE))                                     \
        EXPECT_CHAR("char reference", '#')                              \
            _DO(XH_PPCAT(loop, _CHAR_REFERENCE))                        \
                EXPECT_CHAR("hex", 'x')                                 \
                    code = 0;                                           \
                    _DO(XH_PPCAT(loop, _HEX_CHAR_REFERENCE_LOOP))       \
                        EXPECT_DIGIT("hex digit")                       \
                            code = code * 16 + (c - '0');               \
                            break;                                      \
                        EXPECT_HEX_CHAR_LC("hex a-f")                   \
                            code = code * 16 + (c - 'a') + 10;          \
                            break;                                      \
                        EXPECT_HEX_CHAR_UC("hex A-F")                   \
                            code = code * 16 + (c - 'A') + 10;          \
                            break;                                      \
                        EXPECT_CHAR("reference end", ';')               \
                            goto XH_PPCAT(loop, _REFEFENCE_VALUE);      \
                    END(XH_PPCAT(loop, _HEX_CHAR_REFERENCE_LOOP))       \
                    goto INVALID_REF;                                   \
                EXPECT_DIGIT("digit")                                   \
                    code = (c - '0');                                   \
                    _DO(XH_PPCAT(loop, _CHAR_REFERENCE_LOOP))           \
                        EXPECT_DIGIT("digit")                           \
                            code = code * 10 + (c - '0');               \
                            break;                                      \
                        EXPECT_CHAR("reference end", ';')               \
                            goto XH_PPCAT(loop, _REFEFENCE_VALUE);      \
                    END(XH_PPCAT(loop, _CHAR_REFERENCE_LOOP))           \
                    goto INVALID_REF;                                   \
                EXPECT_ANY("any char")                                  \
                    goto INVALID_REF;                                   \
            END(XH_PPCAT(loop, _CHAR_REFERENCE))                        \
            goto INVALID_REF;                                           \
        EXPECT_CHAR("amp or apos", 'a')                                 \
            if (xh_str_equal3(cur, 'm', 'p', ';')) {                    \
                code = '&';                                             \
                cur += 3;                                               \
                goto XH_PPCAT(loop, _REFEFENCE_VALUE);                  \
            }                                                           \
            if (xh_str_equal4(cur, 'p', 'o', 's', ';')) {               \
                code = '\'';                                            \
                cur += 4;                                               \
                goto XH_PPCAT(loop, _REFEFENCE_VALUE);                  \
            }                                                           \
            goto INVALID_REF;                                           \
        EXPECT_CHAR("lt", 'l')                                          \
            if (xh_str_equal2(cur, 't', ';')) {                         \
                code = '<';                                             \
                cur += 2;                                               \
                goto XH_PPCAT(loop, _REFEFENCE_VALUE);                  \
            }                                                           \
            goto INVALID_REF;                                           \
        EXPECT_CHAR("gt", 'g')                                          \
            if (xh_str_equal2(cur, 't', ';')) {                         \
                code = '>';                                             \
                cur += 2;                                               \
                goto XH_PPCAT(loop, _REFEFENCE_VALUE);                  \
            }                                                           \
            goto INVALID_REF;                                           \
        EXPECT_CHAR("quot", 'q')                                        \
            if (xh_str_equal4(cur, 'u', 'o', 't', ';')) {               \
                code = '"';                                             \
                cur += 4;                                               \
                goto XH_PPCAT(loop, _REFEFENCE_VALUE);                  \
            }                                                           \
            goto INVALID_REF;                                           \
        EXPECT_ANY("any char")                                          \
            goto INVALID_REF;                                           \
    END(XH_PPCAT(loop, _REFERENCE))                                     \
    goto INVALID_REF;                                                   \
XH_PPCAT(loop, _REFEFENCE_VALUE):                                       \
    xh_log_trace1("parse reference value: %u", code);                   \
    if (code == 0 || code > 0x10FFFF) goto INVALID_REF;                 \
    if (code >= 0x80) {                                                 \
        if (code < 0x800) {                                             \
            *enc_cur++ = (code >>  6) | 0xC0;  bits =  0;               \
        }                                                               \
        else if (code < 0x10000) {                                      \
            *enc_cur++ = (code >> 12) | 0xE0;  bits =  6;               \
        }                                                               \
        else if (code < 0x110000) {                                     \
            *enc_cur++ = (code >> 18) | 0xF0;  bits =  12;              \
        }                                                               \
        else {                                                          \
            goto INVALID_REF;                                           \
        }                                                               \
        for (; bits >= 0; bits-= 6) {                                   \
            *enc_cur++ = ((code >> bits) & 0x3F) | 0x80;                \
        }                                                               \
    }                                                                   \
    else {                                                              \
        *enc_cur++ = (xh_char_t) code;                                  \
    }

#define NORMALIZE_LINE_FEED(loop)                                       \
    _DO(XH_PPCAT(loop, _NORMALIZE_LINE_FEED))                           \
        EXPECT_CHAR("LF", '\n')                                         \
            goto XH_PPCAT(loop, _NORMALIZE_LINE_FEED_END);              \
        EXPECT_ANY("any char")                                          \
            cur--;                                                      \
            goto XH_PPCAT(loop, _NORMALIZE_LINE_FEED_END);              \
    END(XH_PPCAT(loop, _NORMALIZE_LINE_FEED))                           \
XH_PPCAT(loop, _NORMALIZE_LINE_FEED_END):                               \
    *enc_cur++ = '\n';

#define NORMALIZE_TEXT(loop, s, l)                                      \
    enc_len = l;                                                        \
    if (enc_len) {                                                      \
        old_cur = cur;                                                  \
        old_eof = eof;                                                  \
        cur     = s;                                                    \
        eof     = cur + enc_len;                                        \
        if (ctx->tmp == NULL) {                                         \
            xh_log_trace1("malloc() %lu", enc_len);                     \
            if ((ctx->tmp = malloc(enc_len)) == NULL) goto MALLOC;      \
            ctx->tmp_size = enc_len;                                    \
        }                                                               \
        else if (enc_len > ctx->tmp_size) {                             \
            xh_log_trace1("realloc() %lu", enc_len);                    \
            if ((enc = realloc(ctx->tmp, enc_len)) == NULL) goto MALLOC;\
            ctx->tmp = enc;                                             \
            ctx->tmp_size = enc_len;                                    \
        }                                                               \
        enc = enc_cur = ctx->tmp;                                       \
        memcpy(enc, cur, enc_len);                                      \
        _DO(XH_PPCAT(loop, _NORMALIZE_TEXT))                            \
            EXPECT_CHAR("reference", '&')                               \
                NORMALIZE_REFERENCE(loop)                               \
                break;                                                  \
            EXPECT_CHAR("CR", '\r')                                     \
                NORMALIZE_LINE_FEED(loop)                               \
                break;                                                  \
            EXPECT_ANY("any char")                                      \
                *enc_cur++ = c;                                         \
        END(XH_PPCAT(loop, _NORMALIZE_TEXT))                            \
        enc_len = enc_cur - enc;                                        \
        cur = old_cur;                                                  \
        eof = old_eof;                                                  \
    }                                                                   \
    else {                                                              \
        enc = s;                                                        \
    }

XH_INLINE void
xh_x2h_xpath_update(xh_char_t *xpath, xh_char_t *name, size_t name_len)
{
    size_t len;

    len = xh_strlen(xpath);
    if (name != NULL) {
        if ((len + name_len + 1) > XH_X2H_XPATH_MAX_LEN)
            rb_raise(xh_parse_error_class, "XPath too long");

        xpath[len++] = '/';
        for (;name_len--;) xpath[len++] = *name++;
    }
    else if (len == 0) {
        rb_raise(xh_parse_error_class, "Can't update xpath, something wrong!");
    }
    else {
        for (;--len && xpath[len] != '/';) {/* void */}
    }
    xpath[len] = '\0';

    xh_log_trace1("xpath: [%s]", xpath);
}

XH_INLINE xh_bool_t
xh_x2h_match_node(xh_char_t *name, size_t name_len, VALUE expr)
{
    size_t     i, l;
    VALUE      ary;
    VALUE      str;
    xh_char_t *expr_str;
    size_t     expr_len;

    xh_log_trace2("match node: [%.*s]", (int) name_len, name);

    str = _NEW_STRING(name, name_len, TRUE);

    if ( RB_TYPE_P(expr, RUBY_T_REGEXP) ) {
        if (rb_reg_search(expr, str, 0, 0) >= 0) return TRUE;
    }
    else if ( RB_TYPE_P(expr, RUBY_T_ARRAY) ) {
        l = RARRAY_LEN(expr);
        ary = expr;
        for (i = 0; i < l; i++) {
            expr = RARRAY_AREF(ary, i);
            if ( RB_TYPE_P(expr, RUBY_T_REGEXP) ) {
                if (rb_reg_search(expr, str, 0, 0) >= 0) return TRUE;
            }
            else {
                expr_str = XH_CHAR_CAST RSTRING_PTR(expr);
                expr_len = RSTRING_LEN(expr);
                if (name_len == expr_len && !xh_strncmp(name, expr_str, name_len)) {
                    return TRUE;
                }
            }
        }
    } else {
        xh_log_trace0("match string");
        expr_str = XH_CHAR_CAST RSTRING_PTR(expr);
        expr_len = RSTRING_LEN(expr);
        xh_log_trace2("expr: [%.*s]", (int) expr_len, expr_str);
        if (name_len == expr_len && !xh_strncmp(name, expr_str, name_len)) {
            xh_log_trace0("match TRUE");
            return TRUE;
        }
    }

    return FALSE;
}

static void
xh_x2h_parse_chunk(xh_x2h_ctx_t *ctx, xh_char_t **buf, size_t *bytesleft, xh_bool_t terminate)
{
    xh_char_t          c, *cur, *node, *end, *content, *eof, *enc,
                      *enc_cur, *old_cur, *old_eof, *content_key;
    unsigned int       depth, real_depth, code, flags;
    int                bits;
    VALUE             *lval, val;
    xh_x2h_node_t     *nodes;
    VALUE              av;
    size_t             enc_len, content_key_len;

    cur            = *buf;
    eof            = cur + *bytesleft;
    nodes          = ctx->nodes;
    depth          = ctx->depth;
    real_depth     = ctx->real_depth;
    flags          = ctx->flags;
    node           = ctx->node;
    end            = ctx->end;
    content        = ctx->content;
    code           = ctx->code;
    lval           = ctx->lval;
    enc            = enc_cur = old_eof = old_cur = NULL;
    c              = '\0';

    if (ctx->opts.content[0] == '\0') {
        content_key = (xh_char_t *) DEF_CONTENT_KEY;
        content_key_len = sizeof(DEF_CONTENT_KEY) - 1;
    }
    else {
        content_key = ctx->opts.content;
        content_key_len = xh_strlen(ctx->opts.content);
    }

#define XH_X2H_PROCESS_STATE(st) case st: goto st;
    switch (ctx->state) {
        case PARSER_ST_NONE: break;
        XH_X2H_PARSER_STATE_LIST
        case XML_DECL_FOUND: break;
        case PARSER_ST_DONE: goto DONE;
    }
#undef XH_X2H_PROCESS_STATE

PARSE_CONTENT:
    content = NULL;
    flags &= ~(XH_X2H_NEED_NORMALIZE | XH_X2H_IS_NOT_BLANK);
    DO(CONTENT)
        EXPECT_CHAR("new element", '<')
            if (content != NULL) {
                if (flags & XH_X2H_IS_NOT_BLANK) {
                    if (flags & XH_X2H_NEED_NORMALIZE) {
                        NORMALIZE_TEXT(TEXT1, content, end - content)
                        NEW_TEXT(enc, enc_len)
                    }
                    else {
                        NEW_TEXT(content, end - content)
                    }
                }
                content = NULL;
            }
            DO(PARSE_ELEMENT)
                EXPECT_CHAR("xml declaration", '?')
                    if (real_depth != 0) goto INVALID_XML;
#undef  NEW_ATTRIBUTE
#define NEW_ATTRIBUTE(k, kl, v, vl) NEW_XML_DECL_ATTRIBUTE(k, kl, v, vl)
#undef  SEARCH_ATTRIBUTE_VALUE
#define SEARCH_ATTRIBUTE_VALUE(loop, top_loop, quot) SEARCH_XML_DECL_ATTRIBUTE_VALUE(loop, top_loop, quot)
                    PARSE_XML_DECLARATION
#undef  NEW_ATTRIBUTE
#define NEW_ATTRIBUTE(k, kl, v, vl) NEW_NODE_ATTRIBUTE(k, kl, v, vl)
#undef  SEARCH_ATTRIBUTE_VALUE
#define SEARCH_ATTRIBUTE_VALUE(loop, top_loop, quot) SEARCH_NODE_ATTRIBUTE_VALUE(loop, top_loop, quot)
                EXPECT_CHAR("comment", '!')
                    DO(XML_COMMENT_NODE_OR_CDATA)
                        EXPECT_CHAR("comment", '-')
                            PARSE_COMMENT
                        EXPECT_CHAR("cdata", '[')
                            if (ctx->opts.trim) {
                                PARSE_CDATA_WITH_TRIM
                                ;
                            }
                            else {
                                PARSE_CDATA
                                ;
                            }
                        EXPECT_ANY("wrong character")
                            goto INVALID_XML;
                    END(XML_COMMENT_NODE_OR_CDATA)
                    goto INVALID_XML;
                EXPECT_CHAR("closing tag", '/')
                    //node = cur;
                    DO(PARSE_CLOSING_TAG)
                        EXPECT_CHAR("end tag name", '>')
                            CLOSE_TAG
                            goto PARSE_CONTENT;
                        EXPECT_BLANK("end tag name")
                            DO(SEARCH_CLOSING_END_TAG)
                                EXPECT_CHAR("end tag", '>')
                                    CLOSE_TAG
                                    goto PARSE_CONTENT;
                                SKIP_BLANK
                                EXPECT_ANY("wrong character")
                                    goto INVALID_XML;
                            END(SEARCH_CLOSING_END_TAG)
                            goto INVALID_XML;
                    END(PARSE_CLOSING_TAG)
                    goto INVALID_XML;
                EXPECT_ANY("opening tag")
                    node = cur - 1;
                    DO(PARSE_OPENING_TAG)
                        EXPECT_CHAR("end tag", '>')
                            OPEN_TAG(node, cur - node - 1)
                            goto PARSE_CONTENT;
                        EXPECT_CHAR("self closing tag", '/')
                            OPEN_TAG(node, cur - node - 1)
                            CLOSE_TAG

                            DO(SEARCH_OPENING_END_TAG)
                                EXPECT_CHAR("end tag", '>')
                                    goto PARSE_CONTENT;
                                EXPECT_ANY("wrong character")
                                    goto INVALID_XML;
                            END(SEARCH_OPENING_END_TAG)
                            goto INVALID_XML;
                        EXPECT_BLANK("end tag name")
                            OPEN_TAG(node, cur - node - 1)

                            SEARCH_ATTRIBUTES(NODE, SEARCH_END_TAG)

                            goto PARSE_CONTENT;
                    END(PARSE_OPENING_TAG);
                    goto INVALID_XML;
            END(PARSE_ELEMENT)

        EXPECT_CHAR("wrong symbol", '>')
            goto INVALID_XML;
        EXPECT_BLANK_WO_CR("blank")
            if (!ctx->opts.trim)
                goto START_CONTENT;
            break;
        EXPECT_CHAR("CR", '\r')
            if (content != NULL) {
                flags |= XH_X2H_NORMALIZE_LINE_FEED;
            }
            if (!ctx->opts.trim)
                goto START_CONTENT;
            break;
        EXPECT_CHAR("reference", '&')
            flags |= XH_X2H_NORMALIZE_REF;
        EXPECT_ANY("any char")
            flags |= XH_X2H_IS_NOT_BLANK;
            START_CONTENT:
            if (content == NULL) content = cur - 1;
            end = cur;
    END(CONTENT)

    if (content != NULL) {
        if (flags & XH_X2H_IS_NOT_BLANK) {
            if (flags & XH_X2H_NEED_NORMALIZE) {
                NORMALIZE_TEXT(TEXT2, content, end - content)
                NEW_TEXT(enc, enc_len)
            }
            else {
                NEW_TEXT(content, end - content)
            }
        }
        content = NULL;
    }

    if (real_depth != 0 || !(flags & XH_X2H_ROOT_FOUND)) goto INVALID_XML;

    ctx->state          = PARSER_ST_DONE;
    *bytesleft          = eof - cur;
    *buf                = cur;
    return;

XML_DECL_FOUND:
    ctx->state          = XML_DECL_FOUND;
CHUNK_FINISH:
    ctx->content        = content;
    ctx->node           = node;
    ctx->end            = end;
    ctx->depth          = depth;
    ctx->real_depth     = real_depth;
    ctx->flags          = flags;
    ctx->code           = code;
    ctx->lval           = lval;
    *bytesleft          = eof - cur;
    *buf                = cur;
    return;

MAX_DEPTH_EXCEEDED:
    rb_raise(xh_parse_error_class, "Maximum depth exceeded");
INVALID_XML:
    rb_raise(xh_parse_error_class, "Invalid XML");
INVALID_REF:
    rb_raise(xh_parse_error_class, "Invalid reference");
MALLOC:
    rb_raise(rb_eNoMemError, "Memory allocation error");
DONE:
    rb_raise(xh_parse_error_class, "Parsing is done");
}

static void
xh_x2h_parse(xh_x2h_ctx_t *ctx, xh_reader_t *reader)
{
    xh_char_t  *buf, *preserve;
    size_t     len, off;
    xh_bool_t  eof;

    do {
        preserve = ctx->node != NULL ? ctx->node : ctx->content;

        len = reader->read(reader, &buf, preserve, &off);
        eof = (len == 0);
        if (off) {
            if (ctx->node    != NULL) ctx->node    -= off;
            if (ctx->content != NULL) ctx->content -= off;
            if (ctx->end     != NULL) ctx->end     -= off;
        }

        xh_log_trace2("read buf: %.*s", (int) len, buf);

        do {
            xh_log_trace2("parse buf: %.*s", (int) len, buf);

            xh_x2h_parse_chunk(ctx, &buf, &len, eof);

            if (ctx->state == XML_DECL_FOUND && ctx->opts.encoding[0] == '\0' && ctx->encoding[0] != '\0') {
                reader->switch_encoding(reader, ctx->encoding, &buf, &len);
            }
        } while (len > 0);
    } while (!eof);

    if (ctx->state != PARSER_ST_DONE)
        rb_raise(xh_parse_error_class, "Invalid XML");
}

static VALUE
xh_x2h_exec(VALUE arg)
{
    xh_x2h_ctx_t *ctx = (xh_x2h_ctx_t *) arg;

    if (ctx->opts.filter.enable) {
        ctx->flags |= XH_X2H_FILTER_ENABLED;
        if (!ctx->opts.block_given)
            ctx->result = rb_ary_new();
    }
    else {
        ctx->result = hash_new();
        ctx->nodes[0].lval = ctx->lval = &ctx->result;
    }

    xh_reader_init(&ctx->reader, ctx->input, ctx->opts.encoding, ctx->opts.buf_size);

    xh_x2h_parse(ctx, &ctx->reader);

    return Qnil;
}

VALUE
xh_x2h(xh_x2h_ctx_t *ctx)
{
    VALUE result;
    int   state;

    result = rb_protect(xh_x2h_exec, (VALUE) ctx, &state);

    if (state) {
        xh_reader_destroy(&ctx->reader);
        rb_exc_raise(rb_errinfo());
    }

    xh_reader_destroy(&ctx->reader);

    result = ctx->result;
    if (ctx->opts.filter.enable) {
        if (ctx->opts.block_given)
            result = Qnil;
    }
    else if (!ctx->opts.keep_root) {
      result = hash_first_value(result);
    }

    return result;
}
