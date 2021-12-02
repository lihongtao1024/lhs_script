#include "lhs_strbuf.h"

static int lhsbuf_grow(LHSVM* vm, LHSSTRBUF* buf, size_t nsize)
{
    if (buf->data == buf->buf)
    {
        buf->data = lhsmem_newobject(vm, nsize);
        memcpy(buf->data, buf->buf, buf->usize);        
    }
    else
    {
        buf->data = lhsmem_renewobject(vm, buf->data, buf->size, nsize);
    }
    buf->size = nsize;
    return LHS_TRUE;
}

int lhsbuf_init(LHSVM* vm, LHSSTRBUF* buf)
{
    buf->usize = 0;
    buf->size = sizeof(buf->buf);
    buf->data = buf->buf;
    return LHS_TRUE;
}

int lhsbuf_reset(LHSVM* vm, LHSSTRBUF* buf)
{
    buf->usize = 0;
    return LHS_TRUE;
}

int lhsbuf_pushchar(LHSVM* vm, LHSSTRBUF* buf, char c)
{
    if (buf->usize + 1 >= buf->size)
    {
        lhsbuf_grow(vm, buf, buf->size << 1);
    }

    buf->data[buf->usize++] = c;
    buf->data[buf->usize] = 0;
    return LHS_TRUE;
}

int lhsbuf_pushlstr(LHSVM* vm, LHSSTRBUF* buf, const char* str, size_t l)
{
    if (buf->usize + l + 1 >= buf->size)
    {
        lhsbuf_grow
        (
            vm, 
            buf, 
            ((buf->usize + l + 1) & ~(sizeof(buf->buf) - 1)) + sizeof(buf->buf)
        );
    }

    memcpy(buf->data + buf->usize, str, l);

    buf->usize += l;
    buf->data[buf->usize] = 0;
    return LHS_TRUE;
}

int lhsbuf_topchar(LHSVM* vm, LHSSTRBUF* buf, char* c)
{
    if (buf->usize <= 0)
    {
        return LHS_FALSE;
    }

    *c = buf->data[buf->usize - 1];
    return LHS_TRUE;
}

int lhsbuf_popchar(LHSVM* vm, LHSSTRBUF* buf, char* c)
{
    if (buf->usize <= 0)
    {
        return LHS_FALSE;
    }

    *c = buf->data[--buf->usize];
    return LHS_TRUE;
}

int lhsbuf_isshort(LHSVM* vm, LHSSTRBUF* buf)
{
    return buf->usize < LHS_SHORTSTRLEN;
}

int lhsbuf_isempty(LHSVM* vm, LHSSTRBUF* buf)
{
    return !buf->usize;
}

void lhsbuf_uninit(LHSVM* vm, LHSSTRBUF* buf)
{
    if (buf->data != buf->buf)
    {
        lhsmem_freeobject(vm, buf->data, buf->size);
    }

    buf->size = 0;
    buf->usize = 0;
}
