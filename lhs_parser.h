#pragma once
#include "lhs_config.h"
#include "lhs_strbuf.h"
#include "lhs_vm.h"

#define LHS_TOKENEOF             EOF
#define LHS_TOKENSYMBOLBEGIN     (UCHAR_MAX            + 1)
#define LHS_TOKENEQUAL           (LHS_TOKENSYMBOLBEGIN + SYMBOL_EQUAL)             //==        271
#define LHS_TOKENNOTEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_NOTEQUAL)          //!=        272
#define LHS_TOKENGREATEQUAL      (LHS_TOKENSYMBOLBEGIN + SYMBOL_GREATEQUAL)        //>=        273
#define LHS_TOKENLESSLEQUAL      (LHS_TOKENSYMBOLBEGIN + SYMBOL_LESSEQUAL)         //<=        274
#define LHS_TOKENADDEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_ADDEQUAL)          //+=        275
#define LHS_TOKENSUBEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_SUBEQUAL)          //-=        276
#define LHS_TOKENMULEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_MULEQUAL)          //*=        277
#define LHS_TOKENDIVEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_DIVEQUAL)          ///=        278
#define LHS_TOKENLINCREMENT      (LHS_TOKENSYMBOLBEGIN + SYMBOL_LINCREMENT)        //++?       279
#define LHS_TOKENLDECREMENT      (LHS_TOKENSYMBOLBEGIN + SYMBOL_LDECREMENT)        //--?       280
#define LHS_TOKENLOGICAND        (LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICAND)          //&&        281
#define LHS_TOKENLOGICOR         (LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICOR)           //||        282
#define LHS_TOKENBITANDEQUAL     (LHS_TOKENSYMBOLBEGIN + SYMBOL_BITANDEQUAL)       //&=        283
#define LHS_TOKENBITOREQUAL      (LHS_TOKENSYMBOLBEGIN + SYMBOL_BITOREQUAL)        //|=        284
#define LHS_TOKENBITXOREQUAL     (LHS_TOKENSYMBOLBEGIN + SYMBOL_BITXOREQUAL)       //^=        285
#define LHS_TOKENLEFTSHIFT       (LHS_TOKENSYMBOLBEGIN + SYMBOL_BITLEFTSHIFT)      //<<        286
#define LHS_TOKENRIGHTSHIFT      (LHS_TOKENSYMBOLBEGIN + SYMBOL_BITRIGHTSHIFT)     //>>        287
#define LHS_TOKENLEFTSHIFTEQUAL  (LHS_TOKENSYMBOLBEGIN + SYMBOL_BITLEFTSHIFTEQUAL) //<<        288
#define LHS_TOKENRIGHTSHIFTEQUAL (LHS_TOKENSYMBOLBEGIN + SYMBOL_BITRIGHTSHIFTEQUAL)//>>        289
#define LHS_TOKENMODEQUAL        (LHS_TOKENSYMBOLBEGIN + SYMBOL_MODEQUAL)          //%=        290
#define LHS_TOKENRINCREMENT      (LHS_TOKENSYMBOLBEGIN + SYMBOL_RINCREMENT)        //?++       291
#define LHS_TOKENRDECREMENT      (LHS_TOKENSYMBOLBEGIN + SYMBOL_RDECREMENT)        //?--       292
#define LHS_TOKENSYMBOLEND       (LHS_TOKENRDECREMENT  + 1)                        //  
#define LHS_TOKENRESERVEDBEGIN   (USHRT_MAX            + 1)
#define LHS_TOKENGLOBAL          (LHS_TOKENRESERVEDBEGIN + 1)                      //set       65537
#define LHS_TOKENLOCAL           (LHS_TOKENRESERVEDBEGIN + 2)                      //var       65538
#define LHS_TOKENFUNCTION        (LHS_TOKENRESERVEDBEGIN + 3)                      //function  65539
#define LHS_TOKENFOR             (LHS_TOKENRESERVEDBEGIN + 4)                      //for       65540
#define LHS_TOKENWHILE           (LHS_TOKENRESERVEDBEGIN + 5)                      //while     65541
#define LHS_TOKENIF              (LHS_TOKENRESERVEDBEGIN + 6)                      //if        65542
#define LHS_TOKENELSE            (LHS_TOKENRESERVEDBEGIN + 7)                      //else      65543
#define LHS_TOKENSWITCH          (LHS_TOKENRESERVEDBEGIN + 8)                      //switch    65544
#define LHS_TOKENCASE            (LHS_TOKENRESERVEDBEGIN + 9)                      //case      65545
#define LHS_TOKENDEFAULT         (LHS_TOKENRESERVEDBEGIN + 10)                     //default   65546
#define LHS_TOKENBREAK           (LHS_TOKENRESERVEDBEGIN + 11)                     //break     65547
#define LHS_TOKENCONTINUE        (LHS_TOKENRESERVEDBEGIN + 12)                     //continue  65548
#define LHS_TOKENTRUE            (LHS_TOKENRESERVEDBEGIN + 13)                     //true      65549
#define LHS_TOKENFALSE           (LHS_TOKENRESERVEDBEGIN + 14)                     //false     65550
#define LHS_TOKENRETURN          (LHS_TOKENRESERVEDBEGIN + 15)                     //return    65551
#define LHS_TOKENIDENTIFY        (LHS_TOKENRESERVEDBEGIN + 16)                     //<name>    65552
#define LHS_TOKENINTEGER         (LHS_TOKENRESERVEDBEGIN + 17)                     //<integer> 65553
#define LHS_TOKENNUMBER          (LHS_TOKENRESERVEDBEGIN + 18)                     //<number>  65554
#define LHS_TOKENSTRING          (LHS_TOKENRESERVEDBEGIN + 19)                     //<string>  65555
#define LHS_TOKENRESERVEDEND     (LHS_TOKENRESERVEDBEGIN + 20)

typedef struct LHSLexical
{
    int token;
    int lookahead;
    long long line;
    long long column;
    LHSSTRBUF buf;
} LHSLexical;

int lhsparser_dofile(LHSVM* vm, const char* fname);
