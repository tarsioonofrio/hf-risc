/*
 * example of coroutines using setjmp()/longjmp()
 */
#if defined __riscv
    #include <hf-risc.h>
#else
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <setjmp.h>
    #include <unistd.h>
    #include <time.h>
    #define delay_ms(x) sleep(x/1000)
#endif
#include "list.h"


#define DELAY		1000
#define N_TASKS		4

#if defined __riscv
    typedef uint32_t jmp_buf[20];
    int32_t setjmp(jmp_buf env);
    void longjmp(jmp_buf env, int32_t val);
#endif


static jmp_buf jmp_g[N_TASKS];
static int cur_g;

#if defined __riscv
    char mem_pool[8192];
#endif

void task0();
void task1();
void task2();
void idle_task();

struct list *list_function;
time_t last_second;

/*
 Preemption
 Set start set jmp process
 wait interruption, when this occur we have two options:
 1. jump to next function.
 2. in interruption handler define next function, return to current function and
    save context, jump to new function.
    How force to save context when returning of interruption?
    Without this is necessary wait until the next save context point in sequential execution.
 */

void context_switch()
{
	if (!setjmp(jmp_g[cur_g])) {
		if (N_TASKS == ++cur_g)
            cur_g = 0;
		longjmp(jmp_g[cur_g], 1);
	}
}

void idle_task()
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */
    cur_g = N_TASKS - 1;		/* the first thread to context switch is this one */
    time_t second;
	setjmp(jmp_g[N_TASKS - 1]);

	while (1) {			/* thread body */
		context_switch();

		printf("idle task...%d\n", cur_g);
        second = time(NULL);
        printf("%ld\n", second - last_second);
		delay_ms(DELAY);
	}
}

void task2(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */
    void (*func)();
    time_t second;
	if (!setjmp(jmp_g[2])) {
        cur_g++;
        func = list_get(list_function, cur_g);
        func();
    }

	while (1) {			/* thread body */
		context_switch();

		printf("task 2...%d\n", cur_g);
        second = time(NULL);
        printf("%ld\n", second - last_second);
		delay_ms(DELAY);
	}
}

void task1(void) {
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */
    void (*func)();
    time_t second;
    if (!setjmp(jmp_g[1])) {
        cur_g++;
        func = list_get(list_function, cur_g);
        func();
}

	while (1) {			/* thread body */
		context_switch();

		printf("task 1...%d\n", cur_g);
        second = time(NULL);
        printf("%ld\n", second - last_second);
		delay_ms(DELAY);
	}
}

void task0(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */
    void (*func)();
    time_t second;
	if (!setjmp(jmp_g[0])) {
        cur_g++;
        func = list_get(list_function, cur_g);
        func();
    }

	while (1) {			/* thread body */
		context_switch();

		printf("task 0...%d\n", cur_g);
        second = time(NULL);
        printf("%ld\n", second - last_second);

		delay_ms(DELAY);
	}
}


//void timer1ctc_handler(void)
//{
//    printf("TIMER1CTC %d\n", TIMER0);
//}

//void timer1ocr_handler(void)
//{
//    static uint32_t tmr1ocr = 0;
//
//    if (++tmr1ocr & 1) {
//        printf("TIMER1OCR %d\n", TIMER0);
//    }
//
//    /* schedule next interrupt (half period) */
//    TIMER1OCR = (TIMER1OCR + 196) % 19531;
//}


int main(void)
{
//    TIMER1PRE = TIMERPRE_DIV256;

    // Set time cycle between 5 ms and 50 ms
    /* TIMER1 base frequency: 8.190 ms or 8190 khz (cycles) */
    /* 8190 / 256 = 32 */
////    TIMER1CTC = 32;
//    TIMER1CTC = 32768;

    /* TIMER1 alt frequency: 100k cycles */
    /* 100000 / 256 = 390.625 */
//    TIMER1OCR = 391;

    /* unlock TIMER1 for reset */
//    TIMER1 = TIMERSET;
//    TIMER1 = 0;

    /* enable interrupt mask for TIMER1 CTC and OCR events */
//    TIMERMASK |= MASK_TIMER1CTC | MASK_TIMER1OCR;
//    for(;;){
//    }


    void (*func)();

    #if defined __riscv
        heap_init((uint32_t *)&mem_pool, sizeof(mem_pool));
    #endif

    list_function = list_init();
    if(list_append(list_function, task0)) printf("FAIL!");
    if(list_append(list_function, task1)) printf("FAIL!");
    if(list_append(list_function, task2)) printf("FAIL!");
    if(list_append(list_function, idle_task)) printf("FAIL!");

    func = list_get(list_function, 0);

    last_second = time(NULL);
    func();

	return 0;
}
