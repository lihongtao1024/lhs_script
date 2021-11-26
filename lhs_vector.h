#pragma once
#include "lhs_config.h"

typedef struct LHSVector
{
    size_t size;
    size_t usize;
    size_t esize;
    void* nodes;
} LHSVector;

int lhsvector_init(void* vm, LHSVector* vector, size_t esize);

int lhsvector_push(void* vm, LHSVector* vector, void* data, 
    size_t* index);

void lhsvector_pop(void* vm, LHSVector* vector, size_t n);

void lhsvector_remove(void* vm, LHSVector* vector, size_t index);

void* lhsvector_at(void* vm, LHSVector* vector, size_t index);

void* lhsvector_increment(void* vm, LHSVector* vector);

void* lhsvector_back(void* vm, LHSVector* vector);

size_t lhsvector_length(void* vm, LHSVector* vector);

void lhsvector_uninit(void* vm, LHSVector* vector);
