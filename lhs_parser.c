#include "lhs_parser.h"
#include "lhs_load.h"
#include "lhs_value.h"
#include "lhs_link.h"
#include "lhs_frame.h"
#include "lhs_code.h"
#include "lhs_error.h"

#define G                                         (1)
#define L                                         (0)
#define N                                         (-1)
#define E                                         (2)
#define LHS_EXPRNONE                              (0)
#define LHS_EXPRCONST                             (1)
#define LHS_EXPRINT                               (2 | LHS_EXPRCONST)
#define LHS_EXPRNUM                               (4 | LHS_EXPRCONST)
#define LHS_EXPRBOOLEAN                           (8 | LHS_EXPRCONST)
#define LHS_EXPRREF                               (16)
#define LHS_EXPRCALL                              (32)

#define lhsparser_castlex(lf)                                       \
((LHSLexical*)(lf)->lexical)

#define lhsparser_issymbol(t)                                       \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) > LHS_TOKENSYMBOLBEGIN &&                                     \
  (t) < LHS_TOKENSYMBOLEND) :                                       \
(lhsloadf_symbol[(t)] > SYMBOL_BEGIN &&                             \
 lhsloadf_symbol[(t)] < SYMBOL_END))

#define lhsparser_istokensymbol(lf)                                 \
lhsparser_issymbol(lhsparser_castlex(lf)->token.t)

#define lhsparser_isunarysymbol(s)                                  \
((s) == SYMBOL_NOT || (s) == SYMBOL_NOTB || (s) == SYMBOL_MINUS)

#define lhsparser_islbracket(lf)                                    \
(lhsparser_castlex(lf)->token.t == '(')

#define lhsparser_resetframe(vm, n)                                 \
lhsframe_castcurframe(vm)->narg = n;                                \
(vm)->currentframe = (vm)->mainframe

#define lhsparser_truncate(t)                                       \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) & UCHAR_MAX) :                                                \
 lhsloadf_symbol[t])

#define lhsparser_isyield(lf, n)                                    \
(lhsparser_castlex(lf)->token.t == LHS_TOKENEOF ||                  \
 ((n) && lhsparser_castlex(lf)->token.t == '}'))

typedef struct LHSExprUnary
{
    struct LHSExprUnary* next;
    char unary;
} LHSExprUnary;

typedef struct LHSExprChain
{
    struct LHSExprChain* prev;
    char type;
    char symbol;
    int nunary;
    LHSExprUnary* unary;
    union
    {
        struct
        {
            char mark;
            int index;
        } ref;
        struct
        {
            char mark;
            int index;
            int narg;
            int nret;
        } call;
        char b;
        long long i;
        double n;
    } factor;
} LHSExprChain;

typedef struct LHSExprState
{
    LHSExprChain* chain;
    LHSExprUnary* unary;
} LHSExprState;

typedef struct LHSIfState
{
    LHSJmp* branch;
    LHSJmp* finish;
} LHSIfState;

typedef struct LHSFuncState
{
    LHSJmp* leave;
    LHSJmp* finish;
} LHSFuncState;

static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_exprstate(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_uninitexprstate(LHSVM* vm, LHSExprState* state);
static int lhsparser_statement(LHSVM* vm, LHSLoadF* loadf, int nested);
static int lhsparser_exprsub(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state);

static const char* reserveds[] =
{
    "", "set", "var", "function", "for", "while",
    "if", "else", "switch", "case", "default", 
    "break", "continue", "true", "false", "return"
};

static const char priorities[][SYMBOL_END] =
{
     /*N/A, +, -, *, /, %, &, |, ^, <, >,==,!=,>=,<=,&&,||,<<,>>, -, !, ~, =*/
/*N/A*/{ E, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
/*+*/  { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*-*/  { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/***/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*/*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*%*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*&*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*|*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*^*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*<*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*>*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*==*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*!=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*>=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*<=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*&&*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*||*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*<<*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*>>*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*-*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N}, 
/*!*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*~*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, N},
/*=*/  { L, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, N}
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
        LHSString* str = lhsvalue_caststring
        (
            lhsmem_newgcstring
            (
                vm, 
                (void*)reserveds[i], 
                i + LHS_TOKENRESERVEDBEGIN
            )
        );
        lhshash_insert(vm, &vm->shortstrhash, str, &str->hash);
    }

    return LHS_TRUE;
}

static int lhsparser_initmainframe(LHSVM* vm, LHSLoadF *loadf, const char* fname)
{
    vm->mainframe = lhsmem_newgcobject
    (
        vm, 
        sizeof(LHSFrame), 
        LHS_TGCFRAME
    );

    lhsframe_init(vm, vm->mainframe);
    lhsframe_enterchunk(vm, vm->mainframe, loadf);
    vm->currentframe = vm->mainframe;

    lhsvm_pushstring(vm, fname);
    LHSVariable* variable = lhsframe_insertvar
    (
        vm, 
        lhsframe_castmainframe(vm), 
        0, 
        0
    );
    LHSValue* value = lhsvalue_castvalue
    (
        lhsvector_at
        (
            vm, 
            &lhsframe_castmainframe(vm)->values, 
            variable->index
        )
    );
    value->type = LHS_TGC;
    value->gc = lhsgc_castgc(variable->name);
    lhsframe_castmainframe(vm)->name = variable->index;
    return LHS_TRUE;
}

static int lhsparser_insertframe(LHSVM* vm, LHSLoadF* loadf)
{
    LHSFrame* frame = lhsframe_castframe
    (
        lhsmem_newgcobject
        (
            vm, 
            sizeof(LHSFrame), 
            LHS_TGCFRAME
        )
    );

    lhsframe_init(vm, frame);
    lhsframe_enterchunk(vm, frame, loadf);
    vm->currentframe = frame;

    lhsvm_pushvalue(vm, -1);
    LHSVariable* variable = lhsframe_getvariable(vm, lhsframe_castmainframe(vm));
    if (variable)
    {
        variable->chunk = 0;
        variable->mark = LHS_MARKGLOBAL;

        LHSValue* value = lhsvector_at
        (
            vm,
            &lhsframe_castmainframe(vm)->values,
            variable->index
        );
        value->type = LHS_TGC;
        value->gc = lhsgc_castgc(frame);
    }
    else
    {
        lhsvm_pushvalue(vm, -1);
        LHSVariable* framebody = lhsframe_insertglobal
        (
            vm, 
            lhsframe_castmainframe(vm), 
            loadf->line, 
            loadf->column
        );
        LHSValue* framevalue = lhsvalue_castvalue
        (
            lhsvector_at
            (
                vm, 
                &lhsframe_castmainframe(vm)->values,
                framebody->index
            )
        );
        framevalue->type = LHS_TGC;
        framevalue->gc = lhsgc_castgc(frame);
    }

    LHSVariable* framename = lhsframe_insertvar
    (
        vm, 
        lhsframe_castcurframe(vm), 
        loadf->line, 
        loadf->column
    );
    LHSValue* namevalue = lhsvalue_castvalue
    (
        lhsvector_at
        (
            vm, 
            &lhsframe_castcurframe(vm)->values, 
            framename->index
        )
    );
    namevalue->type = LHS_TGC;
    namevalue->gc = lhsgc_castgc(framename->name);
    lhsframe_castcurframe(vm)->name = framename->index;

    return LHS_TRUE;
}

static int lhsparser_initjmp(LHSVM* vm, LHSJmp* jmp)
{
    jmp->pos = 0;
    jmp->len = 0;
    jmp->next = 0;
    return LHS_TRUE;
}

static int lhsparser_uninitjmp(LHSLexical* lex, LHSJmp* jmp, LHSVM *vm)
{
    lhs_unused(lex);
    lhsmem_freeobject(vm, jmp, sizeof(LHSJmp));
    return LHS_TRUE;
}

static int lhsparser_initlexical(LHSVM* vm, LHSLoadF* loadf, LHSLexical* lex)
{
    lex->token.t = LHS_TOKENEOF;
    lex->lookahead.t = LHS_TOKENEOF;
    lex->alljmp = 0;
    lhsparser_castlex(loadf) = lex;

    lhsbuf_init(vm, &lex->token.buf);
    lhsbuf_init(vm, &lex->lookahead.buf);
    return LHS_TRUE;
}

static int lhsparser_jmpsolve(LHSLexical* lex, LHSJmp* jmp, LHSVM* vm)
{
    long long* addr = (long long*)(vm->code.data + jmp->pos - sizeof(long long));
    *addr = jmp->pos + jmp->len;
    return LHS_TRUE;
}

static int lhsparser_lexicalsolve(LHSVM* vm, LHSLexical* lex)
{
    lhsslink_foreach(LHSJmp, lex, alljmp, next, lhsparser_jmpsolve, vm);
    return LHS_TRUE;
}

static int lhsparser_uninitlexical(LHSVM* vm, LHSLoadF* loadf)
{
    lhsslink_foreach
    (
        LHSJmp, 
        lhsparser_castlex(loadf), 
        alljmp, 
        next, 
        lhsparser_uninitjmp, 
        vm
    );
    lhsbuf_uninit(vm, &lhsparser_castlex(loadf)->token.buf);
    lhsbuf_uninit(vm, &lhsparser_castlex(loadf)->lookahead.buf);
    return LHS_TRUE;
}

static int lhsparser_nextlexical(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf)
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
                if (lhsbuf_isshort(vm, buf))
                {
                    LHSString* str = lhsvm_findshort(vm, buf->data, buf->usize);
                    if (str && str->reserved)
                    {
                        return str->reserved;
                    }
                }
                return LHS_TOKENIDENTIFIER;
            }

            if (loadf->current == LHS_TOKENEOF)
            {
                return LHS_TOKENEOF;
            }

            lhserr_syntaxerr
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
        if (lhsparser_castlex(loadf)->token.t == LHS_TOKENEOF)
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf, 
                "<eof> unexpected end of solving %s.",
                prefix,
                lhsparser_castlex(loadf)->token.buf.data
            );
        }
        else
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf, 
                "unexpected %s solving, expected '%s', got '%s'.",
                prefix,
                name,
                lhsparser_castlex(loadf)->token.buf.data
            );
        }
    }
    return LHS_TRUE;
}

static int lhsparser_checklookahead(LHSVM* vm, LHSLoadF* loadf,
    int token, const char* prefix, const char* name)
{
    if (lhsparser_castlex(loadf)->lookahead.t != token)
    {
        if (lhsparser_castlex(loadf)->lookahead.t == LHS_TOKENEOF)
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf, 
                "<eof> unexpected end of solving %s.",
                prefix,
                lhsparser_castlex(loadf)->lookahead.buf.data
            );
        }
        else
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf, 
                "unexpected %s solving, expected '%s', got '%s'.",
                prefix,
                name,
                lhsparser_castlex(loadf)->lookahead.buf.data
            );
        }
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

static int lhsparser_resetexprnode(LHSVM* vm, LHSExprChain* chain, LHSExprState* state)
{
    chain->prev = state->chain;
    state->chain = chain;

    chain->symbol = SYMBOL_NONE;
    chain->type = LHS_EXPRNONE;
    chain->nunary = 0;
    lhsslink_init(chain, unary);
    return LHS_TRUE;
}

static int lhsparser_initexprstate(LHSVM* vm, LHSExprState* state)
{
    state->chain = 0;
    lhsslink_init(state, unary);
    return LHS_TRUE;
}

static int lhsparser_uninitexprunary(LHSExprState* state, LHSExprUnary* unary, LHSVM* vm)
{
    lhsmem_freeobject(vm, unary, sizeof(LHSExprUnary));
    return LHS_TRUE;
}

static int lhsparser_uninitexprstate(LHSVM* vm, LHSExprState* state)
{
    lhsslink_foreach(LHSExprUnary, state, unary, next, lhsparser_uninitexprunary, vm);
    return LHS_TRUE;
}

static int lhsparser_resetifstate(LHSVM* vm, LHSLoadF* loadf, LHSIfState* state)
{
    state->branch = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, state->branch);
    lhsslink_push(lhsparser_castlex(loadf), alljmp, state->branch, next);

    state->finish = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, state->finish);
    lhsslink_push(lhsparser_castlex(loadf), alljmp, state->finish, next);
    return LHS_TRUE;
}

static int lhsparser_resetfuncstate(LHSVM* vm, LHSLoadF* loadf, LHSFuncState* state)
{
    state->leave = 0;/* lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, state->leave);
    lhsslink_push(lhsparser_castlex(loadf), alljmp, state->leave, next);*/

    state->finish = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, state->finish);
    lhsslink_push(lhsparser_castlex(loadf), alljmp, state->finish, next);
    return LHS_TRUE;
}

static int lhsparser_exprcode(LHSVM* vm, LHSExprChain* chain)
{
    switch (chain->type)
    {
    case LHS_EXPRINT:
    {
        lhscode_op(vm, OP_PUSH);
        lhscode_integer(vm, chain->factor.i);
        break;
    }
    case LHS_EXPRNUM:
    {
        lhscode_op(vm, OP_PUSH);
        lhscode_number(vm, chain->factor.n);
        break;
    }
    case LHS_EXPRBOOLEAN:
    {
        lhscode_op(vm, OP_PUSH);
        lhscode_boolean(vm, chain->factor.b);
        break;
    }
    case LHS_EXPRREF:
    {
        lhscode_op(vm, OP_PUSH);
        lhscode_ref(vm, chain->factor.ref.mark, chain->factor.ref.index);
        break;
    }
    case LHS_EXPRCALL:
    {
        lhscode_op(vm, OP_CALL);
        lhscode_ref(vm, chain->factor.call.mark, chain->factor.call.index);
        lhscode_index(vm, chain->factor.call.narg);
        lhscode_index(vm, chain->factor.call.nret);
        break;
    }
    default:
    {
        return LHS_TRUE;
    }
    }

    for (int i = 0; i < chain->nunary; ++i)
    {
        lhscode_op(vm, chain->unary->unary);
        chain->unary = chain->unary->next;
    }
    return LHS_TRUE;
}

static int lhsparser_exprmov(LHSVM* vm, LHSExprChain* chain)
{
    lhscode_op(vm, chain->type == LHS_EXPRREF ? OP_MOV : OP_MOVS);
    lhscode_ref(vm, chain->prev->factor.ref.mark, chain->prev->factor.ref.index);
    if (chain->type == LHS_EXPRREF)
    {
        lhscode_ref(vm, chain->factor.ref.mark, chain->factor.ref.index);
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
    case L:
    {
        /*if ((prev->type & LHS_EXPRCONST) &&
            (chain->type & LHS_EXPRCONST))
        {

        }
        else*/
        {
            if (chain->prev->symbol == SYMBOL_ASSIGN)
            {
                lhsparser_exprmov(vm, chain);
            }
            else
            {
                lhsparser_exprcode(vm, chain->prev);
                lhsparser_exprcode(vm, chain);
                lhscode_op(vm, chain->prev->symbol);
            }

            chain->prev->type = LHS_EXPRNONE;
            chain->type = LHS_EXPRNONE;
        }

        chain->prev = prev->prev;
        return lhsparser_exprsolve(vm, loadf, state);
    }
    case E:
    {
        lhsparser_exprcode(vm, chain);
        state->chain = prev->prev;
        return lhsparser_exprsolve(vm, loadf, state);
    }
    case N:
    {
        lhserr_syntaxerr
        (
            vm,
            loadf,
            "unexpected expression symbol '%s'.",
            lhsparser_castlex(loadf)->token.t
        );
    }
    default:
    {
        break;
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
        ++narg;
        lhsparser_exprstate(vm, loadf);
        if (token->t != ',')
        {
            break;
        }
        lhsparser_nexttoken(vm, loadf);
    }
    
    return narg;
}

static int lhsparser_exprfunc(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    /*exprfunc -> LHS_TOKENIDENTIFIER '(' [exprargs] ')'*/
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENIDENTIFIER, "function", "<identifier>");
    lhsvm_pushvalue(vm, -1);
    LHSVariable* function = lhsframe_getvariable(vm, lhsframe_castcurframe(vm));
    if (!function)
    {
        function = lhsframe_insertglobal
        (
            vm,
            lhsframe_castmainframe(vm),
            loadf->line,
            loadf->column
        );
    }
    else
    {
        lhsvm_pop(vm, 1);
    }

    lhsparser_checkandnexttoken(vm, loadf, '(', "function", "(");

    int argn = lhsparser_exprargs(vm, loadf, state);
    state->chain->type = LHS_EXPRCALL;
    state->chain->factor.call.mark = function->mark;
    state->chain->factor.call.index = function->index;
    state->chain->factor.call.narg = argn;
    state->chain->factor.call.nret = LHS_MULTRET;

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
                    [{op_unary}] exprfunc               |
                    [{op_unary}] '('exprstate')'*/
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    switch (token->t)
    {
    case LHS_TOKENINTEGER:
    {
        state->chain->type = LHS_EXPRINT;
        state->chain->factor.i = atoll(token->buf.data);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENNUMBER:
    {
        state->chain->type = LHS_EXPRNUM;
        state->chain->factor.n = atof(token->buf.data);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENSTRING:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        LHSVariable *constant = lhsframe_insertconstant(vm);
        state->chain->type = LHS_EXPRREF;
        state->chain->factor.ref.mark = constant->mark;
        state->chain->factor.ref.index = constant->index;
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENTRUE:
    {
        state->chain->type = LHS_EXPRBOOLEAN;
        state->chain->factor.b = LHS_TRUE;
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENFALSE:
    {
        state->chain->type = LHS_EXPRBOOLEAN;
        state->chain->factor.b = LHS_FALSE;
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
            lhsparser_exprfunc(vm, loadf, state);            
        }
        else
        {
            LHSVariable* variable = lhsframe_getvariable(vm, lhsframe_castcurframe(vm));
            if (!variable)
            {
                lhserr_syntaxerr
                (
                    vm, 
                    loadf, 
                    "undefined variable '%s'.",
                    token->buf.data
                );
            }
            state->chain->type = LHS_EXPRREF;
            state->chain->factor.ref.mark = variable->mark;
            state->chain->factor.ref.index = variable->index;
            lhsparser_nexttoken(vm, loadf);
        }
        break;
    }
    case '(':
    {
        state->chain->type = LHS_EXPRNONE;
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
            lhsslink_init(unary, next);
            lhsslink_push(state, unary, unary, next);

            state->chain->unary = unary;
            ++state->chain->nunary;
            
            lhsparser_exprfactor(vm, loadf, state);
            return LHS_TRUE;
        }

        if (token->t == LHS_TOKENEOF)
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf, 
                "<eof> unexpected end of expression.",
                lhsparser_castlex(loadf)->token.buf.data
            );
        }
        else
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf, 
                "unexpected expression factor, got '%s'.",
                lhsparser_castlex(loadf)->token.buf.data
            );
        }
    }
    }

    return LHS_TRUE;
}

static int lhsparser_exprchain(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    /*exprchain -> exprfactor op_binary*/
    lhsparser_exprfactor(vm, loadf, state);
    
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (lhsparser_isunarysymbol(token->t))
    {
        lhserr_syntaxerr
        (
            vm, 
            loadf, 
            "unexpected unary symbol, got '%s'.",
            token->buf.data
        );
    }

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
    lhsparser_resetexprnode(vm, &chain, state);
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
    lhsparser_resetexprnode(vm, &chain, &state);

    if (lhserr_protectedcallex(vm, lhsparser_exprsub, loadf, &state))
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
    }

    lhsparser_uninitexprstate(vm, &state);
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
        if (lhsframe_getvariable(vm, lhsframe_castcurframe(vm)))
        {
            lhsvm_pop(vm, 1);
            lhserr_syntaxerr
            (
                vm, 
                loadf,
                "variable duplicate definition '%s'.",
                lhsparser_castlex(loadf)->token.buf.data
            );
        }

        LHSVariable* variable = lhsframe_insertvar
        (
            vm, 
            lhsframe_castcurframe(vm), 
            loadf->line, 
            loadf->column
        );

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
        if (lhsframe_getvariable(vm, lhsframe_castcurframe(vm)))
        {
            lhsvm_pop(vm, 1);
            lhserr_syntaxerr
            (
                vm, 
                loadf,
                "variable duplicate definition '%s'.",
                lhsparser_castlex(loadf)->token.buf.data
            );
        }

        LHSVariable* variable = lhsframe_insertglobal
        (
            vm, 
            lhsframe_castcurframe(vm), 
            loadf->line, 
            loadf->column
        );

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

    lhsparser_statement(vm, loadf, LHS_TRUE);

    lhsparser_checkandnexttoken(vm, loadf, '}', "block", "}");
    return LHS_TRUE;
}

int lhsparser_iftrue(LHSVM* vm, LHSLoadF* loadf, LHSIfState* state)
{
    /*iftrue -> blockstate*/
    state->branch->pos = vm->code.usize;
    
    lhsframe_enterchunk(vm, lhsframe_castcurframe(vm), loadf);
    lhsparser_blockstate(vm, loadf);
    lhsframe_leavechunk(vm, lhsframe_castcurframe(vm), loadf);

    state->finish->pos = vm->code.usize;
    return LHS_TRUE;
}

int lhsparser_iffalse(LHSVM* vm, LHSLoadF* loadf, LHSIfState* state)
{
    /*iffalse -> blockstate*/
    state->branch->len = vm->code.usize - state->branch->pos;

    lhsframe_enterchunk(vm, lhsframe_castcurframe(vm), loadf);
    lhsparser_blockstate(vm, loadf);
    lhsframe_leavechunk(vm, lhsframe_castcurframe(vm), loadf);

    state->finish->len = vm->code.usize - state->finish->pos;
    return LHS_TRUE;
}

static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*ifstate -> if '(' exprstate ')' ifstrue [{ifstrue}] [iffalse]*/
    LHSIfState state;
    lhsparser_resetifstate(vm, loadf, &state);

    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENIF, "if", "if");
    lhsparser_checkandnexttoken(vm, loadf, '(', "if", "(");
    lhsparser_exprstate(vm, loadf);
    lhsparser_checkandnexttoken(vm, loadf, ')', "if", ")");
    lhsparser_iftrue(vm, loadf, &state);

    LHSToken* token = &lhsparser_castlex(loadf)->token,
        * lookahead = &lhsparser_castlex(loadf)->lookahead;
    do
    {
        if (token->t != LHS_TOKENELSE)
        {
            break;
        }

        lhsparser_lookaheadtoken(vm, loadf);
        if (lookahead->t != LHS_TOKENIF)
        {
            break;
        }

        lhsparser_nexttoken(vm, loadf);
        state.branch->len = vm->code.usize - state.branch->pos;
        return lhsparser_ifstate(vm, loadf);
    } while (LHS_FALSE);

    if (token->t == LHS_TOKENELSE)
    {
        lhsparser_nexttoken(vm, loadf);
        lhsparser_iffalse(vm, loadf, &state);
    }

    return LHS_TRUE;
}

static int lhsparser_foragr1(LHSVM* vm, LHSLoadF* loadf)
{
    /*forarg1 -> exprstate*/
    return LHS_TRUE;
}

static int lhsparser_foragr2(LHSVM* vm, LHSLoadF* loadf)
{
    /*forarg2 -> exprstate*/
    return LHS_TRUE;
}

static int lhsparser_foragr3(LHSVM* vm, LHSLoadF* loadf)
{
    /*forarg3 -> exprstate*/
    return LHS_TRUE;
}

static int lhsparser_forargs(LHSVM* vm, LHSLoadF* loadf)
{
    /*forargs -> [forarg1] ';' [forarg2] ';' [forarg3]*/
    return LHS_TRUE;
}

static int lhsparser_forstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*forstate -> for '(' forargs ')' blockstate*/
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
        if (lhsframe_getvariable(vm, lhsframe_castcurframe(vm)))
        {
            lhsvm_pop(vm, 1);
            lhserr_syntaxerr
            (
                vm, 
                loadf,
                "param duplicate definition '%s'.",
                token->buf.data
            );
        }

        LHSVariable* param = lhsframe_insertparam
        (
            vm,
            lhsframe_castcurframe(vm),
            loadf->line,
            loadf->column
        );
        param->mark = LHS_MARKSTACK;
        param->index = ++nparam;

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
    LHSFuncState state;
    lhsparser_resetfuncstate(vm, loadf, &state);
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

    state.finish->pos = vm->code.usize;

    lhsparser_insertframe(vm, loadf);
    int narg = lhsparser_funcargs(vm, loadf);
    lhsparser_checkandnexttoken(vm, loadf, ')', "function", ")");

    lhsparser_blockstate(vm, loadf);
    state.finish->len = vm->code.usize - state.finish->pos;

    lhsparser_resetframe(vm, narg);
    return LHS_TRUE;
}

static int lhsparser_retstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*retstate -> return [exprstate]*/
    lhsparser_checkandnexttoken(vm, loadf, LHS_TOKENRETURN, "function", "return");

    LHSFrame* frame = lhsframe_castcurframe(vm);
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    if (token->t == '}')
    {
        if (frame->nret == LHS_RETSULT)
        {
            lhserr_syntaxerr(vm, loadf, "incorrect function return.");
        }

        frame->curchunk = 0;
        frame->nret = LHS_VOIDRET;
        return LHS_TRUE;
    }

    if (frame->nret == LHS_VOIDRET)
    {
        lhserr_syntaxerr(vm, loadf, "incorrect function return.");
    }

    lhsparser_exprstate(vm, loadf);

    frame->curchunk = 0;
    frame->nret = LHS_RETSULT;
    lhsparser_checktoken(vm, loadf, '}', "return", "}");
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
        case '{':
        {
            lhsparser_blockstate(vm, loadf);
            break;
        }
        case LHS_TOKENIDENTIFIER:
        {
            LHSToken* lookahead = &lhsparser_castlex(loadf)->lookahead;
            lhsparser_lookaheadtoken(vm, loadf);
            if (lookahead->t == '=' ||
                lookahead->t == '(')
            {
                goto exprstate;
            }
            else
            {
                goto syntaxerr;
            }
            break;
        }
        case LHS_TOKENINTEGER:
        case LHS_TOKENNUMBER:
        case LHS_TOKENSTRING:
        case LHS_TOKENTRUE:
        case LHS_TOKENFALSE:
        case '(':
        {
syntaxerr:
            lhserr_syntaxerr
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
exprstate:
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
        lhsframe_castmainframe(vm)->curchunk = 0;
        lhsparser_lexicalsolve(vm, &lexical);
        lhscode_dmpcode(vm);
    }
    
    lhsparser_uninitlexical(vm, &loadf);
    lhsloadf_uninit(vm, &loadf);

    return !errcode;
}
