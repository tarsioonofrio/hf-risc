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


#define SYS_BUFFER	2
#define DELAY		1000
#define N_TASKS		3

#if defined __riscv
    typedef uint32_t jmp_buf[20];
    int32_t setjmp(jmp_buf env);
    void longjmp(jmp_buf env, int32_t val);
#endif


static jmp_buf * rt_jmp;
static int rt_running;

#if defined __riscv
    char mem_pool[8192];
#endif

// reserve some stack space
#define RT_RESERVE_SPACE volatile char cushion[1000]
// don't optimize my cushion away
#define RT_DONT_OPTIMIZE  cushion[0] = '@'

void rt_idle_function();

enum enum_state{READY, RUNNING, BLOCKED, DONE, SYS} rt_state;

typedef struct{
    void (*_function)();
    int _id;
    int _period;
    int _execute;
    int _deadline;
    int state;
    int computed;
} rt_task;


int rt_time;
rt_task *rt_running_task;
rt_task *rt_idle_task;
struct list *rt_list_task;
struct list *rt_list_blocked;

void rt_context_switch_circular()
{
	if (!setjmp(rt_jmp[rt_running])) {
        rt_running++;
		if (list_count(rt_list_task) == rt_running)
            rt_running = 0;
		longjmp(rt_jmp[rt_running], 1);
	}
}

void rt_context_switch(){
    int index=0;
    int best_index=0, best_value=65535;
    rt_task *task, *_task;
    task = (rt_task *)malloc(sizeof(rt_task));
    _task = (rt_task *)malloc(sizeof(rt_task));

    /*
     * TODO after this remove rt_running, unnecessary
     * TODO make running_task global
     */

    setjmp(rt_jmp[0]);
    best_index = rt_running;
    best_value = rt_running_task->_period;

    /*
     *  TODO Move change state from ready to blocked to end this procedure, if this change continue at this point
     *  TODO is possible to change one task from running to block and to ready in unique execution of
     *  TODO this scheduler.
     */

    if (rt_running_task->state != SYS) {
        if (rt_running_task->computed == rt_running_task->_execute) {
            rt_running_task->state = DONE;
            rt_running_task = rt_idle_task;
            rt_running = -1;
            best_index = -1;
            best_value = 65535;
        }
    }

    /*
     * TODO optimize this loop, not is necessary call list_get, i can get node, get next node, and
     * TODO call function of node, in next pass next to node etc
     */
    for (index = 0; index < list_count(rt_list_task); index++) {
        task = list_get(rt_list_task, index);
        if (task->state == BLOCKED)
            continue;
        if (task->state == DONE){
//            if ((rt_time == 1) && (task->_period > 1))
//                continue;
            if (rt_time % task->_period != 0)
                continue;
            task->computed = 0;
            task->state = READY;
        }
        if (task->_period < best_value) {
            best_value = task->_period;
            best_index = index;
            rt_running_task = task;
        }
    }

    rt_running = rt_running_task->_id;
    rt_running_task->state = RUNNING;
    rt_running_task->computed++;


    rt_time++;
    // jump 0 is rt_context_switch, jump 1 is rt_task_idle
    longjmp(rt_jmp[rt_running + SYS_BUFFER], 1);
}

void rt_init() {
    int count = list_count(rt_list_task);
    rt_task *task, *running_task;
    task = (rt_task *) malloc(sizeof(rt_task));
    running_task = (rt_task *) malloc(sizeof(rt_task));

    rt_jmp = malloc((list_count(rt_list_task) + 2)* sizeof(jmp_buf));
    rt_running_task = (rt_task *)malloc(sizeof(rt_task));
    rt_idle_task = (rt_task *)malloc(sizeof(rt_task));

    rt_idle_task->_id = -1;
    rt_idle_task->_period = 65535;
    rt_idle_task->_execute = -1;
    rt_idle_task->_function = rt_idle_function;
    rt_idle_task->state = SYS;
    rt_running_task = rt_idle_task;

    rt_running = 1;
    rt_time = 0;

    if (!setjmp(rt_jmp[0]))
        rt_idle_function();

    if (rt_running < count + 1){
        /*
         * TODO optimize this loop, not is necessary call list_get, i can get node, get next node, and
         * TODO call function of node, in next pass next to node etc
         * TODO after this remove rt_running, unnecessary because
         */

        rt_running++;
        running_task = list_get(rt_list_task, rt_running - SYS_BUFFER);
        running_task->_function();
    } else{
        rt_running = 0;
        rt_context_switch();
    }
}


void rt_idle_function(void) {
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */

    if (!setjmp(rt_jmp[1]))
        longjmp(rt_jmp[0], 1);

    while (1) {
        /* thread body */

//        printf("rt_task_idle...%d\n", rt_running);
        printf("_\n");
        delay_ms(DELAY);
        rt_context_switch();
    }
}


int rt_add_task(int id, int period, int execute, void (*f), int state) {
    rt_task *task;
    task = (rt_task *)malloc(sizeof(rt_task));
    task->_id = id;
    task->_period = period;
    task->_execute = execute;
    task->_function = f;
    task->state = state;
    list_append(rt_list_task, task);
}



void f2(void) {
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */

    if (!setjmp(rt_jmp[rt_running]))
        longjmp(rt_jmp[0], 1);

    while (1) {
        /* thread body */

//        printf("\rtask 2...%d\n", rt_running);
        printf("2\n");
        delay_ms(DELAY);
        rt_context_switch();
    }
}

void f1(void) {
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */

    if (!setjmp(rt_jmp[rt_running]))
        longjmp(rt_jmp[0], 1);

	while (1) {
	    /* thread body */

//		printf("\rtask 1...%d\n", rt_running);
        printf("1\n");
		delay_ms(DELAY);
        rt_context_switch();
    }
}

void f0(void)
{
	volatile char cushion[1000];	/* reserve some stack space */
	cushion[0] = '@';		/* don't optimize my cushion away */

    if (!setjmp(rt_jmp[rt_running]))
        longjmp(rt_jmp[0], 1);

	while (1) {
	    /* thread body */

//		printf("\rtask 0...%d\n", rt_running);
        printf("0\n");
		delay_ms(DELAY);
        rt_context_switch();
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

        heap_init((uint32_t *)&mem_pool, sizeof(mem_pool));
    #endif
//    RT_VAR;

    /*
     * TODO Put list initializer in one function;
     */
    rt_list_task = list_init();
    rt_list_blocked = list_init();


    rt_add_task(0, 4, 1, f0, READY);
    rt_add_task(1, 5, 2, f1, READY);
    rt_add_task(2, 7, 2, f2, READY);

    rt_init();

	return 0;
}
