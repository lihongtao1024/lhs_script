#include "lhs_load.h"
#include "lhs_frame.h"
#include "lhs_execute.h"
#include "lhs_error.h"

char lhsloadf_symbols[] =
{  
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NOT,     SYMBOL_NONE,    SYMBOL_NONE,
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

int lhsloadf_skipline(LHSVM* vm, LHSLoadF* loadf)
{
    do
    {
        lhsloadf_getc(loadf);
    } while (!lhsloadf_isnewline(loadf));
    return true;
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
            lhsexecute_symbolerr
            (
                vm, 
                "<EOF> illegal end of file.",
                lhsframe_name(vm, lhsframe_castmainframe(vm)),
                loadf->line,
                loadf->column
            );
            break;
        }
    }

    return true;
}

int lhsloadf_saveidentifier(LHSVM* vm, LHSLoadF* loadf)
{
    lhsbuf_reset(vm, &loadf->lexical->buf);

    do
    {
        lhsbuf_pushchar
        (
            vm, 
            &loadf->lexical->buf, 
            (char)loadf->current
        );
        lhsloadf_getc(loadf);
    } while (lhsloadf_isidentifier(loadf));
    return true;
}

int lhsloadf_savedigital(LHSVM* vm, LHSLoadF* loadf, int *is_double)
{
    lhsbuf_reset(vm, &loadf->lexical->buf);

    int dot = 0;
    *is_double = 0;
    do
    {
        lhsbuf_pushchar
        (
            vm,
            &loadf->lexical->buf,
            (char)loadf->current
        );

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
            lhsexecute_symbolerr
            (
                vm, 
                "illegal decimal.",
                lhsframe_name(vm, lhsframe_castmainframe(vm)),
                loadf->line,
                loadf->column
            );
        }
        else
        {
            *is_double = 1;
        }
    }
    return true;
};

int lhsloadf_savestring(LHSVM* vm, LHSLoadF* loadf)
{
    lhsbuf_reset(vm, &loadf->lexical->buf);
    lhsloadf_getc(loadf);

    do
    {
        lhsbuf_pushchar
        (
            vm, 
            &loadf->lexical->buf, 
            (char)loadf->current
        );
        lhsloadf_getc(loadf);
    } while (!lhsloadf_isquote(loadf));
    lhsloadf_getc(loadf);
    return true;
}

int lhsloadf_init(LHSVM* vm, LHSLoadF* loadf, const char* fname)
{
    loadf->file = fopen(fname, "rb");
    if (!loadf->file)
    {
        lhs_errno("load file %s fail", fname);
        return false;
    }

    loadf->current = 0;
    loadf->column = 0;
    loadf->line = 1;
    loadf->lexical = 0;
    return true;
}

void lhsloadf_uninit(LHSVM* vm, LHSLoadF* loadf)
{
    unused(vm);

    if (loadf->file)
    {
        fclose(loadf->file);
    }
}
