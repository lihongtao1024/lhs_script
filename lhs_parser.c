#include "lhs_parser.h"
#include "lhs_load.h"
#include "lhs_value.h"
#include "lhs_link.h"
#include "lhs_variable.h"
#include "lhs_code.h"
#include "lhs_error.h"

#define G                                     (1)
#define L                                     (0)
#define N                                     (-1)
#define E                                     (2)
#define LHS_EXPRIMMED                         (16)
#define LHS_EXPRREF                           (32)
#define LHS_EXPRNONE                          (0)
#define LHS_EXPRINT                           (1 | LHS_EXPRIMMED)
#define LHS_EXPRNUM                           (2 | LHS_EXPRIMMED)
#define LHS_EXPRBOOLEAN                       (3 | LHS_EXPRIMMED)
#define LHS_EXPRSTR                           (4 | LHS_EXPRREF)
#define LHS_EXPRRAW(t)                        ((t) & (LHS_EXPRIMMED - 1) - 1)

#define lhsparser_issymbol(t)                                                       \
(((t) & ~UCHAR_MAX) ?                                                               \
 ((t) > LHS_TOKENSYMBOLBEGIN &&                                                     \
  (t) < LHS_TOKENSYMBOLEND) :                                                       \
(lhsloadf_symbol[(t)] > SYMBOL_BEGIN &&                                             \
 lhsloadf_symbol[(t)] < SYMBOL_END))                                                

#define lhsparser_istokensymbol(lf)                                                 \
lhsparser_issymbol(lhsparser_castlex(lf)->token.t)                                  

#define lhsparser_isunarysymbol(s)                                                  \
((s) == SYMBOL_NOT || (s) == SYMBOL_NOTB || (s) == SYMBOL_MINUS)                    

#define lhsparser_islbracket(lf)                                                    \
(lhsparser_castlex(lf)->token.t == '(')                                             

#define lhsparser_truncate(t)                                                       \
(((t) & ~UCHAR_MAX) ?                                                               \
 ((t) & UCHAR_MAX) :                                                                \
 lhsloadf_symbol[t])                                                                

#define lhsparser_exprcondiandcheck(vm, lf, n, s)                                   \
{                                                                                   \
    int nstack = lhsparser_castlex(lf)->nstack;                                     \
    lhsparser_exprstate((vm), (lf));                                                \
    if (lhsparser_castlex(lf)->nstack - nstack != (n))                              \
    {                                                                               \
        lhserr_syntax((vm), (lf), s);                                               \
    }                                                                               \
}

#define lhsparser_isyield(lf, n)                                                    \
(lhsparser_castlex(lf)->token.t == LHS_TOKENEOF || ((n) && lhsparser_castlex(lf)->token.t == '}'))                                    

#define lhsparser_chunkforward(vm, lf)                                              \
{                                                                                   \
    LHSChunk* chunk = lhsmem_newobject((vm), sizeof(LHSChunk));                     \
    lhsparser_resetchunk((vm), (lf), chunk);                                        \
    lhslink_forward(lhsparser_castlex(lf), curchunk, chunk, parent);                \
    lhslink_forward(lhsparser_castlex(lf), allchunk, chunk, next);                  \
}

#define lhsparser_chunkback(vm, lf)                                                 \
lhslink_back(lhsparser_castlex(lf), curchunk, lhsparser_castlex(lf)->curchunk, parent)                                     

#define lhsparser_regionforward(vm, lf, t)                                          \
{                                                                                   \
    LHSRegion* region = lhsmem_newobject((vm), sizeof(LHSRegion));                  \
    lhsparser_resetregion((vm), (lf), region, (t));                                 \
    lhslink_forward(lhsparser_castlex(lf), curregion, region, parent);              \
    lhslink_forward(lhsparser_castlex(lf), allregion, region, next);                \
}

#define lhsparser_regionback(vm, lf)                                                \
{                                                                                   \
    LHSRegion* region = lhsparser_castlex(lf)->curregion;                           \
    LHSJmp* jmp = region->breakjmp;                                                 \
    for (int i = 0; i < region->nbreak; i++)                                        \
    {                                                                               \
        jmp->len = (int)(lhsparser_castlex(lf)->curfunction->code.usize - jmp->pos);\
        lhsparser_jmpsolve(lhsparser_castlex(lf), jmp, vm);                         \
        jmp = jmp->next;                                                            \
    }                                                                               \
    lhslink_back(lhsparser_castlex(lf), curregion, lhsparser_castlex(lf)->allregion, parent);\
}

#define lhsparser_op1(vm, lf, s, c)                                                 \
lhscode_op1((vm), &lhsparser_castlex(lf)->curfunction->code, (s), (c));             \
lhsparser_checkexprstack((vm), (lf), (s), 0)

#define lhsparser_opcall(vm, lf, l, c, n, a, w)                                      \
lhscode_op2((vm), &lhsparser_castlex(lf)->curfunction->code, OP_CALL, (l), (c), (n));\
lhsparser_checkexprstack((vm), (lf), OP_CALL, (a) - (w))

#define lhsparser_op2(vm, lf, s, l, c, n)                                           \
lhscode_op2((vm), &lhsparser_castlex(lf)->curfunction->code, (s), (l), (c), (n));   \
lhsparser_checkexprstack((vm), (lf), (s), 0)

#define lhsparser_boolean(vm, lf, v)                                                \
lhscode_boolean((vm), &lhsparser_castlex(lf)->curfunction->code, (v))

#define lhsparser_integer(vm, lf, v)                                                \
lhscode_integer(vm, &lhsparser_castlex(lf)->curfunction->code, (v))

#define lhsparser_number(vm, lf, v)                                                 \
lhscode_number(vm, &lhsparser_castlex(lf)->curfunction->code, (v))

#define lhsparser_ref(vm, lf, m, v)                                                 \
lhscode_ref(vm, &lhsparser_castlex(lf)->curfunction->code, (m), (v))

#define lhsparser_index(vm, lf, v)                                                  \
lhscode_index(vm, &lhsparser_castlex(lf)->curfunction->code, (v))

#define lhsparser_byte(vm, lf, v)                                                  \
lhscode_byte(vm, &lhsparser_castlex(lf)->curfunction->code, (v))

typedef struct LHSExprUnary
{
    char unary;
    struct LHSExprUnary* next;
} LHSExprUnary;

typedef struct LHSExprChain
{
    char type;
    char symbol;
    char swap;
    int line;
    int column;
    int refer;
    int nunary;
    union
    {
        struct
        {
            char mark;
            int index;
        } ref;
        char b;
        double n;
        long long i;        
    } factor;
    LHSExprUnary* unary;
    struct LHSExprChain* prev;
} LHSExprChain;

typedef struct LHSExprState
{
    LHSExprUnary* unary;
    LHSExprChain* chain;
} LHSExprState;

typedef struct LHSIfState
{
    LHSJmp* branch;
    LHSJmp* finish;
    struct LHSIfState* prev;
} LHSIfState;

typedef struct LHSForState
{
    LHSJmp* finish;
    LHSJmp* block;
    LHSJmp* condition;
    LHSJmp* iterate;
} LHSForState;

typedef struct LHSWhileState
{
    LHSJmp* finish;
    LHSJmp* condition;
} LHSWhileState;

typedef struct LHSDoState
{
    LHSJmp* block;
} LHSDoState;

typedef int (*lhsparser_expropr)(LHSVM* vm, LHSLoadF*, LHSExprChain*);
static int lhsparser_initmainfunction(LHSVM* vm, LHSLoadF* loadf, const char* fname);
static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_exprstate(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_uninitexprstate(LHSVM* vm, LHSExprState* state);
static int lhsparser_statement(LHSVM* vm, LHSLoadF* loadf, int nested);
static int lhsparser_exprsub(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state);

const char* lhsparser_symbols[] =
{
    "n/a", "+", "-", "*", "/", "%", 
    "&", "|", "^", "<", ">", "==",
    "!=", ">=", "<=", "&&", "||", 
    "<<", ">>", "-", "!", "~", "="
};

static const char* reserveds[] =
{
    "", "set", "var", "function", "for", 
    "while", "if", "else", "do", "until", 
    "break", "continue", "true", "false", 
    "return"
};

static const char priorities[][SYMBOL_END] =
{
    { E, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N}, 
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
    { L, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, N}
};

static const char* lhsparser_anonymous(LHSVM* vm)
{
    static char buf[64]; static int index;
    sprintf(buf, "1_anonymous_%x", index++);
    return buf;
}

static int lhsparser_initreserved(LHSVM* vm)
{
    for (int i = 0; i < _countof(reserveds); ++i)
    {
        lhsvm_insertshortstr
        (
            vm, 
            reserveds[i], 
            strlen(reserveds[i]), 
            i + LHS_TOKENRESERVEDBEGIN
        );
    }

    return LHS_TRUE;
}

static LHSVar* lhsparser_insertconstant(LHSVM* vm, LHSLoadF* loadf)
{
    const LHSValue* key = lhsvm_getvalue(vm, -1);

    LHSVarDesc fdesc;
    fdesc.chunk = 0;
    fdesc.name = lhsvalue_caststring(key->gc);

    LHSVarDesc* odesc = lhshash_find(vm, &vm->conststrhash, &fdesc);
    if (odesc)
    {
        lhsvm_pop(vm, 1);
        return lhsvector_at(vm, &vm->conststrs, odesc->index);
    }

    LHSVarDesc* desc = lhsvar_castvardesc
    (
        lhsmem_newgcobject(vm, sizeof(LHSVarDesc), LHS_TGCFULLDATA)
    );
    desc->chunk = fdesc.chunk;
    desc->name = fdesc.name;
    lhshash_insert(vm, &vm->conststrhash, desc, 0);

    LHSVar* var = lhsvector_increment(vm, &vm->conststrs);
    memcpy(&var->value, key, sizeof(LHSValue));
    var->desc = desc;

    desc->line = loadf->line;
    desc->column = loadf->column;
    desc->index = (int)vm->conststrs.usize - 1;
    desc->mark = LHS_MARKSTRING;

    lhsvm_pop(vm, 1);
    return var;
}

static LHSVarDesc* lhsparser_insertlocalvar(LHSVM* vm, LHSLoadF* loadf)
{    
    const LHSValue* key = lhsvm_getvalue(vm, -1);
    LHSVarDesc* desc = lhsvar_castvardesc
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSVarDesc), 
            LHS_TGCFULLDATA
        )
    );
    desc->chunk = lhsparser_castlex(loadf)->curchunk->index;
    desc->name = lhsvalue_caststring(key->gc);
    lhshash_insert(vm, &lhsparser_castlex(loadf)->curfunction->localvars, desc, 0);

    desc->line = loadf->line;
    desc->column = loadf->column;
    desc->index = ++lhsparser_castlex(loadf)->curfunction->nlocalvars;
    desc->mark = LHS_MARKLOCAL;
    
    lhsvm_pop(vm, 1);
    return desc;
}

static LHSVar* lhsparser_insertglobalvar(LHSVM* vm, LHSLoadF* loadf)
{
    const LHSValue* key = lhsvm_getvalue(vm, -1);
    LHSVarDesc* desc = lhsvar_castvardesc
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSVarDesc), 
            LHS_TGCFULLDATA
        )
    );
    desc->chunk = 0;
    desc->name = lhsvalue_caststring(key->gc);
    lhshash_insert(vm, &vm->globalvars, desc, 0);

    LHSVar *var = lhsvector_increment(vm, &vm->globalvalues);
    var->value.type = LHS_TNONE;
    var->desc = desc;

    desc->line = loadf->line;
    desc->column = loadf->column;    
    desc->index = (int)vm->globalvalues.usize - 1;
    desc->mark = LHS_MARKGLOBAL;

    lhsvm_pop(vm, 1);
    return var;
}

static const LHSVarDesc* lhsparser_recursionfindvar(LHSVM* vm, LHSLoadF* loadf, 
    LHSValue** value)
{
    const LHSValue* key = lhsvm_getvalue(vm, -1);

    LHSVarDesc fdesc;
    fdesc.chunk = lhsparser_castlex(loadf)->curchunk->index;
    fdesc.name = lhsvalue_caststring(key->gc);

    const LHSVarDesc* odesc = lhshash_find
    (
        vm, 
        &lhsparser_castlex(loadf)->curfunction->localvars, 
        &fdesc
    );
    if (!odesc)
    {
        for (LHSChunk* chunk = lhsparser_castlex(loadf)->curchunk->parent; 
             chunk; 
             chunk = chunk->parent)
        {
            fdesc.chunk = chunk->index;
            odesc = lhshash_find
            (
                vm, 
                &lhsparser_castlex(loadf)->curfunction->localvars, 
                &fdesc
            );
            if (odesc)
            {
                break;
            }
        }

        if (!odesc)
        {
            fdesc.chunk = 0;
            odesc = lhshash_find(vm, &vm->globalvars, &fdesc);
        }
    }

    lhsvm_pop(vm, 1);

    if (!odesc)
    {
        return 0;
    }

    if (value &&
        odesc->mark == LHS_MARKGLOBAL)
    {
        *value = lhsvector_at(vm, &vm->globalvalues, odesc->index);
    }
    return odesc;
}

static int lhsparser_initmainframe(LHSVM* vm, LHSLoadF* loadf, 
    const char* fname)
{
    LHSFrame *frame = lhsframe_castframe
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSFrame), 
            LHS_TGCFRAME
        )
    );
    lhsframe_init(vm, frame);
    lhsparser_castlex(loadf)->curframe = frame;

    lhsvm_pushstring(vm, fname);
    LHSVar* var = lhsparser_insertconstant(vm, loadf);
    frame->name = var->desc->index;

    vm->mainframe = frame;
    lhsparser_initmainfunction(vm, loadf, fname);
    return LHS_TRUE;
}

static int lhsparser_initmainfunction(LHSVM* vm, LHSLoadF* loadf, 
    const char* fname)
{
    LHSFrame* frame = lhsparser_castlex(loadf)->curframe;
    LHSFunction *function = lhsfunction_castfunc
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSFunction), 
            LHS_TGCFUNCTION
        )
    );
    lhsfunction_init(vm, function);
    lhsparser_castlex(loadf)->curfunction = function;

    lhsvm_pushstring(vm, fname);
    LHSVar* var = lhsparser_insertconstant(vm, loadf);
    function->name = var->desc->index;
    function->frame = frame;

    frame->mainfunc = function;
    lhslink_forward(frame, allfunc, function, next);
    return LHS_TRUE;
}

static int lhsparser_enterfunction(LHSVM* vm, LHSLoadF* loadf)
{
    LHSFrame* frame = lhsparser_castlex(loadf)->curframe;
    LHSFunction* function = lhsfunction_castfunc
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSFunction), 
            LHS_TGCFUNCTION
        )
    );

    lhsfunction_init(vm, function);
    lhsparser_castlex(loadf)->curfunction = function;

    lhsvm_pushvalue(vm, -1);

    LHSValue* value;
    const LHSVarDesc* desc = lhsparser_recursionfindvar(vm, loadf, &value);
    if (desc)
    {
        (desc->mark != LHS_MARKGLOBAL) &&
            lhserr_syntax
            (
                vm, 
                loadf, 
                "variable duplicate definition '%s'.",
                desc->name->data
            );

        lhsvar_castvardesc(desc)->line = loadf->line;
        lhsvar_castvardesc(desc)->column = loadf->column;

        value->type = LHS_TGC;
        value->gc = lhsgc_castgc(function);
    }
    else
    {
        lhsvm_pushvalue(vm, -1);
        LHSVar* var = lhsparser_insertglobalvar(vm, loadf);
        var->value.type = LHS_TGC;
        var->value.gc = lhsgc_castgc(function);
    }

    LHSVar* var = lhsparser_insertconstant(vm, loadf);
    function->name = var->desc->index;
    function->frame = lhsparser_castlex(loadf)->curframe;

    lhslink_forward(frame, allfunc, function, next);
    return LHS_TRUE;
}

static int lhsparser_leavefunction(LHSVM* vm, LHSLoadF* loadf)
{
    LHSFrame* frame = lhsparser_castlex(loadf)->curframe;
    lhsparser_castlex(loadf)->curfunction = frame->mainfunc;
    return LHS_TRUE;
}

static int lhsparser_initjmp(LHSVM* vm, LHSLoadF* loadf, LHSJmp* jmp)
{
    jmp->pos = 0;
    jmp->len = 0;
    jmp->func = lhsparser_castlex(loadf)->curfunction;
    lhslink_init(jmp, next);
    return LHS_TRUE;
}

static int lhsparser_uninitjmp(LHSLexical* lex, LHSJmp* jmp, LHSVM *vm)
{
    lhsmem_freeobject(vm, jmp, sizeof(LHSJmp));
    return LHS_TRUE;
}

static int lhsparser_resetchunk(LHSVM* vm, LHSLoadF* loadf, LHSChunk* chunk)
{
    chunk->index = ++lhsparser_castlex(loadf)->chunkid;
    lhslink_init(chunk, next);
    lhslink_init(chunk, parent);
    return LHS_TRUE;
}

static int lhsparser_uninitchunk(LHSLexical* lex, LHSChunk* chunk, LHSVM* vm)
{
    lhsmem_freeobject(vm, chunk, sizeof(LHSChunk));
    return LHS_TRUE;
}

static int lhsparser_uninitregion(LHSLexical* lex, LHSRegion* region, LHSVM* vm)
{
    lhsmem_freeobject(vm, region, sizeof(LHSRegion));
    return LHS_TRUE;
}

static int lhsparser_resetregion(LHSVM* vm, LHSLoadF* loadf, LHSRegion* region,
    int token)
{
    region->token = token;
    region->nbreak = 0;
    lhslink_init(region, breakjmp);
    lhslink_init(region, next);
    lhslink_init(region, parent);
    return LHS_TRUE;
}

static int lhsparser_initlexical(LHSVM* vm, LHSLoadF* loadf, LHSLexical* lex)
{
    lhsparser_castlex(loadf) = lex;

    lex->nstack = 0;
    lex->chunkid = 0;    
    lex->curfunction = 0;
    lex->curframe = 0;
    lex->token.t = LHS_TOKENEOF;
    lex->lookahead.t = LHS_TOKENEOF;

    lhslink_init(lex, alljmp);
    lhslink_init(lex, curchunk);
    lhslink_init(lex, allchunk);
    lhsparser_chunkforward(vm, loadf);

    lhslink_init(lex, curregion);
    lhslink_init(lex, allregion);

    lhsbuf_init(vm, &lex->token.buf);
    lhsbuf_init(vm, &lex->lookahead.buf);
    return LHS_TRUE;
}

static int lhsparser_jmpsolve(LHSLexical* lex, LHSJmp* jmp, LHSVM* vm)
{
    int* addr = (int*)(jmp->func->code.data + jmp->pos - sizeof(int));
    *addr = (int)(jmp->pos + jmp->len);
    return LHS_TRUE;
}

static int lhsparser_lexicalsolve(LHSVM* vm, LHSLexical* lex)
{
    lhslink_foreach(LHSJmp, lex, alljmp, next, lhsparser_jmpsolve, vm);
    return LHS_TRUE;
}

static int lhsparser_uninitlexical(LHSVM* vm, LHSLoadF* loadf)
{
    lhslink_foreach
    (
        LHSJmp, 
        lhsparser_castlex(loadf), 
        alljmp, 
        next, 
        lhsparser_uninitjmp, 
        vm
    );
    lhslink_foreach
    (
        LHSChunk,
        lhsparser_castlex(loadf),
        allchunk,
        next,
        lhsparser_uninitchunk,
        vm
    );
    lhslink_foreach
    (
        LHSRegion, 
        lhsparser_castlex(loadf), 
        allregion, 
        next, 
        lhsparser_uninitregion, 
        vm
    );
    lhsbuf_uninit(vm, &lhsparser_castlex(loadf)->token.buf);
    lhsbuf_uninit(vm, &lhsparser_castlex(loadf)->lookahead.buf);
    return LHS_TRUE;
}

static int lhsparser_nextlexical(LHSVM* vm, LHSLoadF* loadf, LHSBuf* buf)
{
    for (; ; )
    {
        switch (loadf->current)
        {
        case '\f':
        case '\t':        
        case '\v':
        case '\r':
        case ' ':
        {
            lhsloadf_getc(loadf);
            break;
        }
        case '\n':
        {
            lhsloadf_getc(loadf);
            lhsloadf_newline(loadf);
            break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            int is_double = LHS_FALSE;
            lhsloadf_savedigital(vm, loadf, &is_double, buf);
            return is_double ? LHS_TOKENNUMBER : LHS_TOKENINTEGER;
        }
        case '/':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (loadf->current == '/')
            {
                lhsloadf_skipline(vm, loadf);
                break;
            }
            else if (loadf->current == '*')
            {
                lhsloadf_skipcomment(vm, loadf);
                break;
            }
            return '/';
        }
        case '&':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (loadf->current == '&')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENLOGICAND;
            }
            return '&';
        }
        case '|':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (loadf->current == '|')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENLOGICOR;
            }
            return '|';
        }
        case '<':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (loadf->current == '<')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENBLSHIFT;
            }
            else if (loadf->current == '=')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENLESSEQUAL;
            }
            return '<';
        }
        case '>':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (loadf->current == '>')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENBRSHIFT;
            }
            else if (loadf->current == '=')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENGREATEQUAL;
            }
            return '>';
        }
        case '=':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENEQUAL;
            }
            return '=';
        }
        case '!':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_addsymbol(vm, loadf, buf);
                lhsloadf_getc(loadf);
                return LHS_TOKENNOTEQUAL;
            }
            return '!';
        }
        case '-':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            lhsloadf_getc(loadf);
            if (lhsparser_istokensymbol(loadf) ||
                lhsparser_islbracket(loadf))
            {
                return LHS_TOKENMINUS;
            }
            return '-';
        }
        case '+':
        case '*':
        case '%':
        case '~':
        case '^':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
        {
            lhsloadf_savesymbol(vm, loadf, buf);
            int token = loadf->current;
            lhsloadf_getc(loadf);
            return token;
        }
        case '"':
        {
            lhsloadf_savestring(vm, loadf, buf);
            return LHS_TOKENSTRING;
        }
        default:
        {
            if (lhsloadf_isletter(loadf))
            {
                lhsloadf_saveidentifier(vm, loadf, buf);
                if (buf->usize < LHS_SHORTSTRLEN)
                {
                    const LHSString* str = lhsvm_insertshortstr
                    (
                        vm, 
                        buf->data, 
                        buf->usize, 
                        0
                    );
                    return str->reserved ? str->reserved : LHS_TOKENIDENTIFIER;
                }
                return LHS_TOKENIDENTIFIER;
            }

            if (loadf->current == LHS_TOKENEOF)
            {
                return LHS_TOKENEOF;
            }

            lhserr_syntax
            (
                vm, 
                loadf,
                "undefined token '%s'.",
                lhsparser_castlex(loadf)->token.buf.data
            );
        }
        }
    }
}

static int lhsparser_nexttoken(LHSVM* vm, LHSLoadF* loadf)
{
    if (lhsparser_castlex(loadf)->lookahead.t != LHS_TOKENEOF)
    {
        lhsparser_castlex(loadf)->token.t = lhsparser_castlex(loadf)->lookahead.t;
        lhsparser_castlex(loadf)->lookahead.t = LHS_TOKENEOF;
        lhsbuf_reset(vm, &lhsparser_castlex(loadf)->token.buf);
        lhsbuf_pushls
        (
            vm, 
            &lhsparser_castlex(loadf)->token.buf, 
            lhsparser_castlex(loadf)->lookahead.buf.data, 
            lhsparser_castlex(loadf)->lookahead.buf.usize
        );
        return LHS_TRUE;
    }

    lhsparser_castlex(loadf)->token.t = lhsparser_nextlexical
    (
        vm, 
        loadf, 
        &lhsparser_castlex(loadf)->token.buf
    );
    return LHS_TRUE;
}

static int lhsparser_checktoken(LHSVM* vm, LHSLoadF* loadf,
    int token, const char* prefix, const char* name)
{
    if (lhsparser_castlex(loadf)->token.t != token)
    {
        (lhsparser_castlex(loadf)->token.t == LHS_TOKENEOF) &&
            lhserr_syntax
            (
                vm,
                loadf,
                "<eof> unexpected end of solving %s.",
                prefix,
                lhsparser_castlex(loadf)->token.buf.data
            )
            ||
            lhserr_syntax
            (
                vm,
                loadf,
                "unexpected %s solving, expected '%s', got '%s'.",
                prefix,
                name,
                lhsparser_castlex(loadf)->token.buf.data
            );
    }
    return LHS_TRUE;
}

static int lhsparser_checklookahead(LHSVM* vm, LHSLoadF* loadf,
    int token, const char* prefix, const char* name)
{
    if (lhsparser_castlex(loadf)->lookahead.t != token)
    {
        (lhsparser_castlex(loadf)->lookahead.t == LHS_TOKENEOF) &&
            lhserr_syntax
            (
                vm, 
                loadf, 
                "<eof> unexpected end of solving %s.",
                prefix,
                lhsparser_castlex(loadf)->lookahead.buf.data
            )
            ||
            lhserr_syntax
            (
                vm, 
                loadf, 
                "unexpected %s solving, expected '%s', got '%s'.",
                prefix,
                name,
                lhsparser_castlex(loadf)->lookahead.buf.data
            );
    }
    return LHS_TRUE;
}

static int lhsparser_checkandnexttoken(LHSVM* vm, LHSLoadF* loadf, 
    int token, const char *prefix, const char* name)
{
    lhsparser_checktoken(vm, loadf, token, prefix, name);
    lhsparser_nexttoken(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_nexttokenandcheck(LHSVM* vm, LHSLoadF* loadf, 
    int token, const char *prefix, const char* name)
{
    lhsparser_nexttoken(vm, loadf);
    lhsparser_checktoken(vm, loadf, token, prefix, name); 
    return LHS_TRUE;
}

static int lhsparser_lookaheadtoken(LHSVM* vm, LHSLoadF* loadf)
{
    lhsparser_checklookahead(vm, loadf, LHS_TOKENEOF, "lookahead", "<eof>");
    lhsparser_castlex(loadf)->lookahead.t = lhsparser_nextlexical
    (
        vm, 
        loadf, 
        &lhsparser_castlex(loadf)->lookahead.buf
    );
    return LHS_TRUE;
}

static int lhsparser_checkexprstack(LHSVM* vm, LHSLoadF* loadf, char op, int call)
{
    switch (op)
    {
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
    case OP_ANDB:
    case OP_ORB:
    case OP_XORB:
    case OP_L:
    case OP_G:
    case OP_E:
    case OP_NE:
    case OP_GE:
    case OP_LE:
    case OP_AND:
    case OP_OR:
    case OP_SHL:
    case OP_SHR:
    case OP_MOVS:
    case OP_POP:
    case OP_JZ:
    case OP_JNZ:
    {
        lhsparser_castlex(loadf)->nstack--;
        break;
    }    
    case OP_PUSH:
    case OP_RET:
    case OP_RET1:
    {
        lhsparser_castlex(loadf)->nstack++;
        break;
    }
    case OP_CALL:
    {
        lhsparser_castlex(loadf)->nstack -= call;
        break;
    }
    default:
    {
        break;
    }
    }

    (lhsparser_castlex(loadf)->nstack < 0) &&
        lhserr_syntax
        (
            vm, 
            loadf, 
            "illegal token '%s'.", 
            lhsparser_castlex(loadf)->token.buf
        );
    return LHS_TRUE;
}

static int lhsparser_resetexprchain(LHSVM* vm, LHSExprChain* chain, LHSExprState* state)
{
    lhslink_forward(state, chain, chain, prev);

    chain->symbol = SYMBOL_NONE;
    chain->swap = LHS_FALSE;
    chain->type = LHS_EXPRNONE;
    chain->line = 0;
    chain->column = 0;
    chain->refer = 0;
    chain->nunary = 0;
    lhslink_init(chain, unary);
    return LHS_TRUE;
}

static int lhsparser_initexprstate(LHSVM* vm, LHSExprState* state)
{
    state->chain = 0;
    lhslink_init(state, unary);
    return LHS_TRUE;
}

static int lhsparser_uninitexprunary(LHSExprState* state, LHSExprUnary* unary, LHSVM* vm)
{
    lhsmem_freeobject(vm, unary, sizeof(LHSExprUnary));
    return LHS_TRUE;
}

static int lhsparser_uninitexprstate(LHSVM* vm, LHSExprState* state)
{
    lhslink_foreach(LHSExprUnary, state, unary, next, lhsparser_uninitexprunary, vm);
    return LHS_TRUE;
}

static int lhsparser_resetifstate(LHSVM* vm, LHSLoadF* loadf, LHSIfState* state)
{
    state->branch = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->branch);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->branch, next);

    state->finish = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->finish);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->finish, next);

    lhslink_init(state, prev);
    return LHS_TRUE;
}

static int lhsparser_resetforstate(LHSVM* vm, LHSLoadF* loadf, LHSForState* state)
{
    state->finish = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->finish);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->finish, next);

    state->block = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->block);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->block, next);

    state->condition = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->condition);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->condition, next);

    state->iterate = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->iterate);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->iterate, next);

    return LHS_TRUE;
}

static int lhsparser_resetwhilestate(LHSVM* vm, LHSLoadF* loadf, LHSWhileState* state)
{
    state->finish = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->finish);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->finish, next);

    state->condition = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->condition);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->condition, next);

    return LHS_TRUE;
}

static int lhsparser_resetdostate(LHSVM* vm, LHSLoadF* loadf, LHSDoState* state)
{
    state->block = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, state->block);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, state->block, next);

    return LHS_TRUE;
}

static lhsparser_exprunary(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    for (int i = 0; i < chain->nunary; ++i)
    {
        switch (chain->unary->unary)
        {
        case SYMBOL_MINUS:
        {
            switch (chain->type)
            {
            case LHS_EXPRINT:
            {
                chain->factor.i = 0 - chain->factor.i;
                break;
            }
            case LHS_EXPRNUM:
            {
                chain->factor.n = 0 - chain->factor.n;
                break;
            }
            default:
            {
                goto illegalunary;
            }
            }
            break;
        }
        case SYMBOL_NOT:
        {
            if (chain->type == LHS_EXPRBOOLEAN)
            {
                chain->factor.b = !chain->factor.b;
            }
            else
            {
                goto illegalunary;
            }
            break;
        }
        case SYMBOL_NOTB:
        {
            if (chain->type == LHS_EXPRINT)
            {
                chain->factor.i = ~chain->factor.i;
                break;
            }
            else
            {
                goto illegalunary;
            }
            break;
        }
        default:
        {
            goto illegalunary;
        }
        }
        chain->unary = chain->unary->next;
    }

    chain->nunary = 0;
    return LHS_TRUE;

illegalunary:
    lhserr_syntax(vm, loadf, "illegal unary symbol '%s'.", 
        lhsparser_symbols[chain->symbol]);
    return LHS_FALSE;
}

static lhsparser_exproprll(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    LHSExprChain* prev = chain->prev;
    lhsparser_exprunary(vm, loadf, prev);
    lhsparser_exprunary(vm, loadf, chain);

    switch (prev->symbol)
    {
    case SYMBOL_ADD:
    {
        chain->factor.i = prev->factor.i + chain->factor.i;
        break;
    }
    case SYMBOL_SUB:
    {
        chain->factor.i = prev->factor.i - chain->factor.i;
        break;
    }
    case SYMBOL_MUL:
    {
        chain->factor.i = prev->factor.i * chain->factor.i;
        break;
    }
    case SYMBOL_DIV:
    {
        if (prev->factor.i % chain->factor.i)
        {
            chain->type = LHS_EXPRNUM;
            chain->factor.n = (double)prev->factor.i / chain->factor.i;
        }
        else
        {
            chain->factor.i = prev->factor.i / chain->factor.i;
        }
        break;
    }
    case SYMBOL_MOD:
    {
        chain->factor.i = prev->factor.i % chain->factor.i;
        break;
    }
    case SYMBOL_ANDB:
    {
        chain->factor.i = prev->factor.i & chain->factor.i;
        break;
    }
    case SYMBOL_ORB:
    {
        chain->factor.i = prev->factor.i | chain->factor.i;
        break;
    }
    case SYMBOL_XORB:
    {
        chain->factor.i = prev->factor.i ^ chain->factor.i;
        break;
    }
    case SYMBOL_LESS:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i < chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREAT:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i > chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_EQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i == chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_NOTEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i != chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREATEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i >= chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_LESSEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i <= chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_BLSHIFT:
    {
        chain->factor.i = prev->factor.i << chain->factor.i;
        break;
    }
    case SYMBOL_BRSHIFT:
    {
        chain->factor.i = prev->factor.i >> chain->factor.i;
        break;
    }
    default:
    {
        lhserr_syntax(vm, loadf, "illegal expression symbol '%s'.", 
            lhsparser_symbols[prev->symbol]);
    }
    }

    return LHS_TRUE;
}

static lhsparser_exproprln(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    LHSExprChain* prev = chain->prev;
    lhsparser_exprunary(vm, loadf, prev);
    lhsparser_exprunary(vm, loadf, chain);

    switch (prev->symbol)
    {
    case SYMBOL_ADD:
    {
        chain->factor.n = prev->factor.i + chain->factor.n;
        break;
    }
    case SYMBOL_SUB:
    {
        chain->factor.n = prev->factor.i - chain->factor.n;
        break;
    }
    case SYMBOL_MUL:
    {
        chain->factor.n = prev->factor.i * chain->factor.n;
        break;
    }
    case SYMBOL_DIV:
    {
        chain->factor.n = prev->factor.i / chain->factor.n;
        break;
    }
    case SYMBOL_LESS:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i < chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREAT:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i > chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_EQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i == chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_NOTEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i != chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREATEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i >= chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_LESSEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.i <= chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_syntax(vm, loadf, "illegal expression symbol '%s'.", 
            lhsparser_symbols[prev->symbol]);
    }
    }

    return LHS_TRUE;
}

static lhsparser_exproprnl(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    LHSExprChain* prev = chain->prev;
    lhsparser_exprunary(vm, loadf, prev);
    lhsparser_exprunary(vm, loadf, chain);

    switch (prev->symbol)
    {
    case SYMBOL_ADD:
    {
        chain->type = LHS_EXPRNUM;
        chain->factor.n = prev->factor.n + chain->factor.i;
        break;
    }
    case SYMBOL_SUB:
    {
        chain->type = LHS_EXPRNUM;
        chain->factor.n = prev->factor.n - chain->factor.i;
        break;
    }
    case SYMBOL_MUL:
    {
        chain->type = LHS_EXPRNUM;
        chain->factor.n = prev->factor.n * chain->factor.i;
        break;
    }
    case SYMBOL_DIV:
    {
        chain->type = LHS_EXPRNUM;
        chain->factor.n = prev->factor.n / chain->factor.i;
        break;
    }
    case SYMBOL_LESS:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n < chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREAT:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n > chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_EQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n == chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_NOTEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n != chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREATEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n >= chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_LESSEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n <= chain->factor.i) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_syntax(vm, loadf, "illegal expression symbol '%s'.", 
            lhsparser_symbols[prev->symbol]);
    }
    }

    return LHS_TRUE;
}

static lhsparser_exproprnn(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    LHSExprChain* prev = chain->prev;
    lhsparser_exprunary(vm, loadf, prev);
    lhsparser_exprunary(vm, loadf, chain);

    switch (prev->symbol)
    {
    case SYMBOL_ADD:
    {
        chain->factor.n = prev->factor.n + chain->factor.n;
        break;
    }
    case SYMBOL_SUB:
    {
        chain->factor.n = prev->factor.n - chain->factor.n;
        break;
    }
    case SYMBOL_MUL:
    {
        chain->factor.n = prev->factor.n * chain->factor.n;
        break;
    }
    case SYMBOL_DIV:
    {
        chain->factor.n = prev->factor.n / chain->factor.n;
        break;
    }
    case SYMBOL_LESS:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n < chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREAT:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n > chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_EQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n == chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_NOTEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n != chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_GREATEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n >= chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_LESSEQUAL:
    {
        chain->type = LHS_EXPRBOOLEAN;
        chain->factor.b = (prev->factor.n <= chain->factor.n) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_syntax(vm, loadf, "illegal expression symbol '%s'.", 
            lhsparser_symbols[prev->symbol]);
    }
    }

    return LHS_TRUE;
}

static lhsparser_exproprbb(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    LHSExprChain* prev = chain->prev;
    lhsparser_exprunary(vm, loadf, prev);
    lhsparser_exprunary(vm, loadf, chain);

    switch (prev->symbol)
    {
    case SYMBOL_EQUAL:
    {
        chain->factor.b = (prev->factor.b == chain->factor.b) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_NOTEQUAL:
    {
        chain->factor.b = (prev->factor.b != chain->factor.b) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_LOGICAND:
    {
        chain->factor.b = (prev->factor.b && chain->factor.b) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    case SYMBOL_LOGICOR:
    {
        chain->factor.b = (prev->factor.b || chain->factor.b) ?
            LHS_TRUE : LHS_FALSE;
        break;
    }
    default:
    {
        lhserr_syntax(vm, loadf, "illegal expression symbol '%s'.", 
            lhsparser_symbols[prev->symbol]);
    }
    }

    return LHS_TRUE;
}

static lhsparser_exproprerr(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    lhserr_syntax(vm, loadf, "illegal expression symbol '%s'.", 
        lhsparser_symbols[chain->prev->symbol]);
}

static lhsparser_expropr lhsparser_exproperations[][3] =
{
    {lhsparser_exproprll, lhsparser_exproprln, lhsparser_exproprerr},
    {lhsparser_exproprnl, lhsparser_exproprnn, lhsparser_exproprerr},
    {lhsparser_exproprerr,lhsparser_exproprerr,lhsparser_exproprbb}
};

static int lhsparser_exprcode(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    switch (chain->type)
    {
    case LHS_EXPRINT:
    {
        lhsparser_op1(vm, loadf, OP_PUSH, chain);
        lhsparser_integer(vm, loadf, chain->factor.i);
        break;
    }
    case LHS_EXPRNUM:
    {
        lhsparser_op1(vm, loadf, OP_PUSH, chain);
        lhsparser_number(vm, loadf, chain->factor.n);
        break;
    }
    case LHS_EXPRBOOLEAN:
    {
        lhsparser_op1(vm, loadf, OP_PUSH, chain);
        lhsparser_boolean(vm, loadf, chain->factor.b);
        break;
    }
    case LHS_EXPRSTR:
    case LHS_EXPRREF:
    {
        lhsparser_op1(vm, loadf, OP_PUSH, chain);
        lhsparser_ref(vm, loadf, chain->factor.ref.mark, chain->factor.ref.index);
        break;
    }
    default:
    {
        break;
    }
    }

    for (int i = 0; i < chain->nunary; ++i)
    {
        lhsparser_op1(vm, loadf, chain->unary->unary, chain);
        chain->unary = chain->unary->next;
    }
    chain->nunary = 0;

    if (chain->swap)
    {
        lhsparser_op1(vm, loadf, OP_SWAP, chain);
    }
    chain->swap = LHS_FALSE;
    return LHS_TRUE;
}

static int lhsparser_exprmov(LHSVM* vm, LHSLoadF* loadf, LHSExprChain* chain)
{
    if (chain->nunary ||
        chain->type & LHS_EXPRIMMED)
    {
        lhsparser_exprcode(vm, loadf, chain);
    }

    lhsparser_op1(vm, loadf, chain->type & LHS_EXPRREF ? OP_MOV : OP_MOVS, chain);
    lhsparser_ref(vm, loadf, chain->prev->factor.ref.mark, chain->prev->factor.ref.index);

    if (chain->type & LHS_EXPRREF)
    {
        lhsparser_ref(vm, loadf, chain->factor.ref.mark, chain->factor.ref.index);
    }
    return LHS_TRUE;
}

static int lhsparser_exprsolve(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    LHSExprChain* chain = state->chain;
    if (!chain)
    {
        return LHS_FALSE;
    }

    LHSExprChain* prev = chain->prev;
    switch (priorities[prev->symbol][chain->symbol])
    {
    case G:
    {
        break;
    }
    case L:
    {
        if (prev->type & LHS_EXPRIMMED &&
            chain->type & LHS_EXPRIMMED)
        {
            lhsparser_exproperations
                [LHS_EXPRRAW(prev->type)]
                [LHS_EXPRRAW(chain->type)]
                (vm, loadf, chain);
        }
        else
        {
            if (chain->prev->symbol == SYMBOL_ASSIGN)
            {
                lhsparser_exprmov(vm, loadf, chain);
            }
            else
            {
                lhsparser_exprcode(vm, loadf, chain->prev);
                lhsparser_exprcode(vm, loadf, chain);
                lhsparser_op1(vm, loadf, chain->prev->symbol, chain);
            }

            chain->prev->type = LHS_EXPRNONE;
            chain->type = LHS_EXPRNONE;
        }

        chain->prev = prev->prev;
        return lhsparser_exprsolve(vm, loadf, state);
    }
    case E:
    {
        lhsparser_exprcode(vm, loadf, chain);
        state->chain = prev->prev;
        return lhsparser_exprsolve(vm, loadf, state);
    }
    default:
    {
        lhserr_syntax
        (
            vm,
            loadf,
            "unexpected expression symbol '%s'.",
            lhsparser_castlex(loadf)->token.t
        );
    }
    }

    return LHS_TRUE;
}

static int lhsparser_exprargs(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    /*exprargs -> null | {exprstate [',']}*/
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (token->t == ')')
    {
        return 0;
    }

    int narg = 0;
    while (LHS_TRUE)
    {
        narg++;
        lhsparser_exprstate(vm, loadf);
        if (token->t != ',')
        {
            break;
        }
        lhsparser_nexttoken(vm, loadf);
    }
    
    return narg;
}

static int lhsparser_exprcall(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    /*exprcall -> LHS_TOKENIDENTIFIER '(' [exprargs] ')'*/
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENIDENTIFIER, "function", "<identifier>");

    lhsvm_pushvalue(vm, -1);
    lhsvm_pushvalue(vm, -1);
    LHSVar* name = lhsparser_insertconstant(vm, loadf);
    const LHSVarDesc* desc = lhsparser_recursionfindvar(vm, loadf, 0);
    if (!desc)
    {
        desc = lhsparser_insertglobalvar(vm, loadf)->desc;
    }
    else
    {
        lhsvm_pop(vm, 1);
        (desc->mark != LHS_MARKGLOBAL) &&
            lhserr_syntax
            (
                vm, 
                loadf, 
                "function '%s' was defined as local variable.",
                desc->name->data
            );
    }

    lhsparser_checkandnexttoken(vm, loadf, '(', "function", "(");

    char mark = desc->mark, nwant = LHS_VOID;
    int index = desc->index, refer = name->desc->index;
    int narg = lhsparser_exprargs(vm, loadf, state);
    desc = 0;

    if (state)
    {
        nwant = LHS_RESULT;
        state->chain->type = LHS_EXPRNONE;
        state->chain->line = loadf->line;
        state->chain->column = loadf->column;
        state->chain->refer = refer;
        state->chain->factor.ref.mark = mark;
        state->chain->factor.ref.index = index;

        for (LHSExprChain* prev = state->chain->prev;
             prev;
             prev = prev->prev)
        {
            if (prev->type == LHS_EXPRNONE)
            {
                continue;
            }
            prev->swap = LHS_TRUE;
        }
    }

    lhsparser_opcall(vm, loadf, loadf->line, loadf->column, refer, narg, nwant);
    lhsparser_ref(vm, loadf, mark, index);
    lhsparser_index(vm, loadf, narg);
    lhsparser_byte(vm, loadf, nwant);

    lhsparser_checkandnexttoken(vm, loadf, ')', "function", ")");
    return LHS_TRUE;
}

static int lhsparser_exprfactor(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    /*exprfactor -> [{op_unary}] LHS_TOKENINTEGER       | 
                    [{op_unary}] LHS_TOKENNUMBER        | 
                    [{op_unary}] LHS_TOKENTRUE          | 
                    [{op_unary}] LHS_TOKENFALSE         | 
                    [{op_unary}] LHS_TOKENIDENTIFIER    |
                    [{op_unary}] exprcall               |
                    [{op_unary}] '('exprstate')'*/
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    switch (token->t)
    {
    case LHS_TOKENINTEGER:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        LHSVar* var = lhsparser_insertconstant(vm, loadf);
        state->chain->type = LHS_EXPRINT;
        state->chain->line = loadf->line;
        state->chain->column = loadf->column;
        state->chain->refer = var->desc->index;
        state->chain->factor.i = atoll(token->buf.data);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENNUMBER:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        LHSVar* var = lhsparser_insertconstant(vm, loadf);
        state->chain->type = LHS_EXPRNUM;
        state->chain->line = loadf->line;
        state->chain->column = loadf->column;
        state->chain->refer = var->desc->index;
        state->chain->factor.n = atof(token->buf.data);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENSTRING:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        LHSVar* var = lhsparser_insertconstant(vm, loadf);
        state->chain->type = LHS_EXPRSTR;
        state->chain->line = loadf->line;
        state->chain->column = loadf->column;
        state->chain->refer = var->desc->index;
        state->chain->factor.ref.mark = var->desc->mark;
        state->chain->factor.ref.index = var->desc->index;
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENTRUE:
    case LHS_TOKENFALSE:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        LHSVar* var = lhsparser_insertconstant(vm, loadf);
        state->chain->type = LHS_EXPRBOOLEAN;
        state->chain->line = loadf->line;
        state->chain->column = loadf->column;
        state->chain->refer = var->desc->index;
        state->chain->factor.b = (token->t == LHS_TOKENTRUE ? LHS_TRUE : LHS_FALSE);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENIDENTIFIER:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        LHSToken* lookahead = &lhsparser_castlex(loadf)->lookahead;
        if (lookahead->t == LHS_TOKENEOF)
        {
            lhsparser_lookaheadtoken(vm, loadf);
        }

        if (lookahead->t == '(')
        {
            lhsparser_exprcall(vm, loadf, state);            
        }
        else
        {
            lhsvm_pushvalue(vm, -1);
            LHSVar* name = lhsparser_insertconstant(vm, loadf);
            const LHSVarDesc* desc = lhsparser_recursionfindvar(vm, loadf, 0);
            desc || lhserr_syntax
            (
                vm, 
                loadf, 
                "undefined variable '%s'.",
                token->buf.data
            );

            state->chain->type = LHS_EXPRREF;
            state->chain->line = loadf->line;
            state->chain->column = loadf->column;
            state->chain->refer = name->desc->index;
            state->chain->factor.ref.mark = desc->mark;
            state->chain->factor.ref.index = desc->index;
            lhsparser_nexttoken(vm, loadf);
        }
        break;
    }
    case '(':
    {
        state->chain->type = LHS_EXPRNONE;
        for (LHSExprChain* prev = state->chain->prev; 
             prev; 
             prev = prev->prev)
        {
            if (prev->type == LHS_EXPRNONE)
            {
                continue;
            }
            prev->swap = LHS_TRUE;
        }

        lhsparser_nexttoken(vm, loadf);
        lhsparser_exprstate(vm, loadf);
        lhsparser_checkandnexttoken(vm, loadf, ')', "expression", ")");
        break;
    }
    default:
    {
        char op = lhsparser_truncate(token->t);
        if (lhsparser_isunarysymbol(op))
        {
            lhsparser_nexttoken(vm, loadf);

            LHSExprUnary* unary = lhsmem_newobject(vm, sizeof(LHSExprUnary));
            unary->unary = op;
            lhslink_init(unary, next);
            lhslink_forward(state, unary, unary, next);

            state->chain->unary = unary;
            state->chain->nunary++;
            
            lhsparser_exprfactor(vm, loadf, state);
            break;
        }

        (token->t == LHS_TOKENEOF) &&
            lhserr_syntax
            (
                vm, 
                loadf, 
                "<eof> unexpected end of expression.",
                lhsparser_castlex(loadf)->token.buf.data
            )
            ||
            lhserr_syntax
            (
                vm, 
                loadf, 
                "unexpected expression factor, got '%s'.",
                lhsparser_castlex(loadf)->token.buf.data
            );
    }
    }

    return LHS_TRUE;
}

static int lhsparser_exprchain(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    /*exprchain -> exprfactor op_binary*/
    lhsparser_exprfactor(vm, loadf, state);
    
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    (lhsparser_isunarysymbol(token->t)) &&
        lhserr_syntax
        (
            vm, 
            loadf, 
            "unexpected unary symbol, got '%s'.",
            token->buf.data
        );

    if (!lhsparser_issymbol(token->t))
    {
        state->chain->symbol = SYMBOL_NONE;
    }
    else
    {
        state->chain->symbol = lhsparser_truncate(token->t);
        lhsparser_nexttoken(vm, loadf); 
    }

    return LHS_TRUE;
}

static int lhsparser_exprsub(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    /*subexpr -> {exprchain}*/
    LHSExprChain chain;
    lhsparser_resetexprchain(vm, &chain, state);
    lhsparser_exprchain(vm, loadf, state);

    if (lhsparser_exprsolve(vm, loadf, state))
    {
        return lhsparser_exprsub(vm, loadf, state);
    }

    return LHS_TRUE;
}

static int lhsparser_exprstate(LHSVM* vm, LHSLoadF* loadf)
{ 
    /*exprstate -> {subexpr}*/
    LHSExprState state;
    lhsparser_initexprstate(vm, &state);

    LHSExprChain chain;
    lhsparser_resetexprchain(vm, &chain, &state);

    if (lhserr_protectedcallex(vm, lhsparser_exprsub, loadf, &state))
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
        lhsparser_uninitexprstate(vm, &state);
        lhserr_throw(vm, "");
    }

    lhsparser_uninitexprstate(vm, &state);
    return LHS_TRUE;
}

static int lhsparser_callstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*callstate -> exprcall*/
    lhsparser_checktoken(vm, loadf, LHS_TOKENIDENTIFIER, "call", "call");

    LHSToken* token = &lhsparser_castlex(loadf)->token;
    lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
    lhsparser_exprcall(vm, loadf, 0);
    return LHS_TRUE;
}

static int lhsparser_localstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*var -> var identifier ['=' exprstate] {',' identifier ['=' exprstate]}*/
    lhsparser_checktoken(vm, loadf, LHS_TOKENLOCAL, "var", "var");
    while (LHS_TRUE)
    {
        lhsparser_nexttokenandcheck
        (
            vm, 
            loadf, 
            LHS_TOKENIDENTIFIER, 
            "var", 
            "<identifier>"
        );
        lhsvm_pushlstring
        (
            vm, 
            lhsparser_castlex(loadf)->token.buf.data, 
            lhsparser_castlex(loadf)->token.buf.usize
        );

        lhsvm_pushvalue(vm, -1);
        if (lhsparser_recursionfindvar(vm, loadf, 0))
        {
            lhsvm_pop(vm, 1);
            lhserr_syntax
            (
                vm, 
                loadf,
                "variable duplicate definition '%s'.",
                lhsparser_castlex(loadf)->token.buf.data
            );
        }

        lhsparser_insertlocalvar(vm, loadf);

        lhsparser_lookaheadtoken(vm, loadf);
        if (lhsparser_castlex(loadf)->lookahead.t == ',')
        {
            lhsparser_nexttoken(vm, loadf);
            continue;
        }

        if (lhsparser_castlex(loadf)->lookahead.t != '=')
        {
            lhsparser_nexttoken(vm, loadf);
            return LHS_TRUE;
        }

        lhsparser_exprstate(vm, loadf);
        if (lhsparser_castlex(loadf)->token.t == ',')
        {
            continue;
        }

        break;
    }

    return LHS_TRUE;
}

static int lhsparser_globalstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*set -> set identifier ['=' exprstate] {',' identifier ['=' exprstate]}*/
    lhsparser_checktoken(vm, loadf, LHS_TOKENGLOBAL, "set", "set");
    while (LHS_TRUE)
    {
        lhsparser_nexttokenandcheck
        (
            vm, 
            loadf, 
            LHS_TOKENIDENTIFIER, 
            "set", 
            "<identifier>"
        );
        lhsvm_pushlstring
        (
            vm, 
            lhsparser_castlex(loadf)->token.buf.data, 
            lhsparser_castlex(loadf)->token.buf.usize
        );

        lhsvm_pushvalue(vm, -1);
        if (lhsparser_recursionfindvar(vm, loadf, 0))
        {
            lhsvm_pop(vm, 1);
            lhserr_syntax
            (
                vm, 
                loadf,
                "variable duplicate definition '%s'.",
                lhsparser_castlex(loadf)->token.buf.data
            );
        }

        lhsparser_insertglobalvar(vm, loadf);

        lhsparser_lookaheadtoken(vm, loadf);
        if (lhsparser_castlex(loadf)->lookahead.t == ',')
        {
            lhsparser_nexttoken(vm, loadf);
            continue;
        }

        if (lhsparser_castlex(loadf)->lookahead.t != '=')
        {
            lhsparser_nexttoken(vm, loadf);
            return LHS_TRUE;
        }

        lhsparser_exprstate(vm, loadf);
        if (lhsparser_castlex(loadf)->token.t == ',')
        {
            continue;
        }

        break;
    }

    return LHS_TRUE;
}

static int lhsparser_blockstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*blockstate -> '{' '}' | '{' [statement] '}'*/
    lhsparser_checkandnexttoken(vm, loadf, '{', "block", "{");
    if (lhsparser_castlex(loadf)->token.t == '}')
    {
        lhsparser_nexttoken(vm, loadf);
        return LHS_TRUE;
    }

    lhsparser_chunkforward(vm, loadf);
    lhsparser_statement(vm, loadf, LHS_TRUE);
    lhsparser_chunkback(vm, loadf);

    lhsparser_checkandnexttoken(vm, loadf, '}', "block", "}");
    return LHS_TRUE;
}

int lhsparser_iftrue(LHSVM* vm, LHSLoadF* loadf, LHSIfState* state)
{
    /*iftrue -> blockstate*/
    lhsparser_op2(vm, loadf, OP_JZ, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state->branch->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    
    lhsparser_blockstate(vm, loadf);

    lhsparser_op2(vm, loadf, OP_JMP, loadf->line, loadf->column,
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state->finish->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    return LHS_TRUE;
}

int lhsparser_iffalse(LHSVM* vm, LHSLoadF* loadf, LHSIfState* state)
{
    /*iffalse -> blockstate*/
    state->branch->len = (int)
    (
        lhsparser_castlex(loadf)->curfunction->code.usize - state->branch->pos
    );

    lhsparser_blockstate(vm, loadf);

    for (LHSIfState* current = state; current; current = current->prev)
    {
        current->finish->len = (int)
        (
            lhsparser_castlex(loadf)->curfunction->code.usize - current->finish->pos
        );
    }
    return LHS_TRUE;
}

int lhsparser_ifnfalse(LHSVM* vm, LHSLoadF* loadf, LHSIfState* state)
{
    state->branch->len = (int)
    (
        lhsparser_castlex(loadf)->curfunction->code.usize - state->branch->pos
    );
    return LHS_TRUE;
}

static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*ifstate -> if '(' exprstate ')' ifstrue [{ifstrue}] [iffalse]*/
    LHSIfState mainstate;
    lhsparser_resetifstate(vm, loadf, &mainstate);
    LHSIfState* state = &mainstate;

    do
    {
        lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENIF, "if", "if");
        lhsparser_checkandnexttoken(vm, loadf, '(', "if", "(");
        lhsparser_exprcondiandcheck(vm, loadf, 1, "illegal if condition.");
        lhsparser_checkandnexttoken(vm, loadf, ')', "if", ")");
        lhsparser_iftrue(vm, loadf, state);

        LHSToken* token = &lhsparser_castlex(loadf)->token,
            * lookahead = &lhsparser_castlex(loadf)->lookahead;

        if (token->t != LHS_TOKENELSE)
        {
            lhsparser_ifnfalse(vm, loadf, state);
            break;
        }

        lhsparser_lookaheadtoken(vm, loadf);
        if (lookahead->t != LHS_TOKENIF)
        {
            lhsparser_nexttoken(vm, loadf);
            lhsparser_iffalse(vm, loadf, state);
            break;
        }

        lhsparser_nexttoken(vm, loadf);
        state->branch->len = (int)
        (
            lhsparser_castlex(loadf)->curfunction->code.usize - state->branch->pos
        );

        LHSIfState slavestate;
        lhsparser_resetifstate(vm, loadf, &slavestate);
        slavestate.prev = state;
        state = &slavestate;
    } while (LHS_TRUE);

    return LHS_TRUE;
}

static int lhsparser_forarg1(LHSVM* vm, LHSLoadF* loadf, LHSForState* state)
{
    /*forarg1 -> null | localstate | exprstate*/
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    switch (token->t)
    {
    case LHS_TOKENLOCAL:
    {
        lhsparser_localstate(vm, loadf);
        break;
    }
    case ';':
    {
        break;
    }
    default:
    {
        lhsparser_exprstate(vm, loadf);
        break;
    }
    }

    return LHS_TRUE;
}

static int lhsparser_forarg2(LHSVM* vm, LHSLoadF* loadf, LHSForState* state)
{
    /*forarg2 -> null | exprstate*/
    state->condition->pos = lhsparser_castlex(loadf)->curfunction->code.usize;

    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (token->t != ';')
    {
        lhsparser_exprcondiandcheck(vm, loadf, 1, "illegal for condition.");
    }
    else
    {
        lhsparser_op2(vm, loadf, OP_PUSH, loadf->line, loadf->column,
            lhsparser_castlex(loadf)->curfunction->name);
        lhsparser_boolean(vm, loadf, LHS_TRUE);
    }

    lhsparser_op2(vm, loadf, OP_JZ, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state->finish->pos = lhsparser_castlex(loadf)->curfunction->code.usize;

    lhsparser_op2(vm, loadf, OP_JMP, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state->block->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    return LHS_TRUE;
}

static int lhsparser_forarg3(LHSVM* vm, LHSLoadF* loadf, LHSForState* state)
{
    /*forarg3 -> null | exprstate*/
    state->iterate->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    lhsparser_castlex(loadf)->curregion->continuejmp.pos = 
        lhsparser_castlex(loadf)->curfunction->code.usize;

    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (token->t != ')')
    {
        lhsparser_exprcondiandcheck(vm, loadf, 0, "illegal for next.");
    }
    
    lhsparser_op2(vm, loadf, OP_JMP, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state->condition->len = (int)(state->condition->pos - 
        lhsparser_castlex(loadf)->curfunction->code.usize);
    state->condition->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    return LHS_TRUE;
}

static int lhsparser_forargs(LHSVM* vm, LHSLoadF* loadf, LHSForState* state)
{
    /*forargs -> [forarg1] ';' [forarg2] ';' [forarg3]*/
    lhsparser_forarg1(vm, loadf, state);
    lhsparser_checkandnexttoken(vm, loadf, ';', "for", "for");
    lhsparser_forarg2(vm, loadf, state);
    lhsparser_checkandnexttoken(vm, loadf, ';', "for", "for");
    lhsparser_forarg3(vm, loadf, state);    
    return LHS_TRUE;
}

static int lhsparser_forstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*forstate -> for '(' forargs ')' blockstate*/
    LHSForState state;
    lhsparser_resetforstate(vm, loadf, &state);

    lhsparser_regionforward(vm, loadf, LHS_TOKENFOR);
    lhsparser_chunkforward(vm, loadf);

    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENFOR, "for", "for");
    lhsparser_checkandnexttoken(vm, loadf, '(', "for", "(");
    lhsparser_forargs(vm, loadf, &state);
    lhsparser_checkandnexttoken(vm, loadf, ')', "for", ")");

    state.block->len = (int)
    (
        lhsparser_castlex(loadf)->curfunction->code.usize - state.block->pos
    );

    lhsparser_blockstate(vm, loadf);

    lhsparser_op2(vm, loadf, OP_JMP, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state.finish->len = (int)
    (
        lhsparser_castlex(loadf)->curfunction->code.usize - state.finish->pos
    );
    state.iterate->len = (int)
    (
        state.iterate->pos - lhsparser_castlex(loadf)->curfunction->code.usize
    );
    state.iterate->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    
    lhsparser_chunkback(vm, loadf);
    lhsparser_regionback(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_whilearg(LHSVM* vm, LHSLoadF* loadf, LHSWhileState* state)
{
	/*whilearg -> exprstate*/
    state->condition->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    lhsparser_castlex(loadf)->curregion->continuejmp.pos = 
        lhsparser_castlex(loadf)->curfunction->code.usize;

    lhsparser_exprcondiandcheck(vm, loadf, 1, "illegal while condition.");

    lhsparser_op2(vm, loadf, OP_JZ, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state->finish->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    return LHS_TRUE;
}

static int lhsparser_whilestate(LHSVM* vm, LHSLoadF* loadf)
{
    /*whilestate -> while '(' whilearg ')' blockstate*/
    LHSWhileState state;
    lhsparser_resetwhilestate(vm, loadf, &state);

    lhsparser_regionforward(vm, loadf, LHS_TOKENWHILE);

    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENWHILE, "while", "while");
    lhsparser_checkandnexttoken(vm, loadf, '(', "while", "(");
    lhsparser_whilearg(vm, loadf, &state);
    lhsparser_checkandnexttoken(vm, loadf, ')', "while", ")");

    lhsparser_blockstate(vm, loadf);

    lhsparser_op2(vm, loadf, OP_JMP, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state.finish->len = (int)
    (
        lhsparser_castlex(loadf)->curfunction->code.usize - state.finish->pos
    );
    state.condition->len = (int)
    (
        state.condition->pos - lhsparser_castlex(loadf)->curfunction->code.usize
    );
    state.condition->pos = lhsparser_castlex(loadf)->curfunction->code.usize;

    lhsparser_regionback(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_doarg(LHSVM* vm, LHSLoadF* loadf, LHSDoState* state)
{
    /*doarg -> exprstate*/
    LHSRegion* region = lhsparser_castlex(loadf)->curregion;
    region->continuejmp.jmp->len = (int)
    (
        lhsparser_castlex(loadf)->curfunction->code.usize - region->continuejmp.jmp->pos
    );

    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENUNTIL, "until", "until");
    lhsparser_checkandnexttoken(vm, loadf, '(', "until", "(");
    lhsparser_exprcondiandcheck(vm, loadf, 1, "illegal until condition.");
    lhsparser_checkandnexttoken(vm, loadf, ')', "until", ")");

    lhsparser_op2(vm, loadf, OP_JNZ, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);
    state->block->len = (int)
    (
        state->block->pos - lhsparser_castlex(loadf)->curfunction->code.usize
    );
    state->block->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    return LHS_TRUE;
}

static int lhsparser_dostate(LHSVM* vm, LHSLoadF* loadf)
{
    /*dostate -> do blockstate doarg*/
    LHSDoState state;
    lhsparser_resetdostate(vm, loadf, &state);

    lhsparser_regionforward(vm, loadf, LHS_TOKENDO);
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENDO, "do", "do");
    
    state.block->pos = lhsparser_castlex(loadf)->curfunction->code.usize;

    lhsparser_blockstate(vm, loadf);
    lhsparser_doarg(vm, loadf, &state);

    lhsparser_regionback(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_funcargs(LHSVM* vm, LHSLoadF* loadf)
{
    /*funcargs -> null | {identifier [',']}*/
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (token->t == ')')
    {
        return 0;
    }

    int nparam = 0;
    while (LHS_TRUE)
    {
        lhsparser_checktoken(vm, loadf, LHS_TOKENIDENTIFIER, 
            "function", "<identifier>");
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        lhsvm_pushvalue(vm, -1);
        if (lhsparser_recursionfindvar(vm, loadf, 0))
        {
            lhsvm_pop(vm, 1);
            lhserr_syntax
            (
                vm, 
                loadf,
                "param duplicate definition '%s'.",
                token->buf.data
            );
        }

        nparam++;
        lhsparser_insertlocalvar(vm, loadf);
        lhsparser_nexttoken(vm, loadf);
        if (token->t != ',')
        {
            break;
        }

        lhsparser_nexttoken(vm, loadf);
    }

    return nparam;
}

static int lhsparser_funcstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*funcstate -> function [identifier] '(' funcargs ')' blockstate*/
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENFUNCTION, "function", "function");
    
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (token->t != LHS_TOKENIDENTIFIER)
    {
        lhsvm_pushstring(vm, lhsparser_anonymous(vm));
        lhsparser_checkandnexttoken(vm, loadf, '(', "function", "(");
    }
    else
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        lhsparser_nexttokenandcheck(vm, loadf, '(', "function", "(");
        lhsparser_nexttoken(vm, loadf);
    }
    
    lhsparser_enterfunction(vm, loadf);

    int narg = lhsparser_funcargs(vm, loadf);
    lhsparser_checkandnexttoken(vm, loadf, ')', "function", ")");

    lhsparser_blockstate(vm, loadf);

    lhsparser_castlex(loadf)->curfunction->narg = narg;
    if (lhsparser_castlex(loadf)->curfunction->nret == LHS_UNCERTAIN)
    {
        lhsparser_castlex(loadf)->curfunction->nret = LHS_VOID;
        lhsparser_op2(vm, loadf, OP_RET, loadf->line, loadf->column, 
            lhsparser_castlex(loadf)->curfunction->name);
    }

    lhsparser_leavefunction(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_retstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*retstate -> return [exprstate]*/
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENRETURN, "function", "return");

    LHSFunction* function = lhsparser_castlex(loadf)->curfunction;
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (token->t == '}')
    {
        if (function->nret == LHS_RESULT)
        {
            lhserr_syntax(vm, loadf, "incorrect function return.");
        }

        function->nret = LHS_VOID;
        lhsparser_op2(vm, loadf, OP_RET, loadf->line, loadf->column, function->name);
        return LHS_TRUE;
    }

    if (function->nret == LHS_VOID)
    {
        lhserr_syntax(vm, loadf, "incorrect function return.");
    }

    lhsparser_exprstate(vm, loadf);

    function->nret = LHS_RESULT;
    lhsparser_op2(vm, loadf, OP_RET1, loadf->line, loadf->column, function->name);
    lhsparser_checktoken(vm, loadf, '}', "return", "}");
    return LHS_TRUE;
}

static int lhsparser_continuestate(LHSVM* vm, LHSLoadF* loadf)
{
    /*continuestate -> continue*/
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENCONTINUE, "continue", "continue");
    
    LHSRegion* region = lhsparser_castlex(loadf)->curregion;
    if (!region)
    {
        lhserr_syntax(vm, loadf, "unexcepted 'continue'.");
    }

    lhsparser_op2(vm, loadf, OP_JMP, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);

    LHSJmp* jmp = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, jmp);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, jmp, next);

    jmp->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    if (region->token != LHS_TOKENDO)
    {
        jmp->len = (int)(region->continuejmp.pos - jmp->pos);
    }
    else
    {
        region->continuejmp.jmp = jmp;
    }
    return LHS_TRUE;
}

static int lhsparser_breakstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*breakstate -> break*/
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENBREAK, "break", "break");

    LHSRegion* region = lhsparser_castlex(loadf)->curregion;
    if (!region)
    {
        lhserr_syntax(vm, loadf, "unexcepted 'break'.");
    }

    lhsparser_op2(vm, loadf, OP_JMP, loadf->line, loadf->column, 
        lhsparser_castlex(loadf)->curfunction->name);
    lhsparser_index(vm, loadf, 0);

    LHSJmp* jmp = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, loadf, jmp);
    lhslink_forward(lhsparser_castlex(loadf), alljmp, jmp, next);

    jmp->pos = lhsparser_castlex(loadf)->curfunction->code.usize;
    jmp->len = 0;

    region->nbreak++;
    region->breakjmp = jmp;
    return LHS_TRUE;
}

static int lhsparser_statement(LHSVM* vm, LHSLoadF* loadf, int nested)
{
    while (!lhsparser_isyield(loadf, nested))
    {
        switch (lhsparser_castlex(loadf)->token.t)
        {
        case LHS_TOKENLOCAL:
        {
            lhsparser_localstate(vm, loadf);
            break;
        }
        case LHS_TOKENGLOBAL:
        {
            lhsparser_globalstate(vm, loadf);
            break;
        }
        case LHS_TOKENIF:
        {
            lhsparser_ifstate(vm, loadf);
            break;
        }
        case LHS_TOKENFOR:
        {
            lhsparser_forstate(vm, loadf);
            break;
        }
        case LHS_TOKENWHILE:
        {
            lhsparser_whilestate(vm, loadf);
            break;
        }
        case LHS_TOKENDO:
        {
            lhsparser_dostate(vm, loadf);
            break;
        }
        case LHS_TOKENFUNCTION:
        {
            lhsparser_funcstate(vm, loadf);
            break;
        }
        case LHS_TOKENRETURN:
        {
            lhsparser_retstate(vm, loadf);
            break;
        }
        case LHS_TOKENCONTINUE:
        {
            lhsparser_continuestate(vm, loadf);
            break;
        }
        case LHS_TOKENBREAK:
        {
            lhsparser_breakstate(vm, loadf);
            break;
        }
        case '{':
        {
            lhsparser_blockstate(vm, loadf);
            break;
        }
        case LHS_TOKENIDENTIFIER:
        {
            LHSToken* lookahead = &lhsparser_castlex(loadf)->lookahead;
            lhsparser_lookaheadtoken(vm, loadf);
            if (lookahead->t == '=')
            {
                lhsparser_exprstate(vm, loadf);
                break;
            }
         
            if (lookahead->t == '(')
            {
                lhsparser_callstate(vm, loadf);
                break;
            }
        }
        case LHS_TOKENINTEGER:
        case LHS_TOKENNUMBER:
        case LHS_TOKENSTRING:
        case LHS_TOKENTRUE:
        case LHS_TOKENFALSE:
        case '(':
        {
            lhserr_syntax
            (
                vm, 
                loadf, 
                "unexpected token '%s'.", 
                lhsparser_castlex(loadf)->token.buf.data
            );
        }
        case LHS_TOKENEOF:
        {
            break;
        }
        default:
        {
            if (lhsparser_istokensymbol(loadf))
            {
                lhserr_syntax
                (
                    vm,
                    loadf,
                    "unexpected symbol '%s'.",
                    lhsparser_castlex(loadf)->token.buf.data
                );
            }

            lhsparser_exprstate(vm, loadf);
            break;
        }
        }
    }

    return LHS_TRUE;
}

static int lhsparser_initstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsloadf_getc(loadf);
    lhsparser_nexttoken(vm, loadf);
    lhsparser_statement(vm, loadf, LHS_FALSE);
    return LHS_TRUE;
}

int lhsparser_loadfile(LHSVM* vm, const char* fname)
{    
    if (!lhsparser_initreserved(vm))
    {
        return LHS_FALSE;
    }

    LHSLoadF loadf;
    if (!lhsloadf_init(vm, &loadf, fname))
    {
        return LHS_FALSE;
    }

    LHSLexical lexical;
    lhsparser_initlexical(vm, &loadf, &lexical);

    if (!lhsparser_initmainframe(vm, &loadf, fname))
    {
        lhsparser_uninitlexical(vm, &loadf);
        lhsloadf_uninit(vm, &loadf);
        return LHS_FALSE;
    }

    int errcode = lhserr_protectedcall(vm, lhsparser_initstate, &loadf);
    if (errcode)
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
    }
    else
    {
        lhsparser_lexicalsolve(vm, &lexical);
        lhscode_op2
        (
            vm, 
            &lexical.curframe->mainfunc->code,
            OP_EXIT, 
            loadf.line, loadf.column, 
            lexical.curframe->mainfunc->name
        );
        lhscode_dmpcode(vm);
    }
    
    lhsparser_uninitlexical(vm, &loadf);
    lhsloadf_uninit(vm, &loadf);

    return !errcode;
}
