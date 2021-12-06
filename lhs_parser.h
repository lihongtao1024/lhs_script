#pragma once
#include "lhs_config.h"
#include "lhs_strbuf.h"
#include "lhs_vm.h"

#define LHS_TOKENEOF             EOF
#define LHS_TOKENSYMBOLBEGIN     (UCHAR_MAX            + 1)
#define LHS_TOKENEQUAL           (LHS_TOKENSYMBOLBEGIN + SYMBOL_EQUAL)     //==        267
#define LHS_TOKENNOTEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_NOTEQUAL)  //!=        268
#define LHS_TOKENGREATEQUAL      (LHS_TOKENSYMBOLBEGIN + SYMBOL_GREATEQUAL)//>=        269
#define LHS_TOKENLESSLEQUAL      (LHS_TOKENSYMBOLBEGIN + SYMBOL_LESSEQUAL) //<=        270
#define LHS_TOKENLOGICAND        (LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICAND)  //&&        271
#define LHS_TOKENLOGICOR         (LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICOR)   //||        272
#define LHS_TOKENBLSHIFT         (LHS_TOKENSYMBOLBEGIN + SYMBOL_BLSHIFT)   //<<        273
#define LHS_TOKENBRSHIFT         (LHS_TOKENSYMBOLBEGIN + SYMBOL_BRSHIFT)   //>>        274
#define LHS_TOKENMINUS           (LHS_TOKENSYMBOLBEGIN + SYMBOL_MINUS)     //-         275
#define LHS_TOKENSYMBOLEND       (LHS_TOKENMINUS  + 1)                     //  
#define LHS_TOKENRESERVEDBEGIN   (USHRT_MAX            + 1)
#define LHS_TOKENGLOBAL          (LHS_TOKENRESERVEDBEGIN + 1)              //set       65537
#define LHS_TOKENLOCAL           (LHS_TOKENRESERVEDBEGIN + 2)              //var       65538
#define LHS_TOKENFUNCTION        (LHS_TOKENRESERVEDBEGIN + 3)              //function  65539
#define LHS_TOKENFOR             (LHS_TOKENRESERVEDBEGIN + 4)              //for       65540
#define LHS_TOKENWHILE           (LHS_TOKENRESERVEDBEGIN + 5)              //while     65541
#define LHS_TOKENIF              (LHS_TOKENRESERVEDBEGIN + 6)              //if        65542
#define LHS_TOKENELSE            (LHS_TOKENRESERVEDBEGIN + 7)              //else      65543
#define LHS_TOKENSWITCH          (LHS_TOKENRESERVEDBEGIN + 8)              //switch    65544
#define LHS_TOKENCASE            (LHS_TOKENRESERVEDBEGIN + 9)              //case      65545
#define LHS_TOKENDEFAULT         (LHS_TOKENRESERVEDBEGIN + 10)             //default   65546
#define LHS_TOKENBREAK           (LHS_TOKENRESERVEDBEGIN + 11)             //break     65547
#define LHS_TOKENCONTINUE        (LHS_TOKENRESERVEDBEGIN + 12)             //continue  65548
#define LHS_TOKENTRUE            (LHS_TOKENRESERVEDBEGIN + 13)             //true      65549
#define LHS_TOKENFALSE           (LHS_TOKENRESERVEDBEGIN + 14)             //false     65550
#define LHS_TOKENRETURN          (LHS_TOKENRESERVEDBEGIN + 15)             //return    65551
#define LHS_TOKENIDENTIFIER      (LHS_TOKENRESERVEDBEGIN + 16)             //<name>    65552
#define LHS_TOKENINTEGER         (LHS_TOKENRESERVEDBEGIN + 17)             //<integer> 65553
#define LHS_TOKENNUMBER          (LHS_TOKENRESERVEDBEGIN + 18)             //<number>  65554
#define LHS_TOKENSTRING          (LHS_TOKENRESERVEDBEGIN + 19)             //<string>  65555
#define LHS_TOKENRESERVEDEND     (LHS_TOKENRESERVEDBEGIN + 20)

typedef struct LHSToken
{
    int t;
    LHSSTRBUF buf;
} LHSToken;

typedef struct LHSJmp
{
    size_t pos;
    size_t len;
    struct LHSJmp* next;
} LHSJmp;

typedef struct LHSLexical
{
    LHSToken token;
    LHSToken lookahead;
    LHSJmp* alljmp;
} LHSLexical;

int lhsparser_dofile(LHSVM* vm, const char* fname);
