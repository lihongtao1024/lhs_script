#include "lhs_parser.h"
#include "lhs_assert.h"
#include "lhs_load.h"
#include "lhs_value.h"
#include "lhs_link.h"
#include "lhs_frame.h"
#include "lhs_error.h"

#define G 1
#define L 0
#define N -1

static const char* tokens[] =
{
    "", "set", "var", "function", "for", "while",
    "if", "else", "switch", "case", "default", 
    "break", "continue", "true", "false", "return",
    "<name>", "<integer>", "<number>", "<string>"
};

static const char symbols_priority[][SYMBOL_END] =
{
     /*N/A,  (, +, -, *, /, ), !, %,  &,  |,  ~,  ^,  <, >,==,!=,>=,<=,+=,-=,*=,/=,
       ++?,--?,&&,||,&=,|=,^=,<<,>>,<<=,>>=, %=,?++,?--*/
/*N/A*/{ L,  G, G, G, G, G, N, G, G,  G,  G,  G,  G,  G, G, G, G, G, G, G, G, G, G,
         G,  G, G, G, G, G, G, G, G,  G,  G,  G,  G,  G},
/*(*/  { N,  G, G, G, G, G, L, G, G,  G,  G,  G,  G,  G, G, G, G, G, G, G, G, G, G,
         G,  G, G, G, G, G, G, G, G,  G,  G,  G,  G,  G},
/*+*/  { L,  G, L, L, G, G, L, G, G,  L,  L,  G,  L,  L, L, L, L, L, L, L, L, L, L,
         G,  G, L, L, N, N, N, L, L,  N,  N,  N,  G,  G},
/*-*/  { L,  G, L, L, G, G, L, G, G,  L,  L,  G,  L,  L, L, L, L, L, L, L, L, L, L,
         G,  G, L, L, N, N, N, L, L,  N,  N,  N,  G,  G},
/***/  { L,  G, L, L, L, L, L, G, L,  L,  L,  G,  L,  L, L, L, L, L, L, L, L, L, L,
         G,  G, L, L, N, N, N, L, L,  N,  N,  N,  G,  G},
/*/*/  { L,  G, L, L, L, L, L, G, L,  L,  L,  G,  L,  L, L, L, L, L, L, L, L, L, L,
         G,  G, L, L, N, N, N, L, L,  N,  N,  N,  G,  G},
/*)*/  { L,},
/*!*/  { L,},
/*%*/  { L,},
/*&*/  { L,},
/*|*/  { L,},
/*~*/  { L,},
/*^*/  { L,},
/*<*/  { L,},
/*>*/  { L,},
/*==*/ { L,},
/*!=*/ { L,},
/*>=*/ { L,},
/*<=*/ { L,},
/*+=*/ { L,},
/*-=*/ { L,},
/**=*/ { L,},
/*/=*/ { L,},
/*++?*/{ L,},
/*--?*/{ L,},
/*&&*/ { L,},
/*||*/ { L,},
/*&=*/ { L,},
/*|=*/ { L,},
/*^=*/ { L,},
/*<<*/ { L,},
/*>>*/ { L,},
/*<<=*/{ L,},
/*>>=*/{ L,},
/*%=*/ { L,},
/*?++*/{ L,},
/*?--*/{ L,},
};

static int lhsparser_initreserved(LHSVM* vm)
{
    lhsassert_trueresult(vm, false);

    for (size_t i = 0; i < _countof(tokens); ++i)
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
        lhsassert_trueresult(str, false);
        lhshash_insert(vm, &vm->conststr, str, &str->hash);
    }

    return true;
}

static int lhsparser_initmainframe(LHSVM* vm, const char* fname)
{
    lhsassert_trueresult(vm, false);

    vm->mainframe = lhsmem_newgcobject
    (
        vm, 
        sizeof(LHSFrame), 
        LHS_TGCFRAME
    );
    lhsassert_trueresult(vm->mainframe, false);

    lhsframe_init(vm, vm->mainframe, false);
    vm->currentframe = vm->mainframe;
    
    lhsvm_pushstring(vm, fname);
    lhsvm_pushinteger(vm, LHS_FRAMENAME);
    lhsframe_insertvariable(vm, lhsframe_castmainframe(vm), 0, 0);
    return true;
}

static int lhsparser_symbolcomp(int token1, int token2)
{

}

static int lhsparser_enterlexical(LHSVM* vm, LHSLoadF* loadf, LHSLexical* lex)
{
    lhsassert_trueresult(vm && loadf && lex, false);

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
    lhsassert_trueresult(vm && loadf && loadf->lexical, false);

    lhsbuf_uninit(vm, &lhsloadf_castlex(loadf)->buf);
    loadf->lexical = 0;
    return true;
}

static int lhsparser_nextlexical(LHSVM* vm, LHSLoadF* loadf)
{
    lhsassert_trueresult(loadf, false);

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
        case '+':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENADDEQUAL;
            }
            else if (loadf->current == '+')
            {
                lhsloadf_getc(loadf);
                if (!lhsloadf_isright(loadf))
                {
                    return LHS_TOKENLINCREMENT;
                }
                return LHS_TOKENRINCREMENT;
            }
            return '+';
        }
        case '-':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENSUBEQUAL;
            }
            else if (loadf->current == '-')
            {
                lhsloadf_getc(loadf);
                if (!lhsloadf_isright(loadf))
                {
                    return LHS_TOKENLDECREMENT;
                }
                return LHS_TOKENRDECREMENT;
            }
            return '-';
        }
        case '*':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENMULEQUAL;
            }
            return '*';
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
            else if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENDIVEQUAL;
            }
            return '/';
        }
        case '&':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENBITANDEQUAL;
            }
            else if (loadf->current == '&')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENLOGICAND;
            }
            return '&';
        }
        case '|':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENBITOREQUAL;
            }
            else if (loadf->current == '|')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENLOGICOR;
            }
            return '|';
        }
        case '^':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENBITXOREQUAL;
            }
            return '|';
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
        case '<':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '<')
            {
                lhsloadf_getc(loadf);
                if (loadf->current == '=')
                {
                    lhsloadf_getc(loadf);
                    return LHS_TOKENLEFTSHIFTEQUAL;
                }
                return LHS_TOKENLEFTSHIFT;
            }
            return '<';
        }
        case '>':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '>')
            {
                lhsloadf_getc(loadf);
                if (loadf->current == '=')
                {
                    lhsloadf_getc(loadf);
                    return LHS_TOKENRIGHTSHIFTEQUAL;
                }
                return LHS_TOKENRIGHTSHIFT;
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
        case '%':
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '=')
            {
                lhsloadf_getc(loadf);
                return LHS_TOKENMODEQUAL;
            }
            return '%';
        }
        case '~':
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
                if (lhsbuf_isshort(vm, &lhsloadf_castlex(loadf)->buf))
                {
                    LHSString* str = lhsvm_findshort
                    (
                        vm,
                        lhsloadf_castlex(loadf)->buf.data,
                        lhsloadf_castlex(loadf)->buf.usize
                    );
                    if (str)
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
    lhsassert_trueresult(vm && loadf && loadf->lexical, false);

    lhsloadf_castlex(loadf)->line = loadf->line;
    lhsloadf_castlex(loadf)->column = loadf->column;

    if (lhsloadf_castlex(loadf)->lookahead != LHS_TOKENEOF)
    {
        lhsloadf_castlex(loadf)->token = lhsloadf_castlex(loadf)->lookahead;
        lhsloadf_castlex(loadf)->lookahead = LHS_TOKENEOF;
        return true;
    }

    lhsloadf_castlex(loadf)->token = lhsparser_nextlexical(vm, loadf);
    return true;
}

static int lhsparser_lookaheadtoken(LHSVM* vm, LHSLoadF* loadf)
{
    lhsassert_trueresult(vm && loadf && loadf->lexical, false);
    lhsassert_trueresult(lhsloadf_castlex(loadf)->lookahead == LHS_TOKENEOF, false);

    lhsloadf_castlex(loadf)->lookahead = lhsparser_nextlexical(vm, loadf);
    return true;
}

static int lhsparser_expressionstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsassert_trueresult(vm && loadf, false);

    lhsparser_nexttoken(vm, loadf);

    return true;
}

static int lhsparser_movestate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsassert_trueresult(vm && loadf, false);

    lhsparser_nexttoken(vm, loadf);
    if (lhsloadf_castlex(loadf)->token != LHS_TOKENIDENTIFY)
    {
        lhsexecute_symbolerr
        (
            vm, 
            "illegal variable '%s' declaration.",
            lhsframe_name(vm, lhsframe_castmainframe(vm)),
            loadf->line,
            loadf->column,
            lhsloadf_castlex(loadf)->buf.data
        );
        return false;
    }

    lhsvm_pushlstring
    (
        vm, 
        lhsloadf_castlex(loadf)->buf.data, 
        lhsloadf_castlex(loadf)->buf.usize
    );

    lhsparser_lookaheadtoken(vm, loadf);
    if (lhsloadf_castlex(loadf)->lookahead != '=')
    {
        lhsvm_pushnil(vm);
        return lhsframe_insertvariable
        (
            vm, 
            lhsframe_castcurframe(vm), 
            loadf->line, 
            loadf->column
        );
    }

    lhsparser_nexttoken(vm, loadf);
    return lhsparser_expressionstate(vm, loadf);
}

static int lhsparser_procstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsassert_trueresult(vm && loadf, false);

    while (lhsloadf_castlex(loadf)->token != LHS_TOKENEOF)
    {
        switch (lhsloadf_castlex(loadf)->token)
        {
        case LHS_TOKENGLOBAL:
        case LHS_TOKENLOCAL:
        {
            lhsparser_movestate(vm, loadf);
            break;
        }
        default:
        {
            lhsassert_true(false && lhsloadf_castlex(loadf)->token);
            break;
        }
        }

        lhsparser_nexttoken(vm, loadf);
    }

    return true;
}

static int lhsparser_initstate(LHSVM* vm, LHSLoadF* loadf)
{
    lhsassert_trueresult(vm && loadf, false);

    lhsloadf_getc(loadf);
    lhsparser_nexttoken(vm, loadf);
    lhsparser_procstate(vm, loadf);
    return true;
}

int lhsparser_dofile(LHSVM* vm, const char* fname)
{
    lhsassert_trueresult(vm && fname, false);
    
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
