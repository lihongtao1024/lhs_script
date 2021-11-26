#pragma once
#include "lhs_config.h"
#include "lhs_load.h"
#include "lhs_strbuf.h"
#include "lhs_vm.h"

#define LHS_TOKENEOF             EOF
#define LHS_TOKENSYMBOLBEGIN     UCHAR_MAX            + 1
#define LHS_TOKENEQUAL           LHS_TOKENSYMBOLBEGIN + SYMBOL_EQUAL             //==
#define LHS_TOKENNOTEQUAL        LHS_TOKENSYMBOLBEGIN + SYMBOL_NOTEQUAL          //!=
#define LHS_TOKENGREATEQUAL      LHS_TOKENSYMBOLBEGIN + SYMBOL_GREATEQUAL        //>=
#define LHS_TOKENLESSLEQUAL      LHS_TOKENSYMBOLBEGIN + SYMBOL_LESSEQUAL         //<=
#define LHS_TOKENADDEQUAL        LHS_TOKENSYMBOLBEGIN + SYMBOL_ADDEQUAL          //+=
#define LHS_TOKENSUBEQUAL        LHS_TOKENSYMBOLBEGIN + SYMBOL_SUBEQUAL          //-=
#define LHS_TOKENMULEQUAL        LHS_TOKENSYMBOLBEGIN + SYMBOL_MULEQUAL          //*=
#define LHS_TOKENDIVEQUAL        LHS_TOKENSYMBOLBEGIN + SYMBOL_DIVEQUAL          ///=
#define LHS_TOKENLINCREMENT      LHS_TOKENSYMBOLBEGIN + SYMBOL_LINCREMENT        //++?
#define LHS_TOKENLDECREMENT      LHS_TOKENSYMBOLBEGIN + SYMBOL_LDECREMENT        //--?
#define LHS_TOKENLOGICAND        LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICAND          //&&
#define LHS_TOKENLOGICOR         LHS_TOKENSYMBOLBEGIN + SYMBOL_LOGICOR           //||
#define LHS_TOKENBITANDEQUAL     LHS_TOKENSYMBOLBEGIN + SYMBOL_BITANDEQUAL       //&=
#define LHS_TOKENBITOREQUAL      LHS_TOKENSYMBOLBEGIN + SYMBOL_BITOREQUAL        //|=
#define LHS_TOKENBITXOREQUAL     LHS_TOKENSYMBOLBEGIN + SYMBOL_BITXOREQUAL       //^=
#define LHS_TOKENLEFTSHIFT       LHS_TOKENSYMBOLBEGIN + SYMBOL_BITLEFTSHIFT      //<<
#define LHS_TOKENRIGHTSHIFT      LHS_TOKENSYMBOLBEGIN + SYMBOL_BITRIGHTSHIFT     //>>
#define LHS_TOKENLEFTSHIFTEQUAL  LHS_TOKENSYMBOLBEGIN + SYMBOL_BITLEFTSHIFTEQUAL //<<
#define LHS_TOKENRIGHTSHIFTEQUAL LHS_TOKENSYMBOLBEGIN + SYMBOL_BITRIGHTSHIFTEQUAL//>>
#define LHS_TOKENMODEQUAL        LHS_TOKENSYMBOLBEGIN + SYMBOL_MODEQUAL          //%=
#define LHS_TOKENRINCREMENT      LHS_TOKENSYMBOLBEGIN + SYMBOL_RINCREMENT        //?++
#define LHS_TOKENRDECREMENT      LHS_TOKENSYMBOLBEGIN + SYMBOL_RDECREMENT        //?--
#define LHS_TOKENSYMBOLEND       LHS_TOKENRDECREMENT  + 1                        //  
#define LHS_TOKENRESERVEDBEGIN   USHRT_MAX            + 1
#define LHS_TOKENGLOBAL          LHS_TOKENRESERVEDBEGIN + 1                      //set
#define LHS_TOKENLOCAL           LHS_TOKENRESERVEDBEGIN + 2                      //var
#define LHS_TOKENFUNCTION        LHS_TOKENRESERVEDBEGIN + 3                      //function
#define LHS_TOKENFOR             LHS_TOKENRESERVEDBEGIN + 4                      //for
#define LHS_TOKENWHILE           LHS_TOKENRESERVEDBEGIN + 5                      //while
#define LHS_TOKENIF              LHS_TOKENRESERVEDBEGIN + 6                      //if
#define LHS_TOKENELSE            LHS_TOKENRESERVEDBEGIN + 7                      //else
#define LHS_TOKENSWITCH          LHS_TOKENRESERVEDBEGIN + 8                      //switch
#define LHS_TOKENCASE            LHS_TOKENRESERVEDBEGIN + 9                      //case
#define LHS_TOKENDEFAULT         LHS_TOKENRESERVEDBEGIN + 10                     //default
#define LHS_TOKENBREAK           LHS_TOKENRESERVEDBEGIN + 11                     //break
#define LHS_TOKENCONTINUE        LHS_TOKENRESERVEDBEGIN + 12                     //continue
#define LHS_TOKENTRUE            LHS_TOKENRESERVEDBEGIN + 13                     //true
#define LHS_TOKENFALSE           LHS_TOKENRESERVEDBEGIN + 14                     //false
#define LHS_TOKENRETURN          LHS_TOKENRESERVEDBEGIN + 15                     //return
#define LHS_TOKENIDENTIFY        LHS_TOKENRESERVEDBEGIN + 21                     //<name>
#define LHS_TOKENINTEGER         LHS_TOKENRESERVEDBEGIN + 22                     //<integer>
#define LHS_TOKENNUMBER          LHS_TOKENRESERVEDBEGIN + 23                     //<number>
#define LHS_TOKENSTRING          LHS_TOKENRESERVEDBEGIN + 24                     //<string>
#define LHS_TOKENRESERVEDEND     LHS_TOKENRESERVEDBEGIN + 16

typedef struct LHSLexical
{
    int token;
    int lookahead;
    long long line;
    long long column;
    LHSSTRBUF buf;
} LHSLexical;

int lhsparser_dofile(LHSVM* vm, const char* fname);
