#include "lhs_load.h"
#include "lhs_function.h"
#include "lhs_error.h"

char lhsloadf_symbol[] =
{  
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NOT,     SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_MOD,     SYMBOL_ANDB,    SYMBOL_NONE,
    SYMBOL_NONE,     SYMBOL_NONE,     SYMBOL_MUL,     SYMBOL_ADD,     SYMBOL_NONE,
    SYMBOL_SUB,      SYMBOL_DIGIT,    SYMBOL_DIV,     SYMBOL_DIGIT,   SYMBOL_DIGIT,
    SYMBOL_DIGIT,    SYMBOL_DIGIT,    SYMBOL_DIGIT,   SYMBOL_DIGIT,   SYMBOL_DIGIT,
    SYMBOL_DIGIT,    SYMBOL_DIGIT,    SYMBOL_DIGIT,   SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_LESS,     SYMBOL_ASSIGN,   SYMBOL_GREAT,   SYMBOL_NONE,    SYMBOL_NONE,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_NONE,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_XORB,
    SYMBOL_LETTER,   SYMBOL_NONE,     SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_LETTER,  SYMBOL_LETTER,
    SYMBOL_LETTER,   SYMBOL_LETTER,   SYMBOL_LETTER,  SYMBOL_NONE,    SYMBOL_ORB,
    SYMBOL_NONE,     SYMBOL_NOTB,     SYMBOL_NONE,    SYMBOL_NONE,    SYMBOL_NONE,
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
            lhserr_syntax
            (
                vm, 
                loadf,
                "<comment> unexpected end of file."
            );
        }
    }

    return LHS_TRUE;
}

int lhsloadf_saveidentifier(LHSVM* vm, LHSLoadF* loadf, LHSBuf* buf)
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
    LHSBuf* buf)
{
    lhsbuf_reset(vm, buf);

    int dot = 0;
    *is_double = LHS_FALSE;
    do
    {
        lhsbuf_pushc(vm, buf, (char)loadf->current);
        lhsloadf_getc(loadf);
        if (loadf->current == '.')
        {
            ++dot;
        }
    } while (lhsloadf_isdigit(loadf));

    (dot > 1) &&
        lhserr_syntax
        (
            vm, 
            loadf,
            "unexpected digital format '%s'.",
            buf->data
        );

    *is_double = dot ? LHS_TRUE : LHS_FALSE;
    return LHS_TRUE;
};

int lhsloadf_savestring(LHSVM* vm, LHSLoadF* loadf, LHSBuf* buf)
{
    lhsbuf_reset(vm, buf);
    lhsloadf_getc(loadf);

    do
    {
        if (loadf->current == '\\')
        {
            lhsbuf_pushc(vm, buf, (char)loadf->current);
            lhsloadf_getc(loadf);
            
            switch (loadf->current)
            {
            case 'n':
            {
                buf->usize--;
                lhsbuf_pushc(vm, buf, '\n');
                lhsloadf_getc(loadf);
                break;
            }
            case 'r':
            {
                buf->usize--;
                lhsbuf_pushc(vm, buf, '\r');
                lhsloadf_getc(loadf);
                break;
            }
            case 't':
            {
                buf->usize--;
                lhsbuf_pushc(vm, buf, '\t');
                lhsloadf_getc(loadf);
                break;
            }
            case 'f':
            {
                buf->usize--;
                lhsbuf_pushc(vm, buf, '\f');
                lhsloadf_getc(loadf);
                break;
            }
            case 'v':
            {
                buf->usize--;
                lhsbuf_pushc(vm, buf, '\v');
                lhsloadf_getc(loadf);
                break;
            }
            case '"':
            {
                buf->usize--;
                lhsbuf_pushc(vm, buf, '\"');
                lhsloadf_getc(loadf);
                break;
            }
            case '\\':
            {
                buf->usize--;
                lhsbuf_pushc(vm, buf, '\\');
                lhsloadf_getc(loadf);
                break;
            }
            default:
            {
                break;
            }
            }
        }
        else
        {
            lhsbuf_pushc(vm, buf, (char)loadf->current);
            lhsloadf_getc(loadf);
        }
        
        if (loadf->current == '\n' ||
            lhsloadf_iseof(loadf))
        {
            lhserr_syntax
            (
                vm, 
                loadf, 
                "<string> unexpected end."
            );
        } 
    } while (!lhsloadf_isquote(loadf));
    lhsloadf_getc(loadf);
    return LHS_TRUE;
}

int lhsloadf_savesymbol(LHSVM* vm, LHSLoadF* loadf, LHSBuf* buf)
{
    lhsbuf_reset(vm, buf);
    lhsbuf_pushc(vm, buf, (char)loadf->current);
    return LHS_TRUE;
}

int lhsloadf_addsymbol(LHSVM* vm, LHSLoadF* loadf, LHSBuf* buf)
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
    loadf->file && fclose(loadf->file);
}
