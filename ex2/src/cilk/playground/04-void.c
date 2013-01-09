/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : 03-void.cilk
* Creation Date : 08-01-2013
* Last Modified : Tue 08 Jan 2013 05:56:59 PM EET
* Created By : Greg Liras <gregliras@gmail.com>
_._._._._._._._._._._._._._._._._._._._._.*/

#include <stdlib.h>


struct Task {
    void (*func)(void *);
    void *args;
};

typedef struct Task Task;


void execute(struct Task *t)
{
    (*t->func)(t->args);
}


struct Task *setTask(void *func, void *args)
{
    Task *t = malloc(sizeof(struct Task));
    t->func = func;
    t->args = args;
    return t;
}

struct myArgs {
    int i;
    int j;
    int k;
};

void runme(struct myArgs *m)
{
    m->i = 0;
    m->j = 1;
    m->k = 2;
}


int main(void)
{
    struct myArgs asdf;

    Task *t = setTask((void *) runme, (void *) &asdf);

    printf("%d %d %d\n", asdf.i, asdf.j, asdf.k);
    execute(t);
    printf("%d %d %d\n", asdf.i, asdf.j, asdf.k);

}
