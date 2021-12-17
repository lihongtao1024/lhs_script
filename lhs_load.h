#pragma once
#include "lhs_config.h"
#include "lhs_parser.h"
#include "lhs_vm.h"

#define SYMBOL_NONE                        (0)        
#define SYMBOL_BEGIN                       SYMBOL_NONE
#define SYMBOL_ADD                         (1)        
#define SYMBOL_SUB                         (2)        
#define SYMBOL_MUL                         (3)        
#define SYMBOL_DIV                         (4)        
#define SYMBOL_MOD                         (5)        
#define SYMBOL_ANDB                        (6)        
#define SYMBOL_ORB                         (7)        
#define SYMBOL_XORB                        (8)        
#define SYMBOL_LESS                        (9)        
#define SYMBOL_GREAT                       (10)       
#define SYMBOL_EQUAL                       (11)       
#define SYMBOL_NOTEQUAL                    (12)       
#define SYMBOL_GREATEQUAL                  (13)       
#define SYMBOL_LESSEQUAL                   (14)       
#define SYMBOL_LOGICAND                    (15)       
#define SYMBOL_LOGICOR                     (16)       
#define SYMBOL_BLSHIFT                     (17)       
#define SYMBOL_BRSHIFT                     (18)       
#define SYMBOL_MINUS                       (19)       
#define SYMBOL_NOT                         (20)       
#define SYMBOL_NOTB                        (21)       
#define SYMBOL_ASSIGN                      (22)       
#define SYMBOL_END                         (23)       
#define SYMBOL_DIGIT                       (24)       
#define SYMBOL_LETTER                      (25)       
#define SYMBOL_MAX                         (26)
#define SYMBOL_ERROR                       SYMBOL_MAX

#define lhsloadf_castlf(ud)                         \
((LHSLoadF*)ud)

#define lhsloadf_getc(lf)                           \
(lf)->current = getc((lf)->file);++(lf)->column

#define lhsloadf_isnewline(lf)                      \
((lf)->current == '\r' || (lf)->current == '\n')

#define lhsloadf_iseof(lf)                          \
((lf)->current == LHS_TOKENEOF)

#define lhsloadf_newline(lf)                        \
++(lf)->line; (lf)->column = 0

#define lhsloadf_isletter(lf)                       \
(lhsloadf_symbol[(unsigned char)                  \
((lf)->current)] == SYMBOL_LETTER)

#define lhsloadf_isdigit(lf)                        \
(lhsloadf_symbol[(unsigned char)                  \
((lf)->current)] == SYMBOL_DIGIT)

#define lhsloadf_isidentifier(lf)                   \
(lhsloadf_isletter(lf) || lhsloadf_isdigit(lf))

#define lhsloadf_isquote(lf)                        \
((lf)->current == '"')

#define lhsloadf_isright(lf)                        \
((lf)->lexical->token == LHS_TOKENIDENTIFIER)

extern char lhsloadf_symbol[];

typedef struct LHSLoadF
{
    FILE* file;
    int line;
    int column;
    int current;
    void* lexical;
} LHSLoadF;

int lhsloadf_init(LHSVM* vm, LHSLoadF* loadf, const char* fname);

int lhsloadf_skipline(LHSVM* vm, LHSLoadF* loadf);

int lhsloadf_skipcomment(LHSVM* vm, LHSLoadF* loadf);

int lhsloadf_saveidentifier(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf);

int lhsloadf_savedigital(LHSVM* vm, LHSLoadF* loadf, int *is_double, 
    LHSSTRBUF* buf);

int lhsloadf_savestring(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf);

int lhsloadf_savesymbol(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf);

int lhsloadf_addsymbol(LHSVM* vm, LHSLoadF* loadf, LHSSTRBUF* buf);

void lhsloadf_uninit(LHSVM* vm, LHSLoadF* loadf);
