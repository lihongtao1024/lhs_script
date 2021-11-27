#include "lhs_strbuf.h"
#include "lhs_assert.h"

static int lhsbuf_grow(LHSVM* vm, LHSSTRBUF* buf, size_t nsize)
{
    lhsassert_trueresult(vm && buf, false);

    if (buf->data == buf->buf)
    {
        buf->data = lhsmem_newobject(vm, nsize);
        lhsassert_trueresult(buf->data, false);
        memcpy(buf->data, buf->buf, buf->usize);        
    }
    else
    {
        buf->data = lhsmem_renewobject(vm, buf->data, buf->size, nsize);
        lhsassert_trueresult(buf->data, false);
    }
    buf->size = nsize;
    return true;
}

int lhsbuf_init(LHSVM* vm, LHSSTRBUF* buf)
{
    lhsassert_trueresult(vm && buf, false);

    buf->usize = 0;
    buf->size = sizeof(buf->buf);
    buf->data = buf->buf;
    return true;
}

int lhsbuf_reset(LHSVM* vm, LHSSTRBUF* buf)
{
    lhsassert_trueresult(vm && buf, false);

    buf->usize = 0;
    return true;
}

int lhsbuf_pushchar(LHSVM* vm, LHSSTRBUF* buf, char c)
{
    lhsassert_trueresult(vm && buf, false);

    if (buf->usize + 1 >= buf->size)
    {
        lhsbuf_grow(vm, buf, buf->size << 1);
    }

    buf->data[buf->usize++] = c;
    buf->data[buf->usize] = 0;
    return true;
}

int lhsbuf_topchar(LHSVM* vm, LHSSTRBUF* buf, char* c)
{
    lhsassert_trueresult(vm && buf && c, false);

    if (buf->usize <= 0)
    {
        return false;
    }

    *c = buf->data[buf->usize - 1];
    return true;
}

int lhsbuf_popchar(LHSVM* vm, LHSSTRBUF* buf, char* c)
{
    lhsassert_trueresult(vm && buf && c, false);

    if (buf->usize <= 0)
    {
        return false;
    }

    *c = buf->data[--buf->usize];
    return true;
}

int lhsbuf_isshort(LHSVM* vm, LHSSTRBUF* buf)
{
    lhsassert_trueresult(vm && buf, false);

    return buf->usize < LHS_SHORTSTRLEN;
}

int lhsbuf_isempty(LHSVM* vm, LHSSTRBUF* buf)
{
    lhsassert_trueresult(vm && buf, false);

    return !buf->usize;
}

void lhsbuf_uninit(LHSVM* vm, LHSSTRBUF* buf)
{
    lhsassert_truereturn(vm && buf);

    if (buf->data != buf->buf)
    {
        lhsmem_freeobject(vm, buf->data, buf->size);
    }

    buf->size = 0;
    buf->usize = 0;
}
