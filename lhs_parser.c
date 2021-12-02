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

#define lhsparser_isassignment(lf)                                  \
((lf)->lexical->token == '=')

#define lhsparser_isexprsymbol(t)                                   \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) > LHS_TOKENSYMBOLBEGIN &&                                     \
  (t) < LHS_TOKENSYMBOLEND) :                                       \
(lhsloadf_symbolid[(t)] > SYMBOL_BEGIN &&                           \
 lhsloadf_symbolid[(t)] < SYMBOL_END))

#define lhsparser_iscurexprsymbol(lf)                               \
lhsparser_isexprsymbol((lf)->lexical->token)

#define lhsparser_isnextexprsymbol(lf)                              \
lhsparser_isexprsymbol((lf)->lexical->lookahead)

#define lhsparser_isunarysymbol(s)                                  \
((s) == SYMBOL_NOT || (s) == SYMBOL_BNOT || (s) == SYMBOL_MINUS)

#define lhsparser_truncate(t)                                       \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) & UCHAR_MAX) :                                                \
 lhsloadf_symbolid[t])

typedef struct LHSExprState
{
    int read;
    LHSLoadF* loadf;
    LHSSTRBUF symbols;
} LHSExprState;

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

static int lhsparser_initmainframe(LHSVM* vm, const char* fname)
{
    vm->mainframe = lhsmem_newgcobject
    (
        vm, 
        sizeof(LHSFrame), 
        LHS_TGCFRAME
    );

    lhsframe_init(vm, vm->mainframe);
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

static int lhsparser_initlexical(LHSVM* vm, LHSLoadF* loadf, LHSLexical* lex)
{
    lex->token = LHS_TOKENEOF;
    lex->lookahead = LHS_TOKENEOF;
    loadf->lexical = lex;

    lhsbuf_init(vm, &lex->buf);
    return LHS_TRUE;
}

static int lhsparser_uninitlexical(LHSVM* vm, LHSLoadF* loadf)
{
    lhsbuf_uninit(vm, &loadf->lexical->buf);
    loadf->lexical = 0;
    return LHS_TRUE;
}

static int lhsparser_nextlexical(LHSVM* vm, LHSLoadF* loadf)
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
            lhsloadf_savedigital(vm, loadf, &is_double);
            return is_double ? LHS_TOKENNUMBER : LHS_TOKENINTEGER;
        }
        case '/':
        {
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
            lhsloadf_getc(loadf);
            if (loadf->current == '&')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENLOGICAND;
            }
            return '&';
        }
        case '|':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '|')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENLOGICOR;
            }
            return '|';
        }
        case '<':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '<')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENBLSHIFT;
            }
            return '<';
        }
        case '>':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '>')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENBRSHIFT;
            }
            return '>';
        }
        case '=':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENEQUAL;
            }
            return '=';
        }
        case '!':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENNOTEQUAL;
            }
            return '!';
        }
        case '-':
        {
            lhsloadf_getc(loadf);
            if (lhsparser_isassignment(loadf) ||
                lhsparser_iscurexprsymbol(loadf))
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
            int token = loadf->current;
            lhsloadf_getc(loadf);
            return token;
        }
        case '"':
        {
            lhsloadf_savestring(vm, loadf);
            return LHS_TOKENSTRING;
        }
        default:
        {
            if (lhsloadf_isletter(loadf))
            {
                lhsloadf_saveidentifier(vm, loadf);                
                if (lhsbuf_isshort(vm, &loadf->lexical->buf))
                {
                    LHSString* str = lhsvm_findshort
                    (
                        vm,
                        loadf->lexical->buf.data,
                        loadf->lexical->buf.usize
                    );
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
                loadf->lexical->buf.data
            );
        }
        }
    }
}

static int lhsparser_nexttoken(LHSVM* vm, LHSLoadF* loadf)
{
    if (loadf->lexical->lookahead != LHS_TOKENEOF)
    {
        loadf->lexical->token = loadf->lexical->lookahead;
        loadf->lexical->lookahead = LHS_TOKENEOF;
        return LHS_TRUE;
    }

    loadf->lexical->token = lhsparser_nextlexical(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_lookaheadtoken(LHSVM* vm, LHSLoadF* loadf)
{
    loadf->lexical->lookahead = lhsparser_nextlexical(vm, loadf);
    return LHS_TRUE;
}

static int lhsparser_initexprstate(LHSVM* vm, LHSExprState* state, 
    LHSLoadF* loadf)
{
    state->read = LHS_TRUE;
    state->loadf = loadf;
    lhsbuf_init(vm, &state->symbols);
    lhsbuf_pushchar(vm, &state->symbols, SYMBOL_NONE);
    return LHS_TRUE;
}

static void lhsparser_uninitexprstate(LHSVM* vm, LHSExprState* state)
{
    lhsbuf_uninit(vm, &state->symbols);
}

static int lhsparser_exprrecursive(LHSVM* vm, LHSExprState* state, char nsymbol)
{
    int result = LHS_FALSE;
    if (lhsbuf_isempty(vm, &state->symbols))
    {
        return result;
    }

    char osymbol;
    lhsbuf_topchar(vm, &state->symbols, &osymbol);

    switch (lhsparser_prioritytable[osymbol][nsymbol])
    {
    case G:
    {
        lhsbuf_pushchar(vm, &state->symbols, nsymbol);
        break;
    }
    case L:
    {
        if (lhsparser_isunarysymbol(osymbol))
        {
            LHSCode c1;
            c1.mark = LHS_MARKSTACK;
            c1.code.index = -1;
            lhscode_unaryexpr(vm, osymbol, &c1);
        }
        else
        {            
            LHSCode c1;
            c1.mark = LHS_MARKSTACK;
            c1.code.index = -2;
            LHSCode c2;
            c2.mark = LHS_MARKSTACK;
            c2.code.index = -1;
            lhscode_binaryexpr(vm, osymbol, &c1, &c2);
        }

        lhsbuf_popchar(vm, &state->symbols, &osymbol);
        result = LHS_TRUE;
        break;
    }
    case E:
    {
        lhsbuf_popchar(vm, &state->symbols, &osymbol);
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
    LHSCode c; int token = LHS_TOKENEOF;
    while (!lhsbuf_isempty(vm, &state->symbols))
    {
        if (state->read)
        {
            lhsparser_nexttoken(vm, state->loadf);
            token = state->loadf->lexical->token;

            if (token == LHS_TOKENINTEGER)
            {
                c.mark = LHS_MARKINTEGER;
                c.code.i = atoll(state->loadf->lexical->buf.data);
                lhscode_unaryexpr(vm, OP_PUSH, &c);

                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_isnextexprsymbol(state->loadf);
                continue;
            }
            
            if (token == LHS_TOKENNUMBER)
            {
                c.mark = LHS_MARKNUMBER;
                c.code.n = atof(state->loadf->lexical->buf.data);
                lhscode_unaryexpr(vm, OP_PUSH, &c);

                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_isnextexprsymbol(state->loadf);
                continue;
            }

            if (token == LHS_TOKENSTRING)
            {
                lhsvm_pushlstring(vm, state->loadf->lexical->buf.data,
                    state->loadf->lexical->buf.usize);
                LHSVariable* constvar = lhsvm_insertconstant(vm);
                c.mark = constvar->mark;
                c.code.index = constvar->index;
                lhscode_unaryexpr(vm, OP_PUSH, &c);

                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_isnextexprsymbol(state->loadf);
                continue;
            }

            if (token == LHS_TOKENIDENTIFIER)
            {
                lhsvm_pushlstring(vm, state->loadf->lexical->buf.data,
                    state->loadf->lexical->buf.usize);
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
                        state->loadf->lexical->buf.data
                    );
                }
                c.mark = var->mark;
                c.code.index = var->index;
                lhscode_unaryexpr(vm, OP_PUSH, &c);

                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_isnextexprsymbol(state->loadf);
                continue;
            }

            if (token == LHS_TOKENTRUE ||
                token == LHS_TOKENFALSE)
            {
                c.mark = LHS_MARKBOOLEAN;
                c.code.b = state->loadf->lexical->token == LHS_TOKENTRUE ? 1 : 0;
                lhscode_unaryexpr(vm, OP_PUSH, &c);

                lhsparser_lookaheadtoken(vm, state->loadf);
                state->read = lhsparser_isnextexprsymbol(state->loadf);
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

static int lhsparser_directmov(LHSVM* vm, LHSLoadF* loadf, LHSVariable* variable)
{
    lhsparser_nexttoken(vm, loadf);
    lhsparser_exprstate(vm, loadf);
    
    LHSCode c1;
    c1.mark = variable->mark;
    c1.code.index = variable->index;
    LHSCode c2;
    c2.mark = LHS_MARKSTACK;
    c2.code.index = -1;
    lhscode_binaryexpr(vm, OP_MOVE, &c1, &c2);

    return LHS_TRUE;
}

static int lhsparser_movstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsvm_pushlstring
    (
        vm, 
        loadf->lexical->buf.data, 
        loadf->lexical->buf.usize
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
            loadf->lexical->buf.data
        );
    }

    return lhsparser_directmov(vm, loadf, variable);
}

static int lhsparser_variablestate(LHSVM* vm, LHSLoadF* loadf, int global)
{
    lhsparser_nexttoken(vm, loadf);
    if (loadf->lexical->token != LHS_TOKENIDENTIFIER)
    {
        lhserr_syntaxerr
        (
            vm, 
            loadf,
            "solving variable '%s'.",
            loadf->lexical->buf.data
        );
    }

    lhsvm_pushlstring
    (
        vm, 
        loadf->lexical->buf.data, 
        loadf->lexical->buf.usize
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
            loadf->lexical->buf.data
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

    lhsparser_lookaheadtoken(vm, loadf);
    if (loadf->lexical->lookahead != '=')
    {
        return LHS_TRUE;
    }

    return lhsparser_directmov(vm, loadf, variable);
}

int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsparser_exprstate(vm, loadf);

    if (loadf->lexical->token != '{')
    {
        lhserr_syntaxerr
        (
            vm, 
            loadf,
            "unexpected if definition '%s'.",
            loadf->lexical->buf.data
        );
    }
    int a = 10;
    return LHS_TRUE;
}

static int lhsparser_preparestate(LHSVM* vm, LHSLoadF* loadf)
{
    while (loadf->lexical->token != LHS_TOKENEOF)
    {
        switch (loadf->lexical->token)
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
        case LHS_TOKENIF:
        {
            lhsparser_ifstate(vm, loadf);
            lhsparser_nexttoken(vm, loadf);
            break;
        }
        case LHS_TOKENIDENTIFIER:
        {
            lhsparser_lookaheadtoken(vm, loadf);
            if (loadf->lexical->lookahead == '=')
            {
                lhsparser_movstate(vm, loadf);
                lhsparser_nexttoken(vm, loadf);
            }
            else
            {
                lhserr_syntaxerr
                (
                    vm, 
                    loadf,
                    "illegal identifier '%s'.",
                    loadf->lexical->buf.data
                );
            }
            break;
        }
        default:
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf,
                "unknown token '%s'.",
                loadf->lexical->buf.data
            );
        }
        }
    }

    return LHS_TRUE;
}

static int lhsparser_initstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsloadf_getc(loadf);
    lhsparser_nexttoken(vm, loadf);
    lhsparser_preparestate(vm, loadf);
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

    if (!lhsparser_initmainframe(vm, fname))
    {
        lhsloadf_uninit(vm, &loadf);
        return LHS_FALSE;
    }

    LHSLexical lexical;
    lhsparser_initlexical(vm, &loadf, &lexical);

    if (lhserr_protectedcall
    (
        vm, 
        lhsparser_initstate, 
        &loadf
    ))
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
    }
    
    lhsparser_uninitlexical(vm, &loadf);
    lhsloadf_uninit(vm, &loadf);
    return LHS_TRUE;
}
