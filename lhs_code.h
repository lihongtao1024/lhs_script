#pragma once
#include "lhs_config.h"
#include "lhs_variable.h"
#include "lhs_vm.h"

#define OP_NONE                 (0)
#define OP_ADD                  (1)           //+
#define OP_SUB                  (2)           //-
#define OP_MUL                  (3)           //*
#define OP_DIV                  (4)           ///
#define OP_MOD                  (5)           //%
#define OP_BAND                 (6)           //&
#define OP_BOR                  (7)           //|
#define OP_BXOR                 (8)           //^
#define OP_LESS                 (9)           //<
#define OP_GREAT                (10)          //>
#define OP_EQUAL                (11)          //==
#define OP_NOTEQUAL             (12)          //!=
#define OP_GREATEQUAL           (13)          //>=
#define OP_LESSEQUAL            (14)          //<=
#define OP_LOGICAND             (15)          //&&
#define OP_LOGICOR              (16)          //||
#define OP_BLSHIFT              (17)          //<<
#define OP_BRSHIFT              (18)          //>>
#define OP_NOT                  (19)          //!
#define OP_BNOT                 (20)          //~
#define OP_PUSH                 (21)
#define OP_POP                  (22)
#define OP_MOVE                 (23)          //=
#define OP_MAX                  (24)

typedef struct LHSCode
{
    int mark;
    union
    {
        char b;
        int index;
        long long i;
        double n;
    } code;
} LHSCode;

typedef struct LHSInstruction
{
    int op;
    union
    {
        struct
        {
            LHSCode code1;
            LHSCode code2;
        } binary;
        struct
        {
            LHSCode code;
        } unary;
    } body;
} LHSInstruction;

int lhscode_unaryexpr(LHSVM* vm, char symbol, LHSCode* code);

int lhscode_binaryexpr(LHSVM* vm, char symbol, LHSCode* code1, 
    LHSCode* code2);
