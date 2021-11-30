#include "lhs_parser.h"
#include "lhs_load.h"
#include "lhs_value.h"
#include "lhs_link.h"
#include "lhs_frame.h"
#include "lhs_code.h"
#include "lhs_error.h"

#define G (1)
#define L (0)
#define N (-1)
#define E (2)

#define lhsparser_isexprvalue(lf) \
((lf)->lexical->token == LHS_TOKENNUMBER  || \
 (lf)->lexical->token == LHS_TOKENINTEGER || \
 (lf)->lexical->token == LHS_TOKENSTRING  || \
 (lf)->lexical->token == LHS_TOKENTRUE    || \
 (lf)->lexical->token == LHS_TOKENFALSE   || \
 (lf)->lexical->token == LHS_TOKENIDENTIFY)

#define lhsparser_isexprsymbol(lf) \
(((lf)->lexical->token <= UCHAR_MAX) ? \
(lhsloadf_symbols[(lf)->lexical->token] > SYMBOL_BEGIN && \
 lhsloadf_symbols[(lf)->lexical->token] < SYMBOL_END) : \
(((lf)->lexical->token & ~UCHAR_MAX) > LHS_TOKENSYMBOLBEGIN && \
 ((lf)->lexical->token & ~UCHAR_MAX) < LHS_TOKENSYMBOLEND))

#define lhsparser_isunarysymbol(s) \
((s) == SYMBOL_NOT || (s) == SYMBOL_BNOT)

#define lhsparser_truncate(t) \
((t) <= UCHAR_MAX ? lhsloadf_symbols[t] : ((t) & ~UCHAR_MAX))

static const char* tokens[] =
{
    "", "set", "var", "function", "for", "while",
    "if", "else", "switch", "case", "default", 
    "break", "continue", "true", "false", "return",
    "<name>", "<integer>", "<number>", "<string>"
};

static const char symbols_priority[][SYMBOL_END] =
{
     /*N/A, +, -, *, /, %, &, |, ^, <, >,==,!=,>=,<=,&&,||,<<,>>, !, ~, (, ),*/
/*N/A*/{ E, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, N,},
/*+*/  { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, L,},
/*-*/  { L, L, L, G, G, G, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, L,},
/***/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, L,},
/*/*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, L,},
/*%*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, G, L,},
/*&*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*|*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*^*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*<*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*>*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*==*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*!=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*>=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*<=*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*&&*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*||*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*<<*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*>>*/ { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, G, L,},
/*!*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, L,},
/*~*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, G, L,},
/*(*/  { N, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, E,},
/*)*/  { L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,},
};

static int lhsparser_initreserved(LHSVM* vm)
{
    for (int i = 0; i < _countof(tokens); ++i)
    {
        LHSString* str = lhsvalue_caststring
        (
            lhsmem_newgcstring
            (
                vm, 
                (void*)tokens[i], 
                i + LHS_TOKENRESERVEDBEGIN
            )
        );
        lhshash_insert(vm, &vm->shortstrhash, str, &str->hash);
    }

    return true;
}

static int lhsparser_initmainframe(LHSVM* vm, const char* fname)
{
    vm->mainframe = lhsmem_newgcobject
    (
        vm, 
        sizeof(LHSFrame), 
        LHS_TGCFRAME
    );

    lhsframe_init(vm, vm->mainframe, false);
    vm->currentframe = vm->mainframe;
    
    lhsvm_pushstring(vm, fname);
    LHSVariable* variable = lhsframe_insertvariable
    (
        vm, 
        lhsframe_castmainframe(vm), 
        0, 
        0
    );
    LHSValue *value = lhsvalue_castvalue
    (
        lhsvector_at
        (
            vm, 
            &lhsframe_castmainframe(vm)->localvalues, 
            variable->index
        )
    );
    value->type = LHS_TGC;
    value->gc = lhsgc_castgc(variable->name);
    return true;
}

static int lhsparser_enterlexical(LHSVM* vm, LHSLoadF* loadf, LHSLexical* lex)
{
    lex->token = LHS_TOKENEOF;
    lex->lookahead = LHS_TOKENEOF;
    lex->column = loadf->column;
    lex->line = loadf->line;
    loadf->lexical = lex;

    lhsbuf_init(vm, &lex->buf);
    return true;
}

static int lhsparser_leavelexical(LHSVM* vm, LHSLoadF* loadf)
{
    lhsbuf_uninit(vm, &loadf->lexical->buf);
    loadf->lexical = 0;
    return true;
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
            int is_double = false;
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
        case '+':
        case '-':
        case '*':
        case '%':
        case '~':
        case '^':
        case '(':
        case ')':
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
                return LHS_TOKENIDENTIFY;
            }

            if (loadf->current == LHS_TOKENEOF)
            {
                return LHS_TOKENEOF;
            }

            lhsexecute_symbolerr
            (
                vm, 
                "undefined token found.",
                lhsframe_name(vm, lhsframe_castmainframe(vm)),
                loadf->line,
                loadf->column
            );
            return LHS_TOKENEOF;
        }
        }
    }
}

static int lhsparser_nexttoken(LHSVM* vm, LHSLoadF* loadf)
{
    loadf->lexical->line = loadf->line;
    loadf->lexical->column = loadf->column;

    if (loadf->lexical->lookahead != LHS_TOKENEOF)
    {
        loadf->lexical->token = loadf->lexical->lookahead;
        loadf->lexical->lookahead = LHS_TOKENEOF;
        return true;
    }

    loadf->lexical->token = lhsparser_nextlexical(vm, loadf);
    return true;
}

static int lhsparser_lookaheadtoken(LHSVM* vm, LHSLoadF* loadf)
{
    loadf->lexical->lookahead = lhsparser_nextlexical(vm, loadf);
    return true;
}

static int lhsparser_expressionproc(LHSVM* vm, LHSSTRBUF* symbols, 
    LHSVector* values, char nsymbol)
{
    int is_continue = false;
    if (lhsbuf_isempty(vm, symbols))
    {
        return is_continue;
    }

    char osymbol;
    lhsbuf_topchar(vm, symbols, &osymbol);
    switch (symbols_priority[osymbol][nsymbol])
    {
    case G:
    {
        lhsbuf_pushchar(vm, symbols, nsymbol);
        break;
    }
    case L:
    {
        is_continue = true;
        if (lhsparser_isunarysymbol(nsymbol))
        {
            LHSCode* code = (LHSCode*)lhsvector_at
            (
                vm,
                values,
                lhsvector_length(vm, values) - 1
            );
            lhscode_unaryexpr(vm, osymbol, code);
        }
        else
        {
            /*LHSCode* code1 = (LHSCode*)lhsvector_at
            (
                vm,
                values,
                lhsvector_length(vm, values) - 2
            );
            if (code1->mark != LHS_MARKNONE)
            {
                lhscode_unaryexpr(vm, OP_PUSH, code1);
            }

            LHSCode* code2 = (LHSCode*)lhsvector_at
            (
                vm,
                values,
                lhsvector_length(vm, values) - 1
            );
            if (code2->mark != LHS_MARKNONE)
            {
                lhscode_unaryexpr(vm, OP_PUSH, code2);
            }*/
            
            LHSCode s1;
            s1.mark = LHS_MARKSTACK;
            s1.code.index = -2;
            LHSCode s2;
            s2.mark = LHS_MARKSTACK;
            s2.code.index = -1;
            lhscode_binaryexpr(vm, osymbol, &s1, &s2);
            lhsvector_pop(vm, values, 2);

            LHSCode s3;
            s3.mark = LHS_MARKNONE;
            lhsvector_push(vm, values, &s3, 0);
        }

        lhsbuf_popchar(vm, symbols, &osymbol);
        break;
    }
    case E:
    {
        lhsbuf_popchar(vm, symbols, &osymbol);
        break;
    }
    default:
    {
        lhsbuf_uninit(vm, symbols);
        lhsvector_uninit(vm, values);
        lhsexecute_protectederr(vm, "illegal expression.");
        break;
    }
    }

    return is_continue;
}

static int lhsparser_expressionstate(LHSVM* vm, LHSLoadF* loadf)
{    
    LHSSTRBUF symbols;
    lhsbuf_init(vm, &symbols);
    lhsbuf_pushchar(vm, &symbols, SYMBOL_NONE);

    LHSVector values;
    lhsvector_init(vm, &values, sizeof(LHSCode));

    while (!lhsbuf_isempty(vm, &symbols))
    {
        lhsparser_nexttoken(vm, loadf);
        if (loadf->lexical->token == LHS_TOKENINTEGER)
        {
            LHSCode code;
            code.mark = LHS_MARKINTEGER;
            code.code.i = atoll(loadf->lexical->buf.data);
            lhscode_unaryexpr(vm, OP_PUSH, &code);
            lhsvector_push(vm, &values, &code, 0);
        }
        else if (loadf->lexical->token == LHS_TOKENNUMBER)
        {
            LHSCode code;
            code.mark = LHS_MARKNUMBER;
            code.code.n = atof(loadf->lexical->buf.data);
            lhscode_unaryexpr(vm, OP_PUSH, &code);
            lhsvector_push(vm, &values, &code, 0);
        }
        else if (loadf->lexical->token == LHS_TOKENSTRING)
        {
            lhsvm_pushlstring(vm, loadf->lexical->buf.data,
                loadf->lexical->buf.usize);
            LHSVariable* constvar = lhsvm_insertconstant(vm);
            LHSCode code;
            code.mark = constvar->mark;
            code.code.index = constvar->index;
            lhscode_unaryexpr(vm, OP_PUSH, &code);
            lhsvector_push(vm, &values, &code, 0);
        }
        else if (loadf->lexical->token == LHS_TOKENIDENTIFY)
        {
            lhsvm_pushlstring(vm, loadf->lexical->buf.data,
                loadf->lexical->buf.usize);
            LHSVariable* var = lhsframe_getvariable
            (
                vm, 
                lhsframe_castcurframe(vm)
            );
            LHSCode code;
            code.mark = var->mark;
            code.code.index = var->index;
            lhscode_unaryexpr(vm, OP_PUSH, &code);
            lhsvector_push(vm, &values, &code, 0);
        }
        else if (loadf->lexical->token == LHS_TOKENTRUE ||
                 loadf->lexical->token == LHS_TOKENFALSE)
        {
            LHSCode code;
            code.mark = LHS_MARKBOOLEAN;
            code.code.b = loadf->lexical->token == LHS_TOKENTRUE ? 1 : 0;
            lhscode_unaryexpr(vm, OP_PUSH, &code);
            lhsvector_push(vm, &values, &code, 0);
        }
        else
        {
            char symbol = SYMBOL_NONE;
            if (lhsparser_isexprsymbol(loadf))
            {
                symbol = lhsparser_truncate(loadf->lexical->token);
            }

            while (lhsparser_expressionproc(vm, &symbols, &values, symbol));
        }
    }

    lhsbuf_uninit(vm, &symbols);
    lhsvector_uninit(vm, &values);
    return true;
}

static int lhsparser_movstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsparser_nexttoken(vm, loadf);
    if (loadf->lexical->token != LHS_TOKENIDENTIFY)
    {
        lhsexecute_symbolerr
        (
            vm, 
            "illegal variable '%s' declaration.",
            lhsframe_name(vm, lhsframe_castmainframe(vm)),
            loadf->line,
            loadf->column,
            loadf->lexical->buf.data
        );
        return false;
    }

    lhsvm_pushlstring
    (
        vm, 
        loadf->lexical->buf.data, 
        loadf->lexical->buf.usize
    );
    LHSVariable *variable = lhsframe_insertvariable
    (
        vm, 
        lhsframe_castcurframe(vm), 
        loadf->line, 
        loadf->column
    );

    lhsparser_lookaheadtoken(vm, loadf);
    if (loadf->lexical->lookahead != '=')
    {
        return true;
    }

    lhsparser_nexttoken(vm, loadf);
    lhsparser_expressionstate(vm, loadf);
    
    LHSCode c1;
    c1.mark = variable->mark;
    c1.code.index = variable->index;
    LHSCode c2;
    c2.mark = LHS_MARKSTACK;
    c2.code.index = -1;
    lhscode_binaryexpr(vm, OP_MOVE, &c1, &c2);

    return true;
}

static int lhsparser_procstate(LHSVM* vm, LHSLoadF* loadf)
{
    while (loadf->lexical->token != LHS_TOKENEOF)
    {
        switch (loadf->lexical->token)
        {
        case LHS_TOKENGLOBAL:
        case LHS_TOKENLOCAL:
        {
            lhsparser_movstate(vm, loadf);
            break;
        }
        default:
        {
            lhsparser_nexttoken(vm, loadf);
            break;
        }
        }
    }

    return true;
}

static int lhsparser_initstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsloadf_getc(loadf);
    lhsparser_nexttoken(vm, loadf);
    lhsparser_procstate(vm, loadf);
    return true;
}

int lhsparser_dofile(LHSVM* vm, const char* fname)
{    
    if (!lhsparser_initreserved(vm))
    {
        return false;
    }

    LHSLoadF loadf;
    if (!lhsloadf_init(vm, &loadf, fname))
    {
        return false;
    }

    if (!lhsparser_initmainframe(vm, fname))
    {
        lhsloadf_uninit(vm, &loadf);
        return false;
    }

    LHSLexical lexical;
    lhsparser_enterlexical(vm, &loadf, &lexical);

    if (lhsexecute_protectedrun(vm, lhsparser_initstate, &loadf))
    {
        lhs_errmsg(lhsvm_tostring(vm, -1));
        lhsvm_pop(vm, 2);
    }
    
    lhsparser_leavelexical(vm, &loadf);
    lhsloadf_uninit(vm, &loadf);
    return true;
}
