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

static jmp_buf jmp[N_TASKS];
static int cur;

void context_switch()
{
	if (!setjmp(jmp[cur])) {
		if (N_TASKS == ++cur)
			cur = 0;
		longjmp(jmp[cur], 1);
	}
}

void idle_task()
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */
    cur = N_TASKS - 1;		/* the first thread to context switch is this one */

	setjmp(jmp[N_TASKS - 1]);

	while (1) {			/* thread body */
		context_switch();

		printf("idle task...\n");
		delay_ms(DELAY);
	}
}

void task2(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

	if (!setjmp(jmp[2]))
		idle_task();

	while (1) {			/* thread body */
		context_switch();

		printf("task 2...\n");
		delay_ms(DELAY);
	}
}

void task1(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

	if (!setjmp(jmp[1]))
		task2();

	while (1) {			/* thread body */
		context_switch();

		printf("task 1...\n");
		delay_ms(DELAY);
	}
}

void task0(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

	if (!setjmp(jmp[0]))
		task1();

	while (1) {			/* thread body */
		context_switch();

		printf("task 0...\n");
		delay_ms(DELAY);
	}
}


void task(int id_task)
{
    volatile char cushion[1000];	/* reserve some stack space */
    cushion[0] = '@';		/* don't optimize my cushion away */

    if (!setjmp(jmp[id_task]))
        task1();

    while (1) {			/* thread body */
        context_switch();

        printf("task 0...\n");
        delay_ms(DELAY);
    }
}


#if defined __riscv
    char mem_pool[8192];
#endif


int main(void)
{
    TIMER1PRE = TIMERPRE_DIV256;

    /* TIMER1 base frequency: 5M cycles */
    /* 5000000 / 256 = 19531.25 */
    TIMER1CTC = 19531;

    /* TIMER1 alt frequency: 100k cycles */
    /* 100000 / 256 = 390.625 */
    TIMER1OCR = 391;

    /* unlock TIMER1 for reset */
    TIMER1 = TIMERSET;
    TIMER1 = 0;

    /* enable interrupt mask for TIMER1 CTC and OCR events */
    TIMERMASK |= MASK_TIMER1CTC | MASK_TIMER1OCR;

    for(;;){
    }



//    task0();

	return 0;
}
