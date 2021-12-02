#pragma once

#define lhsslink_init(s, m) \
(s)->m = 0

#define lhsslink_push(s, m, p, n) \
{\
    (p)->n = (s)->m;\
    (s)->m = (p);\
}

#define lhslink_pop(s, m, p, n) \
{\
    (s)->m = (p)->n;\
}

#define lhsslink_removeequal(t, s, m, n, a) \
{\
    t** l = &(s)->m;\
    for (; *l; )\
    {\
        t* e = *l;\
        if (e == (a))\
        {\
            *l = e->n;\
        }\
        else\
        {\
            l = &e->n;\
        }\
    }\
}

#define lhsslink_removeif(t, s, m, n, f) \
{\
    t** l = &(s)->m;\
    for (; *l; )\
    {\
        t* e = *l;\
        if (!(f) || (f)((s), e))\
        {\
            *l = e->n;\
        }\
        else\
        {\
            l = &e->n;\
        }\
    }\
}

#define lhsslink_freeif(t, s, m, n, f, c) \
{\
    t** l = &(s)->m;\
    for (; *l; )\
    {\
        t* e = *l;\
        if (!(f) || (f)((s), e))\
        {\
            *l = e->n;\
            (c)((s), e);\
        }\
        else\
        {\
            l = &e->n;\
        }\
    }\
}

#define lhsslink_foreach(t, s, m, n, c, u) \
{\
    t* l = s->m;\
    for (; l; )\
    {\
        t* e = l->n;\
        (c)((s), l, (u));\
        l = e;\
    }\
}

