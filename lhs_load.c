#include "lhs_load.h"
#include "lhs_frame.h"
#include "lhs_execute.h"
#include "lhs_error.h"

char lhsloadf_symbolid[] =
{  
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NOT,     SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_MOD,     SYMBOL_BAND,    SYMBOL_NONE,
    SYMBOL_LBRACKET, SYMBOL_RBRACKET, SYMBOL_MUL,     SYMBOL_ADD,     SYMBOL_NONE,
    SYMBOL_SUB,      SYMBOL_DIGIT,    SYMBOL_DIV,     SYMBOL_DIGIT,   SYMBOL_DIGIT,
    SYMBOL_DIGIT,    SYMBOL_DIGIT,    SYMBOL_DIGIT,   SYMBOL_DIGIT,   SYMBOL_DIGIT,
    SYMBOL_DIGIT,    SYMBOL_DIGIT,    SYMBOL_DIGIT,   SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_LESS,     SYMBOL_NONE,     SYMBOL_GREAT,   SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_BXOR,
    SYMBOL_LETTER,   SYMBOL_NONE,     SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_NONE,    SYMBOL_BOR,
    SYMBOL_NONE,     SYMBOL_BNOT,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE
};

const char* lhsloadf_symbolname[] =
{
    "n/a", "+", "-", "*", "/", "%", "&", "|", "^",
    "<", ">", "==", "!=", ">=", "<=", "&&", 
    "||", "<<", ">>", "-", "!", "~", "(", ")"
};

int lhsloadf_skipline(LHSVM* vm, LHSLoadF* loadf)
{
    do
    {
        lhsloadf_getc(loadf);
    } while (!lhsloadf_isnewline(loadf) && 
             !lhsloadf_iseof(loadf));
    return LHS_TRUE;
}

int lhsloadf_skipcomment(LHSVM* vm, LHSLoadF* loadf)
{
    for (; ; )
    {
        lhsloadf_getc(loadf);
        if (loadf->current == '*')
        {
            lhsloadf_getc(loadf);
            if (loadf->current == '/')
            {
                lhsloadf_getc(loadf);
                break;
            }
        }
        else if (loadf->current == '\n')
        {
            lhsloadf_newline(loadf);
        }
        else if (loadf->current == LHS_TOKENEOF)
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf,
                "<comment> unexpected end of file."
            );
        }
    }

    return LHS_TRUE;
}

int lhsloadf_saveidentifier(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf)
{
    lhsbuf_reset(vm, buf);

    do
    {
        lhsbuf_pushc(vm, buf, (char)loadf->current);
        lhsloadf_getc(loadf);
    } while (lhsloadf_isidentifier(loadf));
    return LHS_TRUE;
}

int lhsloadf_savedigital(LHSVM* vm, LHSLoadF* loadf, int *is_double, 
    LHSSTRBUF* buf)
{
    lhsbuf_reset(vm, buf);

    int dot = 0;
    *is_double = 0;
    do
    {
        lhsbuf_pushc(vm, buf, (char)loadf->current);
        lhsloadf_getc(loadf);
        if (loadf->current == '.')
        {
            ++dot;
        }
    } while (lhsloadf_isdigit(loadf));

    if (dot)
    {
        if (dot > 1)
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf,
                "solving decimal '%s'.",
                buf->data
            );
        }
        else
        {
            *is_double = 1;
        }
    }
    return LHS_TRUE;
};

int lhsloadf_savestring(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf)
{
    lhsbuf_reset(vm, buf);
    lhsloadf_getc(loadf);

    do
    {
        lhsbuf_pushc(vm, buf, (char)loadf->current);
        lhsloadf_getc(loadf);
        if (lhsloadf_iseof(loadf))
        {
            lhserr_syntaxerr
            (
                vm, 
                loadf, 
                "<string> unexpected end of file."
            );
        }
    } while (!lhsloadf_isquote(loadf));
    lhsloadf_getc(loadf);
    return LHS_TRUE;
}

int lhsloadf_savesymbol(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf)
{
    lhsbuf_reset(vm, buf);
    lhsbuf_pushc(vm, buf, (char)loadf->current);
    return LHS_TRUE;
}

int lhsloadf_addsymbol(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf)
{
    lhsbuf_pushc(vm, buf, (char)loadf->current);
    return LHS_TRUE;
}

int lhsloadf_init(LHSVM* vm, LHSLoadF* loadf, const char* fname)
{
    loadf->file = fopen(fname, "rb");
    if (!loadf->file)
    {
        lhserr_no("load file %s fail", fname);
        return LHS_FALSE;
    }

    loadf->current = 0;
    loadf->column = 0;
    loadf->line = 1;
    loadf->lexical = 0;
    return LHS_TRUE;
}

void lhsloadf_uninit(LHSVM* vm, LHSLoadF* loadf)
{
    lhs_unused(vm);

    if (loadf->file)
    {
        fclose(loadf->file);
    }
}
