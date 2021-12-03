#pragma once
#include "lhs_config.h"
#include "lhs_variable.h"
#include "lhs_strbuf.h"
#include "lhs_vm.h"

#define OP_NONE                 (0)
#define OP_ADD                  (1)                     //+
#define OP_SUB                  (2)                     //-
#define OP_MUL                  (3)                     //*
#define OP_DIV                  (4)                     ///
#define OP_MOD                  (5)                     //%
#define OP_BAND                 (6)                     //&
#define OP_BOR                  (7)                     //|
#define OP_BXOR                 (8)                     //^
#define OP_LESS                 (9)                     //<
#define OP_GREAT                (10)                    //>
#define OP_EQUAL                (11)                    //==
#define OP_NOTEQUAL             (12)                    //!=
#define OP_GREATEQUAL           (13)                    //>=
#define OP_LESSEQUAL            (14)                    //<=
#define OP_LOGICAND             (15)                    //&&
#define OP_LOGICOR              (16)                    //||
#define OP_BLSHIFT              (17)                    //<<
#define OP_BRSHIFT              (18)                    //>>
#define OP_NEG                  (19)                    //-
#define OP_NOT                  (20)                    //!
#define OP_BNOT                 (21)                    //~
#define OP_PUSH                 (22)
#define OP_POP                  (23)
#define OP_MOV                  (24)                    //=
#define OP_PUSHC                (25)
#define OP_POPC                 (26)
#define OP_JMP                  (27)
#define OP_JMPF                 (28)
#define OP_NOP                  (29)
#define OP_MAX                  (30)

#define lhscode_unaryb(vm, s, v)                        \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, LHS_MARKBOOLEAN);   \
    lhsbuf_pushc((vm), &(vm)->code, (v));               \
    LHSCode code;                                       \
    code.mark = LHS_MARKBOOLEAN;                        \
    code.code.b = (v);                                  \
    lhscode_unarydump((vm), (s), &code);                \
}

#define lhscode_unaryl(vm, s, v)                        \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, LHS_MARKINTEGER);   \
    lhsbuf_pushl((vm), &(vm)->code, (v));               \
    LHSCode code;                                       \
    code.mark = LHS_MARKINTEGER;                        \
    code.code.i = (v);                                  \
    lhscode_unarydump((vm), (s), &code);                \
}

#define lhscode_unaryf(vm, s, v)                        \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, LHS_MARKNUMBER);    \
    lhsbuf_pushf((vm), &(vm)->code, (v));               \
    LHSCode code;                                       \
    code.mark = LHS_MARKNUMBER;                         \
    code.code.n = (v);                                  \
    lhscode_unarydump((vm), (s), &code);                \
}

#define lhscode_unaryi(vm, s, m, v)                     \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, (m));               \
    lhsbuf_pushi((vm), &(vm)->code, (v));               \
    LHSCode code;                                       \
    code.mark = (m);                                    \
    code.code.index = (v);                              \
    lhscode_unarydump((vm), (s), &code);                \
}

#define lhscode_unary(vm, s)                            \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    LHSCode code;                                       \
    code.mark = LHS_MARKNONE;                           \
    lhscode_unarydump((vm), (s), &code);                \
}

#define lhscode_binary(vm, s, m1, v1, m2, v2)           \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, (m1));              \
    lhsbuf_pushi((vm), &(vm)->code, (v1));              \
    lhsbuf_pushc((vm), &(vm)->code, (m2));              \
    lhsbuf_pushi((vm), &(vm)->code, (v2));              \
    LHSCode code1;                                      \
    code1.mark = (m1);                                  \
    code1.code.index = (v1);                            \
    LHSCode code2;                                      \
    code2.mark = (m2);                                  \
    code2.code.index = (v2);                            \
    lhscode_binarydump((vm), (s), &code1, &code2);      \
}

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

int lhscode_unarydump(LHSVM* vm, char symbol, LHSCode* code);

int lhscode_binarydump(LHSVM* vm, char symbol, LHSCode* code1, 
    LHSCode* code2);
