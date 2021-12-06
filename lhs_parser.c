#include "lhs_parser.h"
#include "lhs_load.h"
#include "lhs_value.h"
#include "lhs_link.h"
#include "lhs_frame.h"
#include "lhs_code.h"
#include "lhs_error.h"

#define G   (1)
#define L   (0)
#define N   (-1)
#define E   (2)

#define lhsparser_isexprvalue(t)                                    \
((t) == LHS_TOKENNUMBER  ||                                         \
 (t) == LHS_TOKENINTEGER ||                                         \
 (t) == LHS_TOKENSTRING  ||                                         \
 (t) == LHS_TOKENTRUE    ||                                         \
 (t) == LHS_TOKENFALSE   ||                                         \
 (t) == LHS_TOKENIDENTIFIER)

#define lhsparser_castlex(lf)                                       \
((LHSLexical*)(lf)->lexical)

#define lhsparser_isassignment(lf)                                  \
(lhsparser_castlex(lf)->token.t == '=')

#define lhsparser_isexprsymbol(t)                                   \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) > LHS_TOKENSYMBOLBEGIN &&                                     \
  (t) < LHS_TOKENSYMBOLEND) :                                       \
(lhsloadf_symbolid[(t)] > SYMBOL_BEGIN &&                           \
 lhsloadf_symbolid[(t)] < SYMBOL_END))

#define lhsparser_iscurrentsymbol(lf)                               \
lhsparser_isexprsymbol(lhsparser_castlex(lf)->token.t)

#define lhsparser_islookaheadsymbol(lf)                              \
lhsparser_isexprsymbol(lhsparser_castlex(state->loadf)->lookahead.t)

#define lhsparser_isunarysymbol(s)                                  \
((s) == SYMBOL_NOT || (s) == SYMBOL_BNOT || (s) == SYMBOL_MINUS)

#define lhsparser_truncate(t)                                       \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) & UCHAR_MAX) :                                                \
 lhsloadf_symbolid[t])

#define lhsparser_ischunkfollow(lf)                                 \
(lhsparser_castlex(lf)->token.t == '}' ||                           \
 lhsparser_castlex(lf)->token.t == LHS_TOKENEOF)

typedef struct LHSExprState
{
    int read;
    LHSLoadF* loadf;
    LHSSTRBUF symbols;
} LHSExprState;

typedef struct LHSIfState
{
    LHSLoadF* loadf;
    LHSJmp* branch;
    LHSJmp* finish;
} LHSIfState;

static int lhsparser_statement(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_statesolve(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf);

static const char* reserveds[] =
{
    "", "set", "var", "function", "for", "while",
    "if", "else", "switch", "case", "default", 
    "break", "continue", "true", "false", "return",
    "<name>", "<integer>", "<number>", "<string>"
};

static const char lhsparser_prioritytable[][SYMBOL_END] =
{
     /*N/A, +, -, *, /, %, &, |, ^, <, >,==,!=,>=,<=,&&,||,<<,>>, -, !, ~, (, )*/
/*N/A*/{ E, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, N},
/*+*/  { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*-*/  { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/***/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*/*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*%*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*&*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*|*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*^*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*<*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*>*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*==*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*!=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*>=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*<=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*&&*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*||*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*<<*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*>>*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*-*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L}, 
/*!*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*~*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, G, L},
/*(*/  { N, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, E},
/*)*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, N, N, N, N, N}
};

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
    LHSVariable* variable = lhsframe_insertvariable
    (
        vm, 
        lhsframe_castmainframe(vm), 
        0, 
        0,
        LHS_TRUE
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

static int lhsparser_solvejmp(LHSLexical* lex, LHSJmp* jmp, LHSVM* vm)
{
    long long* addr = (long long*)(vm->code.data + jmp->pos - sizeof(long long));
    *addr = (long long)vm->code.data + jmp->pos + jmp->len;
    return LHS_TRUE;
}

static int lhsparser_lexicalsolve(LHSVM* vm, LHSLexical* lex)
{
    lhsslink_foreach(LHSJmp, lex, alljmp, next, lhsparser_solvejmp, vm);
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
            if (lhsparser_isassignment(loadf) ||
                lhsparser_iscurrentsymbol(loadf))
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
                    if (str && str->extra)
                    {
                        return str->extra;
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
    int token, const char* prefix)
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
                "unexpected %s solving, got '%s'.",
                prefix,
                lhsparser_castlex(loadf)->token.buf.data
            );
        }
    }
    return LHS_TRUE;
}

static int lhsparser_checklookahead(LHSVM* vm, LHSLoadF* loadf,
    int token, const char* prefix)
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
                "unexpected %s solving, got '%s'.",
                prefix,
                lhsparser_castlex(loadf)->lookahead.buf.data
            );
        }
    }
    return LHS_TRUE;
}

static int lhsparser_checkandnexttoken(LHSVM* vm, LHSLoadF* loadf, 
    int token, const char *prefix)
{
    lhsparser_checktoken(vm, loadf, token, prefix);
    lhsparser_nexttoken(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_nexttokenandcheck(LHSVM* vm, LHSLoadF* loadf, 
    int token, const char *prefix)
{
    lhsparser_nexttoken(vm, loadf);
    lhsparser_checktoken(vm, loadf, token, prefix);
    return LHS_TRUE;
}

static int lhsparser_lookaheadtoken(LHSVM* vm, LHSLoadF* loadf)
{
    lhsparser_castlex(loadf)->lookahead.t = lhsparser_nextlexical
    (
        vm, 
        loadf, 
        &lhsparser_castlex(loadf)->lookahead.buf
    );
    return LHS_TRUE;
}

static int lhsparser_checkandlookahead(LHSVM* vm, LHSLoadF* loadf, 
    int token, const char *prefix)
{
    lhsparser_checktoken(vm, loadf, token, prefix);
    lhsparser_lookaheadtoken(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_lookaheadandcheck(LHSVM* vm, LHSLoadF* loadf, 
    int token, const char *prefix)
{
    lhsparser_lookaheadtoken(vm, loadf);
    lhsparser_checklookahead(vm, loadf, token, prefix);
    return LHS_TRUE;
}

static int lhsparser_initexprstate(LHSVM* vm, LHSExprState* state, 
    LHSLoadF* loadf)
{
    state->read = LHS_TRUE;
    state->loadf = loadf;
    lhsbuf_init(vm, &state->symbols);
    lhsbuf_pushc(vm, &state->symbols, SYMBOL_NONE);
    return LHS_TRUE;
}

static void lhsparser_uninitexprstate(LHSVM* vm, LHSExprState* state)
{
    lhsbuf_uninit(vm, &state->symbols);
}

static int lhsparser_initifstate(LHSVM* vm, LHSIfState* state, LHSLoadF* loadf)
{
    state->loadf = loadf;

    state->branch = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, state->branch);
    lhsslink_push(lhsparser_castlex(loadf), alljmp, state->branch, next);

    state->finish = lhsmem_newobject(vm, sizeof(LHSJmp));
    lhsparser_initjmp(vm, state->finish);
    lhsslink_push(lhsparser_castlex(loadf), alljmp, state->finish, next);
    return LHS_TRUE;
}

static void lhsparser_uninitifstate(LHSVM* vm, LHSIfState* state)
{

}

static int lhsparser_exprrecursive(LHSVM* vm, LHSExprState* state, char nsymbol)
{
    int result = LHS_FALSE;
    if (lhsbuf_isempty(vm, &state->symbols))
    {
        return result;
    }

    char osymbol;
    lhsbuf_topc(vm, &state->symbols, &osymbol);

    switch (lhsparser_prioritytable[osymbol][nsymbol])
    {
    case G:
    {
        lhsbuf_pushc(vm, &state->symbols, nsymbol);
        break;
    }
    case L:
    {
        if (lhsparser_isunarysymbol(osymbol))
        {
            lhscode_unaryi(vm, osymbol, LHS_MARKSTACK, -1);
        }
        else
        {
            lhscode_binary(vm, osymbol, LHS_MARKSTACK, -2, LHS_MARKSTACK, -1);
        }

        lhsbuf_popc(vm, &state->symbols, &osymbol);
        result = LHS_TRUE;
        break;
    }
    case E:
    {
        lhsbuf_popc(vm, &state->symbols, &osymbol);
        break;
    }
    default:
    {
        lhserr_syntaxerr
        (
            vm, 
            state->loadf,
            "solving symbol '%s, %s'.",
            lhsloadf_symbolname[osymbol],
            lhsloadf_symbolname[nsymbol]
        );
    }
    }

    return result;
}

static int lhsparser_exprsolve(LHSVM* vm, LHSExprState* state)
{
    int token = LHS_TOKENEOF;
    while (!lhsbuf_isempty(vm, &state->symbols))
    {
        if (state->read)
        {
            lhsparser_nexttoken(vm, state->loadf);
            token = lhsparser_castlex(state->loadf)->token.t;

            if (token == LHS_TOKENINTEGER)
            {
                lhscode_unaryl(vm, OP_PUSH, atoll(lhsparser_castlex(state->loadf)->token.buf.data));
                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_islookaheadsymbol(state->loadf);
                continue;
            }
            
            if (token == LHS_TOKENNUMBER)
            {
                lhscode_unaryf(vm, OP_PUSH, atof(lhsparser_castlex(state->loadf)->token.buf.data));
                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_islookaheadsymbol(state->loadf);
                continue;
            }

            if (token == LHS_TOKENSTRING)
            {
                lhsvm_pushlstring(vm, lhsparser_castlex(state->loadf)->token.buf.data,
                    lhsparser_castlex(state->loadf)->token.buf.usize);
                LHSVariable* constvar = lhsvm_insertconstant(vm);
                lhscode_unaryi(vm, OP_PUSH, constvar->mark,  constvar->index);
                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_islookaheadsymbol(state->loadf);
                continue;
            }

            if (token == LHS_TOKENIDENTIFIER)
            {
                lhsvm_pushlstring(vm, lhsparser_castlex(state->loadf)->token.buf.data,
                    lhsparser_castlex(state->loadf)->token.buf.usize);
                LHSVariable* var = lhsframe_getvariable
                (
                    vm, 
                    lhsframe_castcurframe(vm)
                );
                if (!var)
                {
                    lhserr_syntaxerr
                    (
                        vm, 
                        state->loadf, 
                        "undefined variable '%s'.", 
                        lhsparser_castlex(state->loadf)->token.buf.data
                    );
                }
                lhscode_unaryi(vm, OP_PUSH, var->mark,var->index);
                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_islookaheadsymbol(state->loadf);
                continue;
            }

            if (token == LHS_TOKENTRUE ||
                token == LHS_TOKENFALSE)
            {
                lhscode_unaryb(vm, OP_PUSH,LHS_TOKENTRUE ? 1 : 0);
                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_islookaheadsymbol(state->loadf);
                continue;
            }
        }

        char nsymbol = SYMBOL_NONE;
        if (lhsparser_isexprsymbol(token))
        {
            nsymbol = lhsparser_truncate(token);
        }

        while (lhsparser_exprrecursive(vm, state, nsymbol));
    }

    return LHS_TRUE;
}

static int lhsparser_exprstate(LHSVM* vm, LHSLoadF* loadf)
{   
    LHSExprState exprstate;
    lhsparser_initexprstate(vm, &exprstate, loadf);

    if (lhserr_protectedcall
    (
        vm, 
        lhsparser_exprsolve,
        &exprstate
    ))
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);

        lhsparser_uninitexprstate(vm, &exprstate);
        lhserr_throw(vm, "");
    }

    lhsparser_uninitexprstate(vm, &exprstate);
    return LHS_TRUE;
}

static int lhsparser_directmov(LHSVM* vm, LHSLoadF* loadf, 
    LHSVariable* variable)
{
    lhsparser_exprstate(vm, loadf);
    lhscode_binary(vm, OP_MOV, variable->mark, variable->index,LHS_MARKSTACK, -1);
    return LHS_TRUE;
}

static int lhsparser_movstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsvm_pushlstring
    (
        vm, 
        lhsparser_castlex(loadf)->token.buf.data, 
        lhsparser_castlex(loadf)->token.buf.usize
    );
    LHSVariable* variable = lhsframe_getvariable
    (
        vm,
        lhsframe_castcurframe(vm)
    );
    if (!variable)
    {
        lhserr_syntaxerr
        (
            vm, 
            loadf,
            "undefined variable '%s'.",
            lhsparser_castlex(loadf)->token.buf.data
        );
    }

    lhsparser_nexttoken(vm, loadf);
    if (lhsparser_castlex(loadf)->token.t != '=')
    {
        lhserr_syntaxerr
        (
            vm, 
            loadf,
            "unexpected token '%s'.",
            lhsparser_castlex(loadf)->token.buf.data
        );
    }

    return lhsparser_directmov(vm, loadf, variable);
}

static int lhsparser_variablestate(LHSVM* vm, LHSLoadF* loadf, int global)
{
    lhsparser_nexttoken(vm, loadf);
    if (lhsparser_castlex(loadf)->token.t != LHS_TOKENIDENTIFIER)
    {
        lhserr_syntaxerr
        (
            vm, 
            loadf,
            "solving variable '%s'.",
            lhsparser_castlex(loadf)->token.buf.data
        );
    }

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

    LHSVariable* variable = lhsframe_insertvariable
    (
        vm, 
        lhsframe_castcurframe(vm), 
        loadf->line, 
        loadf->column,
        global
    );

    lhsparser_nexttoken(vm, loadf);
    if (lhsparser_castlex(loadf)->token.t != '=')
    {
        return LHS_TRUE;
    }

    return lhsparser_directmov(vm, loadf, variable);
}

int lhsparser_ifprefix(LHSVM* vm, LHSIfState* state)
{
    lhscode_unaryl(vm, OP_JMPF, 0);
    state->branch->pos = vm->code.usize;
    lhsframe_enterchunk(vm, lhsframe_castcurframe(vm), state->loadf);
    lhsparser_statement(vm, state->loadf);
    lhsframe_leavechunk(vm, lhsframe_castcurframe(vm), state->loadf);
    lhscode_unaryl(vm, OP_JMP, 0);
    state->finish->pos = vm->code.usize;
    return LHS_TRUE;
}

int lhsparser_ifsuffix(LHSVM* vm, LHSIfState* state)
{
    state->branch->len = vm->code.usize - state->branch->pos;
    lhsframe_enterchunk(vm, lhsframe_castcurframe(vm), state->loadf);
    lhsparser_statement(vm, state->loadf);
    lhsframe_leavechunk(vm, lhsframe_castcurframe(vm), state->loadf);
    state->finish->len = vm->code.usize - state->finish->pos;
    return LHS_TRUE;
}

int lhsparser_ifsolve(LHSVM* vm, LHSIfState* state)
{
    lhsparser_lookaheadandcheck(vm, state->loadf, '(', "if");
    lhsparser_exprstate(vm, state->loadf);
    lhsparser_checkandnexttoken(vm, state->loadf, '{', "if");    
    lhsparser_ifprefix(vm, state);
    lhsparser_checkandnexttoken(vm, state->loadf, '}', "if");

    do
    {
        if (lhsparser_castlex(state->loadf)->token.t != LHS_TOKENELSE)
        {
            break;
        }

        lhsparser_lookaheadtoken(vm, state->loadf);
        if (lhsparser_castlex(state->loadf)->lookahead.t != LHS_TOKENIF)
        {
            break;
        }

        lhsparser_nexttoken(vm, state->loadf);
        state->branch->len = vm->code.usize - state->branch->pos;
        return lhsparser_ifstate(vm, state->loadf);
    } while (LHS_FALSE);

    if (lhsparser_castlex(state->loadf)->token.t == LHS_TOKENELSE)
    {
        lhsparser_nexttokenandcheck(vm, state->loadf, '{', "if");
        lhsparser_nexttoken(vm, state->loadf);
        lhsparser_ifsuffix(vm, state);
        lhsparser_checkandnexttoken(vm, state->loadf, '}', "if");
    }

    return LHS_TRUE;
}

static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf)
{
    LHSIfState ifstate;
    lhsparser_initifstate(vm, &ifstate, loadf);

    if (lhserr_protectedcall
    (
        vm, 
        lhsparser_ifsolve,
        &ifstate
    ))
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);

        lhsparser_uninitifstate(vm, &ifstate);
        lhserr_throw(vm, "");
    }

    lhsparser_uninitifstate(vm, &ifstate);
    return LHS_TRUE;
}

static int lhsparser_statement(LHSVM* vm, LHSLoadF* loadf)
{
    while (!lhsparser_ischunkfollow(loadf))
    {
        lhsparser_statesolve(vm, loadf);
    }

    return LHS_TRUE;
}

static int lhsparser_statesolve(LHSVM* vm, LHSLoadF* loadf)
{
    switch (lhsparser_castlex(loadf)->token.t)
    {
    case LHS_TOKENLOCAL:
    {
        lhsparser_variablestate(vm, loadf, LHS_FALSE);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENGLOBAL:
    {
        lhsparser_variablestate(vm, loadf, LHS_TRUE);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENIDENTIFIER:
    {
        lhsparser_movstate(vm, loadf);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENIF:
    {
        lhsparser_ifstate(vm, loadf);
        break;
    }
    case LHS_TOKENEOF:
    {
        break;
    }
    default:
    {
        lhserr_syntaxerr
        (
            vm, 
            loadf,
            "unknown token '%s'.",
            lhsparser_castlex(loadf)->token.buf.data
        );
    }
    }
    return LHS_TRUE;
}

static int lhsparser_initstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsloadf_getc(loadf);
    lhsparser_nexttoken(vm, loadf);
    lhsparser_statement(vm, loadf);
    lhsparser_statesolve(vm, loadf);
    return LHS_TRUE;
}

int lhsparser_dofile(LHSVM* vm, const char* fname)
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
        lhsparser_lexicalsolve(vm, &lexical);
        lhscode_dmpcode(vm);
    }
    
    lhsparser_uninitlexical(vm, &loadf);
    lhsloadf_uninit(vm, &loadf);
    return LHS_TRUE;
}
