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

int lhsbuf_topc(void* vm, LHSSTRBUF* buf, char* c)
{
    if (buf->usize < sizeof(char))
    {
        return LHS_FALSE;
    }

    *c = buf->data[buf->usize - sizeof(char)];
    return LHS_TRUE;
}

int lhsbuf_topcp(void* vm, LHSSTRBUF* buf, char** c)
{
    if (buf->usize < sizeof(char))
    {
        return LHS_FALSE;
    }

    *c = &buf->data[buf->usize - sizeof(char)];
    return LHS_TRUE;
}

int lhsbuf_popc(void* vm, LHSSTRBUF* buf, char* c)
{
    if (buf->usize < sizeof(char))
    {
        return LHS_FALSE;
    }

    buf->usize -= sizeof(char);
    *c = buf->data[buf->usize];
    buf->data[buf->usize] = 0;
    return LHS_TRUE;
}

int lhsbuf_topi(void* vm, LHSSTRBUF* buf, int* i)
{
    if (buf->usize < sizeof(int))
    {
        return LHS_FALSE;
    }

    *i = *(int*)&buf->data[buf->usize - sizeof(int)];
    return LHS_TRUE;
}

int lhsbuf_topip(void* vm, LHSSTRBUF* buf, int** i)
{
    if (buf->usize < sizeof(int))
    {
        return LHS_FALSE;
    }

    *i = (int*)&buf->data[buf->usize - sizeof(int)];
    return LHS_TRUE;
}

int lhsbuf_popi(void* vm, LHSSTRBUF* buf, int* i)
{
    if (buf->usize < sizeof(int))
    {
        return LHS_FALSE;
    }

    buf->usize -= sizeof(int);
    *i = *(int*)&buf->data[buf->usize];
    buf->data[buf->usize] = 0;
    return LHS_TRUE;
}

int lhsbuf_topl(void* vm, LHSSTRBUF* buf, long long* l)
{
    if (buf->usize < sizeof(long long))
    {
        return LHS_FALSE;
    }

    *l = *(long long*)&buf->data[buf->usize - sizeof(long long)];
    return LHS_TRUE;
}

int lhsbuf_toplp(void* vm, LHSSTRBUF* buf, long long** l)
{
    if (buf->usize < sizeof(long long))
    {
        return LHS_FALSE;
    }

    *l = (long long*)&buf->data[buf->usize - sizeof(long long)];
    return LHS_TRUE;
}

int lhsbuf_popl(void* vm, LHSSTRBUF* buf, long long* l)
{
    if (buf->usize < sizeof(long long))
    {
        return LHS_FALSE;
    }

    buf->usize -= sizeof(long long);
    *l = *(long long*)&buf->data[buf->usize];
    buf->data[buf->usize] = 0;
    return LHS_TRUE;
}

int lhsbuf_topf(void* vm, LHSSTRBUF* buf, double* n)
{
    if (buf->usize < sizeof(double))
    {
        return LHS_FALSE;
    }

    *n = *(double *)&buf->data[buf->usize - sizeof(double)];
    return LHS_TRUE;
}

int lhsbuf_topfp(void* vm, LHSSTRBUF* buf, double** n)
{
    if (buf->usize < sizeof(double))
    {
        return LHS_FALSE;
    }

    *n = (double *)&buf->data[buf->usize - sizeof(double)];
    return LHS_TRUE;
}

int lhsbuf_popf(void* vm, LHSSTRBUF* buf, double* n)
{
    if (buf->usize < sizeof(double))
    {
        return LHS_FALSE;
    }

    buf->usize -= sizeof(double);
    *n = *(double *)&buf->data[buf->usize];
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
