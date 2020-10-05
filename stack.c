#include "libbip.h"
#include "stack.h"


struct point_t pop(struct stack_t* stk) 
{
    return stk->array[--stk->head];
}
int push(struct stack_t* stk, struct point_t value) {
    if (!full(stk)) {
        _memcpy(&stk->array[stk->head], &value, sizeof(struct point_t));
        stk->head++;
        return stk->head;
    }
    else
        return -1;
}
void init(struct stack_t* stk) {
    stk->head = 0;
}

int empty(struct stack_t* stk) {
    return stk->head == 0;
}
int full(struct stack_t* stk) {
    return stk->head == MAX;
}