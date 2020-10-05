#define MAX 128

typedef unsigned char byte;

struct point_t
{
    short x;
    short y;
};

typedef struct stack_t {
    struct point_t array[MAX];
    int head;
} stack;

int empty(struct stack_t* stk);
int full(struct stack_t* stk);
struct point_t pop(struct stack_t* stk);
int push(struct stack_t* stk, struct point_t pt);
void init(struct stack_t* stk);
