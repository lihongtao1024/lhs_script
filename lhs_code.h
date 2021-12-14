#pragma once
#include "lhs_config.h"
#include "lhs_variable.h"
#include "lhs_strbuf.h"
#include "lhs_vm.h"

#define LHS_MULTRET             (-1)
#define LHS_VOIDRET             (0)
#define LHS_RETSULT             (1)

#define OP_NONE                 (0)
#define OP_ADD                  (1)                     //+
#define OP_SUB                  (2)                     //-
#define OP_MUL                  (3)                     //*
#define OP_DIV                  (4)                     ///
#define OP_MOD                  (5)                     //%
#define OP_ANDB                 (6)                     //&
#define OP_ORB                  (7)                     //|
#define OP_XORB                 (8)                     //^
#define OP_L                    (9)                     //<
#define OP_G                    (10)                    //>
#define OP_E                    (11)                    //==
#define OP_NE                   (12)                    //!=
#define OP_GE                   (13)                    //>=
#define OP_LE                   (14)                    //<=
#define OP_AND                  (15)                    //&&
#define OP_OR                   (16)                    //||
#define OP_SHL                  (17)                    //<<
#define OP_SHR                  (18)                    //>>
#define OP_NEG                  (19)                    //-
#define OP_NOT                  (20)                    //!
#define OP_NOTB                 (21)                    //~
#define OP_MOV                  (22)                    //=
#define OP_PUSH                 (23)
#define OP_POP                  (24)
#define OP_PUSHC                (25)
#define OP_POPC                 (26)
#define OP_JMP                  (27)
#define OP_JZ                   (28)
#define OP_JNZ                  (29)
#define OP_NOP                  (30)
#define OP_CALL                 (31)
#define OP_RET                  (32)
#define OP_RETURN               (33)
#define OP_EXIT                 (34)
#define OP_MAX                  (35)

#define lhscode_unaryb(vm, s, v)                        \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, LHS_MARKBOOLEAN);   \
    lhsbuf_pushc((vm), &(vm)->code, (v));               \
}

#define lhscode_unaryl(vm, s, v)                        \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, LHS_MARKINTEGER);   \
    lhsbuf_pushl((vm), &(vm)->code, (v));               \
}

#define lhscode_unaryf(vm, s, v)                        \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, LHS_MARKNUMBER);    \
    lhsbuf_pushf((vm), &(vm)->code, (v));               \
}

#define lhscode_unaryi(vm, s, m, v)                     \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, (m));               \
    lhsbuf_pushi((vm), &(vm)->code, (v));               \
}

#define lhscode_unary(vm, s)                            \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
}

#define lhscode_binary(vm, s, m1, v1, m2, v2)           \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, (s));               \
    lhsbuf_pushc((vm), &(vm)->code, (m1));              \
    lhsbuf_pushi((vm), &(vm)->code, (v1));              \
    lhsbuf_pushc((vm), &(vm)->code, (m2));              \
    lhsbuf_pushi((vm), &(vm)->code, (v2));              \
}

#define lhscode_call(vm, m, v, an, rb)                  \
{                                                       \
    lhsbuf_pushc((vm), &(vm)->code, OP_CALL);           \
    lhsbuf_pushc((vm), &(vm)->code, (m));               \
    lhsbuf_pushi((vm), &(vm)->code, (v));               \
    lhsbuf_pushi((vm), &(vm)->code, (an));              \
    lhsbuf_pushc((vm), &(vm)->code, (rb));              \
}

int lhscode_dmpcode(LHSVM* vm);
