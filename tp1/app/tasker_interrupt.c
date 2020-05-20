/*
 * example of coroutines using setjmp()/longjmp()
 */

#include <hf-risc.h>

#define N_TASKS		4

typedef uint32_t jmp_buf[20];
int32_t setjmp(jmp_buf env);
void longjmp(jmp_buf env, int32_t val);

static jmp_buf jmp[N_TASKS];
static int cur;

void context_switch()
{
    printf("CUR ...%d\n", cur);
    if (!setjmp(jmp[cur])) {
        cur++;
        if (N_TASKS == cur)
            cur = 0;
        printf("CUR SET ...%d\n", cur);
        longjmp(jmp[cur], 1);
    }
}

void timer1ctc_handler(void)
{
    printf("TIMER1CTC %d\n", TIMER0);
    context_switch();
}


void idle_task()
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */
	cur = N_TASKS - 1;		/* the first thread to context switch is this one */
	

	while (1) {			/* thread body */
//		context_switch();
        cur++;
        printf("idle task ...%d\n", cur);
        setjmp(jmp[3]);

		delay_ms(100);
	}
}

void task2(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

	while (1) {			/* thread body */
//		context_switch();
        cur++;
        printf("task2 ...%d\n", cur);
        if (!setjmp(jmp[2]))
            idle_task();

		delay_ms(100);
	}
}

void task1(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

	while (1) {			/* thread body */
//		context_switch();
        cur++;
        printf("task1 ...%d\n", cur);
        if (!setjmp(jmp[1]))
            task2();

		delay_ms(100);
	}
}

void task0(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

	while (1) {			/* thread body */
//		context_switch();
        cur++;
        printf("task0 ...%d\n", cur);
        if (!setjmp(jmp[0]))
            task1();

		delay_ms(100);
	}
}

int main(void)
{
    TIMER1PRE = TIMERPRE_DIV256;
    TIMER1CTC = 32768;
    /* unlock TIMER1 for reset */
    TIMER1 = TIMERSET;
    TIMER1 = 0;
    /* enable interrupt mask for TIMER1 CTC events */
    TIMERMASK |= MASK_TIMER1CTC;

    task0();
    
	return 0;
}
