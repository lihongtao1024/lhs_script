#pragma once
#include "lhs_config.h"
#include "lhs_strbuf.h"
#include "lhs_vm.h"

#define LHS_TOKENNONE            (0)
#define LHS_TOKENEOF             EOF
#define LHS_TOKENSYMBOLBEGIN     (UCHAR_MAX + 1)
#define LHS_TOKENEQUAL           (LHS_TOKENSYMBOLBEGIN + SYMBOL_EQUAL)
#define LHS_TOKENNOTEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_NOTEQUAL)
#define LHS_TOKENGREATEQUAL      (LHS_TOKENSYMBOLBEGIN + SYMBOL_GREATEQUAL)
#define LHS_TOKENLESSEQUAL       (LHS_TOKENSYMBOLBEGIN + SYMBOL_LESSEQUAL) 
#define LHS_TOKENLOGICAND        (LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICAND)
#define LHS_TOKENLOGICOR         (LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICOR)
#define LHS_TOKENBLSHIFT         (LHS_TOKENSYMBOLBEGIN + SYMBOL_BLSHIFT)
#define LHS_TOKENBRSHIFT         (LHS_TOKENSYMBOLBEGIN + SYMBOL_BRSHIFT)
#define LHS_TOKENMINUS           (LHS_TOKENSYMBOLBEGIN + SYMBOL_MINUS)
#define LHS_TOKENSYMBOLEND       (LHS_TOKENMINUS + 1)
#define LHS_TOKENRESERVEDBEGIN   (USHRT_MAX + 1)
#define LHS_TOKENGLOBAL          (LHS_TOKENRESERVEDBEGIN + 1)
#define LHS_TOKENLOCAL           (LHS_TOKENRESERVEDBEGIN + 2)
#define LHS_TOKENFUNCTION        (LHS_TOKENRESERVEDBEGIN + 3)
#define LHS_TOKENFOR             (LHS_TOKENRESERVEDBEGIN + 4)
#define LHS_TOKENWHILE           (LHS_TOKENRESERVEDBEGIN + 5)
#define LHS_TOKENIF              (LHS_TOKENRESERVEDBEGIN + 6)
#define LHS_TOKENELSE            (LHS_TOKENRESERVEDBEGIN + 7)
#define LHS_TOKENDO              (LHS_TOKENRESERVEDBEGIN + 8)
#define LHS_TOKENUNTIL           (LHS_TOKENRESERVEDBEGIN + 9)
#define LHS_TOKENBREAK           (LHS_TOKENRESERVEDBEGIN + 10)
#define LHS_TOKENCONTINUE        (LHS_TOKENRESERVEDBEGIN + 11)
#define LHS_TOKENTRUE            (LHS_TOKENRESERVEDBEGIN + 12)
#define LHS_TOKENFALSE           (LHS_TOKENRESERVEDBEGIN + 13)
#define LHS_TOKENRETURN          (LHS_TOKENRESERVEDBEGIN + 14)
#define LHS_TOKENRESERVEDEND     (LHS_TOKENRESERVEDBEGIN + 15)
#define LHS_TOKENIDENTIFIER      (LHS_TOKENRESERVEDBEGIN + 16)
#define LHS_TOKENINTEGER         (LHS_TOKENRESERVEDBEGIN + 17)
#define LHS_TOKENNUMBER          (LHS_TOKENRESERVEDBEGIN + 18)
#define LHS_TOKENSTRING          (LHS_TOKENRESERVEDBEGIN + 19)

#define lhsparser_castlex(lf)                                           \
((LHSLexical*)(lf)->lexical)

typedef struct LHSToken
{
    int t;
    LHSSTRBUF buf;
} LHSToken;

typedef struct LHSJmp
{
    int len;
    size_t pos;    
    struct LHSJmp* next;
} LHSJmp;

typedef struct LHSChunk
{
    int index;
    struct LHSChunk* next;
    struct LHSChunk* parent; 
} LHSChunk;

typedef struct LHSRegion
{
    int token;
    int nbreak;
    LHSJmp* breakjmp;
    union
    {
        size_t pos;
        LHSJmp* jmp;
    } continuejmp;
    struct LHSRegion* next;
    struct LHSRegion* parent; 
} LHSRegion;

typedef struct LHSLexical
{
    int chunkid;
    int stacklayers;
    LHSToken token;
    LHSToken lookahead;
    LHSJmp* alljmp;
    LHSChunk* curchunk;
    LHSChunk* allchunk;
    LHSRegion* curregion;
    LHSRegion* allregion;
} LHSLexical;

extern const char* lhsparser_symbols[];

int lhsparser_loadfile(LHSVM* vm, const char* fname);
