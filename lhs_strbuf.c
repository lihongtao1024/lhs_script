#include "lhs_strbuf.h"
#include "lhs_vm.h"

static int lhsbuf_grow(void* vm, LHSSTRBUF* buf, size_t nsize)
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

int lhsbuf_init(void* vm, LHSSTRBUF* buf)
{
    buf->usize = 0;
    buf->size = sizeof(buf->buf);
    buf->data = buf->buf;
    return LHS_TRUE;
}

int lhsbuf_reset(void* vm, LHSSTRBUF* buf)
{
    buf->usize = 0;
    return LHS_TRUE;
}

int lhsbuf_pushc(void* vm, LHSSTRBUF* buf, char c)
{
    if (buf->usize + sizeof(char) >= buf->size)
    {
        lhsbuf_grow(vm, buf, buf->size << 1);
    }

    buf->data[buf->usize++] = c;
    buf->data[buf->usize] = 0;
    return LHS_TRUE;
}

int lhsbuf_pushi(void* vm, LHSSTRBUF* buf, int i)
{
    return lhsbuf_pushls(vm, buf, (const char*)&i, sizeof(i));
}

int lhsbuf_pushl(void* vm, LHSSTRBUF* buf, long long l)
{
    return lhsbuf_pushls(vm, buf, (const char*)&l, sizeof(l));
}

int lhsbuf_pushf(void* vm, LHSSTRBUF* buf, double n)
{
    return lhsbuf_pushls(vm, buf, (const char*)&n, sizeof(n));
}

int lhsbuf_pushls(void* vm, LHSSTRBUF* buf, const char* str, size_t l)
{
    if (buf->usize + l + sizeof(char) >= buf->size)
    {
        lhsbuf_grow
        (
            vm, 
            buf, 
            ((buf->usize + l + sizeof(char)) & 
            ~(sizeof(buf->buf) - sizeof(char))) + 
            sizeof(buf->buf)
        );
    }

    memcpy(buf->data + buf->usize, str, l);

    buf->usize += l;
    buf->data[buf->usize] = 0;
    return LHS_TRUE;
}

void lhsbuf_uninit(void* vm, LHSSTRBUF* buf)
{
    if (buf->data != buf->buf)
    {
        lhsmem_freeobject(vm, buf->data, buf->size);
    }

    buf->size = 0;
    buf->usize = 0;
}
