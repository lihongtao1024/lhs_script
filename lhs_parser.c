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

#define lhsparser_castlex(lf)                                       \
((LHSLexical*)(lf)->lexical)

#define lhsparser_issymbol(t)                                       \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) > LHS_TOKENSYMBOLBEGIN &&                                     \
  (t) < LHS_TOKENSYMBOLEND) :                                       \
(lhsloadf_symbol[(t)] > SYMBOL_BEGIN &&                           \
 lhsloadf_symbol[(t)] < SYMBOL_END))

#define lhsparser_istokensymbol(lf)                                 \
lhsparser_issymbol(lhsparser_castlex(lf)->token.t)

#define lhsparser_isunarysymbol(s)                                  \
((s) == SYMBOL_NOT || (s) == SYMBOL_BNOT || (s) == SYMBOL_MINUS)

#define lhsparser_islbracket(lf)                                    \
(lhsparser_castlex(lf)->token.t == '(')

#define lhsparser_truncate(t)                                       \
(((t) & ~UCHAR_MAX) ?                                               \
 ((t) & UCHAR_MAX) :                                                \
 lhsloadf_symbol[t])

#define lhsparser_isfininsh(lf)                                     \
(lhsparser_castlex(lf)->token.t == LHS_TOKENEOF)

typedef struct LHSExprState
{
    struct LHSExprState* prev;
    int symbol;
} LHSExprState;

typedef struct LHSIfState
{
    LHSLoadF* loadf;
    LHSJmp* branch;
    LHSJmp* finish;
} LHSIfState;

static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_exprstate(LHSVM* vm, LHSLoadF* loadf);
static int lhsparser_statement(LHSVM* vm, LHSLoadF* loadf);

static const char* reserveds[] =
{
    "", "set", "var", "function", "for", "while",
    "if", "else", "switch", "case", "default", 
    "break", "continue", "true", "false", "return"
};

static const char lhsparser_priority[][SYMBOL_END] =
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

static int lhsparser_jmpsolve(LHSLexical* lex, LHSJmp* jmp, LHSVM* vm)
{
    long long* addr = (long long*)(vm->code.data + jmp->pos - sizeof(long long));
    *addr = (long long)vm->code.data + jmp->pos + jmp->len;
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
    lhsparser_castlex(loadf)->lookahead.t = lhsparser_nextlexical
    (
        vm, 
        loadf, 
        &lhsparser_castlex(loadf)->lookahead.buf
    );
    return LHS_TRUE;
}

static int lhsparser_resetexprstate(LHSVM* vm, LHSExprState* state)
{
    state->prev = 0;
    state->symbol = SYMBOL_NONE;
    return LHS_TRUE;
}

static int lhsparser_resetifstate(LHSVM* vm, LHSIfState* state, LHSLoadF* loadf)
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

static int lhsparser_exprsolve(LHSVM* vm, LHSLoadF* loadf, LHSExprState* state)
{
    if (!state)
    {
        return LHS_FALSE;
    }

    switch (lhsparser_priority[state->prev->symbol][state->symbol])
    {
    case L:
    {
        lhscode_binary(vm, state->prev->symbol, LHS_MARKSTACK, -2, LHS_MARKSTACK, -1);
        state->prev = state->prev->prev;
        return lhsparser_exprsolve(vm, loadf, state);
    }
    case E:
    {
        state = state->prev->prev;
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

static int lhsparser_exprfactor(LHSVM* vm, LHSLoadF* loadf)
{
    /*exprfactor -> [{op_unary}] LHS_TOKENINTEGER    | 
                    [{op_unary}] LHS_TOKENNUMBER     | 
                    [{op_unary}] LHS_TOKENTRUE       | 
                    [{op_unary}] LHS_TOKENFALSE      | 
                    [{op_unary}] LHS_TOKENIDENTIFIER |
                    [{op_unary}] '('exprstate')'*/
    LHSToken* token = &lhsparser_castlex(loadf)->token;
    switch (token->t)
    {
    case LHS_TOKENINTEGER:
    {
        lhscode_unaryl(vm, OP_PUSH, atoll(token->buf.data));
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENNUMBER:
    {
        lhscode_unaryf(vm, OP_PUSH, atof(token->buf.data));
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENSTRING:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
        LHSVariable *constant = lhsvm_insertconstant(vm);
        lhscode_unaryi(vm, OP_PUSH, constant->mark, constant->index);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENTRUE:
    {
        lhscode_unaryb(vm, OP_PUSH, 1);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENFALSE:
    {
        lhscode_unaryb(vm, OP_PUSH, 0);
        lhsparser_nexttoken(vm, loadf);
        break;
    }
    case LHS_TOKENIDENTIFIER:
    {
        lhsvm_pushlstring(vm, token->buf.data, token->buf.usize);
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
        lhscode_unaryi(vm, OP_PUSH, variable->mark, variable->index);
        lhsparser_nexttoken(vm, loadf);
        break; 
    }
    case '(':
    {
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
            lhsparser_exprfactor(vm, loadf);
            lhscode_unaryi(vm, op, LHS_MARKSTACK, -1);
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
    lhsparser_exprfactor(vm, loadf);
    
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
        state->symbol = SYMBOL_NONE;
    }
    else
    {
        state->symbol = lhsparser_truncate(token->t);
        lhsparser_nexttoken(vm, loadf); 
    }

    return LHS_TRUE;
}

static int lhsparser_subexpr(LHSVM* vm, LHSLoadF* loadf, LHSExprState* ostate)
{
    /*subexpr -> {exprchain}*/
    LHSExprState nstate;
    lhsparser_resetexprstate(vm, &nstate);
    nstate.prev = ostate;
    lhsparser_exprchain(vm, loadf, &nstate);

    if (lhsparser_exprsolve(vm, loadf, &nstate))
    {
        return lhsparser_subexpr(vm, loadf, &nstate);
    }

    return LHS_TRUE;
}

static int lhsparser_exprstate(LHSVM* vm, LHSLoadF* loadf)
{ 
    /*expr -> {subexpr}*/
    LHSExprState state;
    lhsparser_resetexprstate(vm, &state);
    lhsparser_subexpr(vm, loadf, &state);
    return LHS_TRUE;
}

static int lhsparser_localstate(LHSVM* vm, LHSLoadF* loadf)
{
    /*var -> identifier ['=' exprstate] {',' identifier ['=' exprstate]}*/
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

        LHSVariable* variable = lhsframe_insertvariable
        (
            vm, 
            lhsframe_castcurframe(vm), 
            loadf->line, 
            loadf->column,
            LHS_FALSE
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
    /*set -> identifier ['=' exprstate] {',' identifier ['=' exprstate]}*/
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

        LHSVariable* variable = lhsframe_insertvariable
        (
            vm, 
            lhsframe_castcurframe(vm), 
            loadf->line, 
            loadf->column,
            LHS_TRUE
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
    lhsparser_nexttokenandcheck(vm, state->loadf, '(', "if", "(");
    lhsparser_exprstate(vm, state->loadf);
    lhsparser_checkandnexttoken(vm, state->loadf, '{', "if", "{");
    lhsparser_ifprefix(vm, state);
    lhsparser_checkandnexttoken(vm, state->loadf, '}', "if", "}");

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
        lhsparser_nexttokenandcheck(vm, state->loadf, '{', "if", "{");
        lhsparser_nexttoken(vm, state->loadf);
        lhsparser_ifsuffix(vm, state);
        lhsparser_checkandnexttoken(vm, state->loadf, '}', "if", "}");
    }

    return LHS_TRUE;
}

static int lhsparser_ifstate(LHSVM* vm, LHSLoadF* loadf)
{
    LHSIfState ifstate;
    lhsparser_resetifstate(vm, &ifstate, loadf);

    if (lhserr_protectedcall
    (
        vm, 
        lhsparser_ifsolve,
        &ifstate
    ))
    {
        lhserr_msg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);

        lhserr_throw(vm, "");
    }

    return LHS_TRUE;
}

static int lhsparser_statement(LHSVM* vm, LHSLoadF* loadf)
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
    case LHS_TOKENEOF:
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

static int lhsparser_solvestate(LHSVM* vm, LHSLoadF* loadf)
{
    while (!lhsparser_isfininsh(loadf))
    {
        lhsparser_statement(vm, loadf);
    }

    return LHS_TRUE;
}

static int lhsparser_initstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsloadf_getc(loadf);
    lhsparser_nexttoken(vm, loadf);
    lhsparser_solvestate(vm, loadf);
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
