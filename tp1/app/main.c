/*
 * example of coroutines using setjmp()/longjmp()
 */
#if defined __x86_64__
    #include <stdint.h>
    #include <stdio.h>
    #include <setjmp.h>
    #define delay_ms(x) sleep(x/1000)
#else
    #include <hf-risc.h>
#endif

#define DELAY		1000
#define N_TASKS		4

#if defined __x86_64__
#else
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

int main(void)
{
	task0();

	return 0;
}
