#ifndef __LIB_KERNEL_DEQUE_H
#define __LIB_KERNEL_DEQUE_H
#include "global.h"

#define offset(struct_type, member) (int)(&((struct_type*)0)->member)
#define elem_to_entry(struct_type, struct_member_name, elem_ptr) (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))
#define NULL ((void*)0)

struct DEQUE_ELEM
{
    struct DEQUE_ELEM* prev;
    struct DEQUE_ELEM* next;
};

struct DEQUE
{
    struct DEQUE_ELEM head;
    struct DEQUE_ELEM tail;
};

typedef int16_t (function) (struct DEQUE_ELEM*, int arg);

void deque_init(struct DEQUE*);
void deque_insert_before(struct DEQUE_ELEM* before, struct DEQUE_ELEM* elem);
void deque_push(struct DEQUE* deque, struct DEQUE_ELEM* elem);
void deque_iterate(struct DEQUE* deque);
void deque_append(struct DEQUE* deque, struct DEQUE_ELEM* elem);
void deque_remove(struct DEQUE_ELEM* elem);
struct DEQUE_ELEM* deque_pop(struct DEQUE* deque);
int16_t deque_empty(struct DEQUE* deque);
uint32_t deque_len(struct DEQUE* deque);
struct DEQUE_ELEM* deque_traversal(struct DEQUE* deque, function func, int arg);
int16_t elem_find(struct DEQUE* deque, struct DEQUE_ELEM* obj_elem);

#endif 