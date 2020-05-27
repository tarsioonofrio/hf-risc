#include "rt.h"


jmp_buf *rt_jmp;
int rt_running_id;

// if we use delay or not
#define DELAY       0
// time of delay
#define DELAY_TIME	100
// log type or what is printed,
// 0 = no log, 1 = task id, 2 = task name
#define LOG		    2


void f(void){
    volatile char cushion[1000];	/* reserve some stack space */
    cushion[0] = '@';		/* don't optimize my cushion away */

    if (!setjmp(rt_jmp[rt_running_id]))
        longjmp(rt_jmp[0], 1);


    while (1) {
        /* thread body */
//		printf("task 0...%d\n", rt_running);
        if (LOG == 1) {
            printf("%d", rt_get_self_id());
        }
        if (LOG == 2) {
            printf("%s", rt_get_self_name());
        }
        if (DELAY) delay_ms(DELAY_TIME);
        #if defined __riscv
            if (TIMER ==0) rt_context_switch();
        #else
            rt_context_switch();
        #endif
    }
}


void f2(void){
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */

    if (!setjmp(rt_jmp[rt_running_id]))
        longjmp(rt_jmp[0], 1);

    while (1) {
        /* thread body */
        if (LOG == 1) {
            printf("%d", rt_get_self_id());
        }
        if (LOG == 2) {
            printf("%s", rt_get_self_name());
        }
        if (DELAY) delay_ms(DELAY_TIME);
        #if defined __riscv
            if (TIMER ==0) rt_context_switch();
        #else
            rt_context_switch();
        #endif
    }
}

void f1(void){
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */

    if (!setjmp(rt_jmp[rt_running_id]))
        longjmp(rt_jmp[0], 1);

	while (1) {
	    /* thread body */
        if (LOG == 1) {
            printf("%d", rt_get_self_id());
        }
        if (LOG == 2) {
            printf("%s", rt_get_self_name());
        }
        if (DELAY) delay_ms(DELAY_TIME);
        #if defined __riscv
            if (TIMER ==0) rt_context_switch();
        #else
            rt_context_switch();
        #endif
    }
}

void f0(void){
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

    if (!setjmp(rt_jmp[rt_running_id]))
        longjmp(rt_jmp[0], 1);

	while (1) {
	    /* thread body */
        if (LOG == 1) {
            printf("%d", rt_get_self_id());
        }
        if (LOG == 2) {
            printf("%s", rt_get_self_name());
        }
        if (DELAY) delay_ms(DELAY_TIME);
        #if defined __riscv
            if (TIMER ==0) rt_context_switch();
        #else
            rt_context_switch();
        #endif
    }
}


int main(void){
    rt_create();

    rt_add_task(f, 20, 3, 0, "1", READY);
    rt_add_task(f, 05, 2, 0, "2", READY);
    rt_add_task(f, 10, 2, 0, "3", READY);

    rt_start();
 	return 0;
}
