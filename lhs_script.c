#include "lhs_vm.h"
#include "lhs_hash.h"
#include "lhs_vector.h"
#include "lhs_value.h"
#include "lhs_link.h"
#include "lhs_parser.h"

/*
int test_equal(void* data1, void* data2)
{
    return *(int*)data1 == *(int*)data2;
}

long long test_calc(void* data)
{
    return (long long)(*((int*)data));
}

void print_hash(LHSHashTable* hash)
{
    for (int i = 0; i < hash->size; ++i)
    {
        LHSHashNode* node = hash->nodes[i];
        if (!node)
        {
            printf(u8"(0) ");
            continue;
        }

        printf(u8"(");
        while (node)
        {
            void* data = *(void**)(((char*)node) + offsetof(LHSHashNode, data));
            printf(u8"%d ", *((int*)data));

            node = node->next;
        }
        printf(u8")");
    }
    printf(u8"\n");
}
*/

/*
int test_equal(void* data1, void* data2)
{
    size_t l = strlen((const char*)data1);
    if (l != strlen((const char*)data2))
    {
        return false;
    }

    return !memcmp(data1, data2, l);
}

long long test_calc(void* data)
{
    size_t l = strlen((const char*)data);
    return lhsstr_hash((const char*)data, l, l & 0xffff);
}

void print_hash(LHSHashTable* hash)
{
    for (int i = 0; i < hash->size; ++i)
    {
        LHSHashNode* node = hash->nodes[i];
        if (!node)
        {
            printf(u8"(0) ");
            continue;
        }

        printf(u8"(");
        while (node)
        {
            void* data = *(void**)(((char*)node) + offsetof(LHSHashNode, data));
            printf(u8"%s ", (char*)data);

            node = node->next;
        }
        printf(u8")");
    }
    printf(u8"\n");
}
*/

/*
typedef struct SLNode
{
    int flag;
    struct SLNode* next;
}SLNode;

typedef struct SList
{
    SLNode* nodes;
}SList;

int SListRemove(SList* list, SLNode* node)
{
    return node->flag == 3;
}
*/

int main()
{
    /*
    SList list;
    lhslink_init(&list, nodes);
    
    SLNode node1;
    node1.flag = 1;
    node1.next = 0;

    SLNode node2;
    node2.flag = 2;
    node2.next = 0;

    SLNode node3;
    node3.flag = 3;
    node3.next = 0;

    SLNode node4;
    node4.flag = 4;
    node4.next = 0;

    SLNode node5;
    node5.flag = 5;
    node5.next = 0;
   
    lhsslink_push(&list, nodes, &node1, next);
    lhsslink_push(&list, nodes, &node2, next);
    lhsslink_push(&list, nodes, &node3, next);
    lhsslink_push(&list, nodes, &node4, next);
    lhsslink_push(&list, nodes, &node5, next);
    lhsslink_remove(SLNode, &list, nodes, next, &node2);
    lhsslink_removeif(SLNode, &list, nodes, next, SListRemove);
    */

    /*
    LHSVector vec;
    lhsvector_init(vm, &vec, sizeof(int));

    int n1 = 1, n2 = 2, n3 = 3, n4 = 4, n5 = 5;
    size_t index;
    lhsvector_insert(vm, &vec, &n1, &index);
    lhsvector_insert(vm, &vec, &n2, &index);
    lhsvector_insert(vm, &vec, &n3, &index);
    lhsvector_insert(vm, &vec, &n4, &index);
    lhsvector_insert(vm, &vec, &n5, &index);

    int v = *(int*)lhsvector_at(vm, &vec, 3);
    printf("%d ", v);

    lhsvector_remove(vm, &vec, 3);
    for (size_t i = 0; i < vec.usize; ++i)
    {
        v = *(int*)lhsvector_at(vm, &vec, i);
        printf("%d ", v);
    }
    lhsvector_uninit(vm, &vec);
    */

    /*
    LHSHashTable hashtable;
    lhshash_init(vm, &hashtable, test_calc, test_equal);

    int n1 = 1, n2 = 3, n3 = 4, n4 = 7, n5 = 27, n6 = 19;
    long long hash;

    lhshash_insert(vm, &hashtable, &n1, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, &n2, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, &n3, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, &n4, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, &n5, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, &n6, &hash);
    print_hash(&hashtable);
    
    void* data = lhshash_find(vm, &hashtable, &n2);
    lhshash_remove(vm, &hashtable, &n6);
    print_hash(&hashtable);

    lhshash_uninit(vm, &hashtable);
    */

    /*
    LHSHashTable hashtable;
    lhshash_init(vm, &hashtable, test_calc, test_equal);

    char s1[] = "lihong", s2[] = "test", s3[] = "中国", s4[] = "中国人", s5[] = "aa中 国kwg 一哈b";
    long long hash;

    lhshash_insert(vm, &hashtable, s1, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, s2, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, s3, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, s4, &hash);
    print_hash(&hashtable);

    lhshash_insert(vm, &hashtable, s5, &hash);
    print_hash(&hashtable);

    void* data = lhshash_find(vm, &hashtable, s3);
    char* p = data;
    lhshash_uninit(vm, &hashtable);
    */

    int aa = 3 * 3 & 1;
    //int bb = 10 + aa++;

    LHSVM* vm = lhsvm_create(0);
    lhsparser_dofile(vm, "./test.lhs");
    lhsvm_destroy(vm);


    return 0;
}

