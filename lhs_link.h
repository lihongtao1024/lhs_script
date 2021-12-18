#pragma once

#define lhslink_init(s, m)                  \
(s)->m = 0

#define lhslink_forward(s, m, p, n)         \
{                                           \
    (p)->n = (s)->m;                        \
    (s)->m = (p);                           \
}

#define lhslink_back(s, m, p, n)            \
{                                           \
    (s)->m = (p)->n;                        \
}

#define lhslink_removeif(t, s, m, n, f)     \
{                                           \
    t** l = &(s)->m;                        \
    for (; *l; )                            \
    {                                       \
        t* e = *l;                          \
        if (!(f) || (f)((s), e))            \
        {                                   \
            *l = e->n;                      \
        }                                   \
        else                                \
        {                                   \
            l = &e->n;                      \
        }                                   \
    }                                       \
}

#define lhslink_foreach(t, s, m, n, c, u)   \
{                                           \
    t* l = s->m;                            \
    for (; l; )                             \
    {                                       \
        t* e = l->n;                        \
        (c)((s), l, (u));                   \
        l = e;                              \
    }                                       \
}

