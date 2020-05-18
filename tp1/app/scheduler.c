/*
 * example of coroutines using setjmp()/longjmp()
 */

/*
 TODO
 Preemption
 Set start set jmp process
 wait interruption, when this occur we have two options:
 0. in interruption handler define next function, return to current function and
    save context, jump to new function.
    How force to save context when returning of interruption?
    Without this is necessary wait until the next save context point in sequential execution.
 1. NO I CANT DO THAT:
    jump to next function.
    because i will be locked in interruption,
    i need to go out from interruption
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
#define N_TASKS		3

#if defined __riscv
    typedef uint32_t jmp_buf[20];
    int32_t setjmp(jmp_buf env);
    void longjmp(jmp_buf env, int32_t val);
#endif


static jmp_buf rt_jmp[N_TASKS];
static int rt_curr;

#if defined __riscv
    char mem_pool[8192];
#endif

// reserve some stack space
#define RT_RESERVE_SPACE volatile char cushion[1000]
// don't optimize my cushion away
#define RT_DONT_OPTIMIZE  cushion[0] = '@'


void task0();
void task1();
void task2();
void rt_task();

int rt_time;

typedef struct{
    void (*_function)();
    int _period;
    int _execute;
    int _deadline;
    int state;
    int computed;
} rt_parameter;

enum enum_state{READY, RUNNING, BLOCKED} rt_state;

struct list *rt_list_function;
struct list *rt_list_parameter;
struct list *rt_list_state;

struct list *rt_list_ready;
struct list *rt_list_running;
struct list *rt_list_blocked;

int rt_vec_period[N_TASKS];
int rt_vec_execute[N_TASKS];
int rt_vec_deadline[N_TASKS];

int rt_vec_state[N_TASKS];
int rt_vec_computed[N_TASKS];

void rt_context_switch_circular()
{
	if (!setjmp(rt_jmp[rt_curr])) {
        rt_curr++;
		if (list_count(rt_list_function) == rt_curr)
            rt_curr = 0;
		longjmp(rt_jmp[rt_curr], 1);
	}
}

void rt_context_switch()
{
    int index=0, value=0, count=0;
    int best_index=-1, best_value=65535;
    count = list_count(rt_list_parameter);
    rt_parameter *param, *best_param;
    param = (rt_parameter *)malloc(sizeof(rt_parameter));
    best_param = (rt_parameter *)malloc(sizeof(rt_parameter));

    /*
     * TODO use three list one for each off: READY, RUNNING and BLOCKED or do three loops
     * TODO with this flow is possible to set two different values to one task
     */

    if (!setjmp(rt_jmp[rt_curr])) {
        best_index = rt_curr;
        best_value = rt_vec_period[rt_curr];
        best_param = list_get(rt_list_parameter, rt_curr);
        best_value = best_param->_period;

//        if (rt_vec_computed[rt_curr] == rt_vec_execute[rt_curr]){
//            rt_vec_state[rt_curr] = BLOCKED;
//            best_index=-1;
//            best_value=65535;
//        }

        if (best_param->computed == best_param->_execute){
            best_param->state = BLOCKED;
            best_index=-1;
            best_value=65535;
        }


//        for (index=0; index < count; index++) {
//            if (rt_vec_state[index] != READY)
//                continue;
//            if (rt_vec_period[index] < best_value) {
//                best_value = rt_vec_period[index];
//                best_index = index;
//            }
//        }

        /*
         * TODO Replace list by other list with only READY tasks.
         */

        for (index = 0; index < list_count(rt_list_parameter); index++) {
            param = list_get(rt_list_parameter, index);
            if (param->state != READY)
                continue;
            if (param->_period < best_value) {
                best_value = param->_period;
                best_index = index;
                best_param = param;
            }

        }

        /*
         * TODO Add loop to work with BLOCK tasks. Change this state to READY
         */
//        rt_vec_state[best_index] = RUNNING;
//        rt_vec_computed[best_index]++;
//        rt_curr = best_index;

        best_param->state = RUNNING;
        best_param->computed++;
        rt_curr = best_index;

        rt_time++;
        longjmp(rt_jmp[rt_curr], 1);
    }
}


void rt_task()
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */
//    rt_curr = N_TASKS - 1;		/* the first thread to context switch is this one */
//	setjmp(rt_jmp[N_TASKS - 1]);
	setjmp(rt_jmp[rt_curr]);

	while (1) {			/* thread body */
        rt_context_switch();
		printf("rt task...%d\n", rt_curr);
		delay_ms(DELAY);
    }
}


void rt_task_test(void)
{
    volatile char cushion[1000];	/* reserve some stack space */
    cushion[0] = '@';		/* don't optimize my cushion away */

    void (*func)();
    if (!setjmp(rt_jmp[rt_curr])) {
        if (rt_curr < list_count(rt_list_parameter) -1) {
            rt_curr++;
            func = list_get(rt_list_function, rt_curr);
            func();
        }
    }

    while (1) {			/* thread body */
        rt_context_switch();
        printf("rt_task_test id: %d\n", rt_curr);
        delay_ms(DELAY);
    }
}


void task2(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */
    void (*func)();
	if (!setjmp(rt_jmp[rt_curr])) {
	    if (rt_curr < list_count(rt_list_parameter) -1) {
            rt_curr++;
            func = list_get(rt_list_function, rt_curr);
            func();
        }
    }

	while (1) {			/* thread body */
        rt_context_switch();

		printf("\rtask 2...%d\n", rt_curr);
		delay_ms(DELAY);
    }
}

void task1(void) {
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */

    void (*func)();
    if (!setjmp(rt_jmp[rt_curr])) {
        if (rt_curr < list_count(rt_list_parameter) -1) {
            rt_curr++;
            func = list_get(rt_list_function, rt_curr);
            func();
        }
    }

	while (1) {			/* thread body */
        rt_context_switch();

		printf("\rtask 1...%d\n", rt_curr);
		delay_ms(DELAY);
    }
}

void task0(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

    void (*func)();
    if (!setjmp(rt_jmp[rt_curr])) {
        if (rt_curr < list_count(rt_list_parameter) -1) {
            rt_curr++;
            func = list_get(rt_list_function, rt_curr);
            func();
        }
    }

	while (1) {			/* thread body */
        rt_context_switch();

		printf("\rtask 0...%d\n", rt_curr);
		delay_ms(DELAY);
    }
}


#if defined __riscv
    void timer1ctc_handler(void)
    {
        printf("TIMER1CTC %d\n", TIMER0);
    }

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
#endif



int main(void)
{
    #if defined __riscv
        TIMER1PRE = TIMERPRE_DIV256;

        // Set time cycle between 5 ms and 50 ms
        /* TIMER1 base frequency: 8.190 ms or 8190 khz (cycles) */
        /* 8190 / 256 = 32 */
    //    TIMER1CTC = 32;
        TIMER1CTC = 32768;

        /* TIMER1 alt frequency: 100k cycles */
        /* 100000 / 256 = 390.625 */
    //    TIMER1OCR = 391;

        /* unlock TIMER1 for reset */
        TIMER1 = TIMERSET;
        TIMER1 = 0;

        /* enable interrupt mask for TIMER1 CTC and OCR events */
        TIMERMASK |= MASK_TIMER1CTC;
    //    TIMERMASK |= MASK_TIMER1CTC | MASK_TIMER1OCR;
    //    for(;;);
        heap_init((uint32_t *)&mem_pool, sizeof(mem_pool));
    #endif
//    RT_VAR;
    void (*func)();
    rt_list_function = list_init();
    rt_list_parameter = list_init();

    rt_parameter *param, *param1, *param2;
    param = (rt_parameter *)malloc(sizeof(rt_parameter));
    param1 = (rt_parameter *)malloc(sizeof(rt_parameter));
    param2 = (rt_parameter *)malloc(sizeof(rt_parameter));

    if(list_append(rt_list_function, task0)) printf("FAIL!");
    rt_vec_period[0] = 4;
    rt_vec_execute[0] = 1;
    rt_vec_state[0] = READY;
    param->_period = 4;
    param->_execute = 1;
    param->_function = task0;
    param->state = READY;
    list_append(rt_list_parameter, param);

    if(list_append(rt_list_function, task1)) printf("FAIL!");
    rt_vec_period[1] = 5;
    rt_vec_execute[1] = 2;
    rt_vec_state[1] = READY;
    param1->_period = 5;
    param1->_execute = 2;
    param1->_function = task1;
    param1->state = READY;
    list_append(rt_list_parameter, param1);

    if(list_append(rt_list_function, task2)) printf("FAIL!");
    rt_vec_period[2] = 7;
    rt_vec_execute[2] = 2;
    rt_vec_state[2] = READY;
    param2->_period = 7;
    param2->_execute = 2;
    param2->_function = task2;
    param2->state = READY;
    list_append(rt_list_parameter, param2);

//    if(list_append(rt_list_function, rt_task_test)) printf("FAIL!");

    func = list_get(rt_list_function, 0);

//    rt_last_second = time(NULL);
    func();
	return 0;
}
