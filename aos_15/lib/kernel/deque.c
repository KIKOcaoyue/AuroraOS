#include "deque.h"
#include "interrupt.h"

void deque_init(struct DEQUE* deque)
{
    deque->head.prev = NULL;
    deque->head.next = &deque->tail;
    deque->tail.prev = &deque->head;
    deque->tail.next = NULL;
}

/* 将elem插入到before之前 */
void deque_insert_before(struct DEQUE_ELEM* before, struct DEQUE_ELEM* elem)
{
    enum intr_status old_status = intr_disable();
    before->prev->next = elem;
    elem->prev = before->prev;
    elem->next = before;
    before->prev = elem;
    intr_set_status(old_status);
}

/* 添加元素到队首 */
void deque_push(struct DEQUE* deque, struct DEQUE_ELEM* elem)
{
    deque_insert_before(deque->head.next, elem);
}

/* 添加元素到队尾 */
void deque_append(struct DEQUE* deque, struct DEQUE_ELEM* elem)
{
    deque_insert_before(&deque->tail, elem);
}

/* 删除elem */
void deque_remove(struct DEQUE_ELEM* elem)
{
    enum intr_status old_status = intr_disable();
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    intr_set_status(old_status);
}

/* 弹出队列的第一个元素 */
struct DEQUE_ELEM* deque_pop(struct DEQUE* deque)
{
    struct DEQUE_ELEM* elem = deque->head.next;
    deque_remove(elem);
    return elem;
}

/* 从队列中查找oelem, 成功返回ture, 否则返回false */
int16_t elem_find(struct DEQUE* deque, struct DEQUE_ELEM* oelem)
{
    struct DEQUE_ELEM* elem = deque->head.next;
    while(elem != &deque->tail)
    {
        if(elem == oelem)
        {
            return 1;
        }
        elem = elem->next;
    }
    return 0;
}

/* 
    将队列deque中的每个元素elem和arg传给回调函数func
    arg给func用来判断elem是否符合条件
    本函数的功能是遍历队列, 逐个判断是否有符合条件的元素, 找到符合条件的元素返回元素指针, 否则返回NULL
 */
struct DEQUE_ELEM* deque_traversal(struct DEQUE* deque, function func, int arg)
{
    struct DEQUE_ELEM* elem = deque->head.next;
    if(deque_empty(deque))
    {
        return NULL;
    }
    while(elem != &deque->tail)
    {
        if(func(elem, arg))
        {
            return elem;
        }
        elem = elem->next;
    }
    return NULL;
}

uint32_t deque_len(struct DEQUE* deque)
{
    struct DEQUE_ELEM* elem = deque->head.next;
    uint32_t length = 0;
    while(elem != &deque->tail)
    {
        length++;
        elem = elem->next;
    }
    return length;
}

int16_t deque_empty(struct DEQUE* deque)
{
    return (deque->head.next==&deque->tail ? 1 : 0);
}