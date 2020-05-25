
#if defined __riscv
    #include <hf-risc.h>
    #define PRINT_LINE  printf("%d\n", __LINE__);
#else
    #include <stdio.h>
    #include <stdlib.h>
    #include <setjmp.h>
    #include <unistd.h>
    #include <string.h>
    #define delay_ms(x) sleep(x/1000)
    #define PRINT_LINE  ;
#endif

#include "list.h"

// DONT CHANGE
#define SYS_BUFFER	2

// if we use delay or not
#define DELAY       0
// time of delay
#define DELAY_TIME	100
// log type or what is printed,
// 0 = no log, 1 = task id, 2 = task name
#define LOG		    2
// if use timer ctc in riscv or not
#define TIMER       1

#if defined __riscv
    typedef uint32_t jmp_buf[20];
    int32_t setjmp(jmp_buf env);
    void longjmp(jmp_buf env, int32_t val);
    char mem_pool[8192];
#endif


static jmp_buf *rt_jmp;
static int rt_running_id;

// reserve some stack space
//#define RT_RESERVE_SPACE volatile char cushion[1000]
// don't optimize my cushion away
//#define RT_DONT_OPTIMIZE  cushion[0] = '@'

void rt_idle_function();

enum enum_state{READY, RUNNING, BLOCKED, DONE, SYS} rt_state;

typedef struct{
    void (*_function)();
    int _id;
    char *_name;
    int _period;
    int _capacity;
    int _deadline;
    int state;
    int executed;
} rt_task;



int rt_time;
rt_task *rt_running_task;
rt_task *rt_idle_task;
struct list *rt_list_task;
struct list *rt_list_blocked;

int rt_del_task(int id){
    return list_remove(rt_list_task, id);
}

int rt_get_self_id(){
    return rt_running_task->_id;
}

const char * rt_get_self_name(){
    return rt_running_task->_name;
}

int rt_get_self_period(){
    return rt_running_task->_period;
}

int rt_get_self_capacity(){
    return rt_running_task->_capacity;
}

int rt_get_self_deadline(){
    return rt_running_task->_deadline;
}

int rt_get_self_state(){
    return rt_running_task->state;
}

int rt_get_self_executed(){
    return rt_running_task->executed;
}


void rt_context_switch_circular(){
	if (!setjmp(rt_jmp[rt_running_id])) {
        rt_running_id++;
		if (list_count(rt_list_task) == rt_running_id)
            rt_running_id = 0;
		longjmp(rt_jmp[rt_running_id], 1);
	}
}

void rt_context_switch(){
    rt_task *task = (rt_task *)malloc(sizeof(rt_task));

    setjmp(rt_jmp[0]);

    if (rt_running_task->state != SYS) {
        if (rt_running_task->executed == rt_running_task->_capacity) {
            rt_running_task->state = DONE;
            rt_running_task = rt_idle_task;
        }
    }

    for (struct list *element = rt_list_task->next; element != NULL; element = element->next) {
        task = element->elem;
        if (task->state == BLOCKED)
            continue;
        if (task->state == DONE){
            if (rt_time % task->_period != 0)
                continue;
            task->executed = 0;
            task->state = READY;
        }
        if (task->_period < rt_running_task->_period) {
            rt_running_task = task;
        }
    }

    rt_running_id = rt_running_task->_id;
    rt_running_task->state = RUNNING;
    rt_running_task->executed++;

    if (LOG == 3){
        printf("task %s\n", rt_running_task->_name);
    }

    rt_time++;
    // jump 0 is rt_context_switch, jump 1 is rt_task_idle
    longjmp(rt_jmp[rt_running_id + SYS_BUFFER], 1);
}

void rt_start(){
    int count = list_count(rt_list_task);
    rt_task *task, *running_task;
    task = (rt_task *) malloc(sizeof(rt_task));
    running_task = (rt_task *) malloc(sizeof(rt_task));
    struct list * element;

    rt_jmp = malloc((list_count(rt_list_task) + 2)* sizeof(jmp_buf));
    rt_running_task = (rt_task *)malloc(sizeof(rt_task));
    rt_idle_task = (rt_task *)malloc(sizeof(rt_task));

    rt_idle_task->_id = -1;
    rt_idle_task->_name = "rt_idle_task";
    rt_idle_task->_period = 65535;
    rt_idle_task->_capacity = -1;
    rt_idle_task->_function = rt_idle_function;
    rt_idle_task->state = SYS;
    rt_running_task = rt_idle_task;

    rt_running_id = 1;
    rt_time = 0;

    if (!setjmp(rt_jmp[0]))
        rt_idle_function();

    element = rt_list_task->next;
    if (rt_running_id < count + 1){
        rt_running_id++;
        running_task = element->elem;
        running_task->_function();
    }
    rt_running_id = 0;
    PRINT_LINE;
    #if defined __riscv
        if (TIMER){
            rt_clock();
            for (;;);
        } else rt_context_switch();
    #else
        rt_context_switch();
    #endif
}

void rt_create(){
    #if defined __riscv
    heap_init((uint32_t *) &mem_pool, sizeof(mem_pool));
    #endif
    rt_list_task = list_init();
    rt_list_blocked = list_init();
    rt_running_id = 0;
}


int rt_add_task(void (*function), int period, int capacity, int deadline, char *name, int state){
    if ((period == 0) || (capacity == 0))
        return 0;
    if(period < capacity)
        return 0;
    rt_task *task;
    task = (rt_task *)malloc(sizeof(rt_task));
    task->_id = rt_running_id;
    task->_name=(char *)malloc(strlen(name)*sizeof(char));
    strncpy(task->_name ,name,strlen(name));
    task->_period = period;
    task->_capacity = capacity;
    task->_deadline = deadline;
    task->_function = function;
    task->state = state;
    if (list_append(rt_list_task, task)){
        return 0;
    }
    else{
        rt_running_id++;
        return 1;
    }
}

#if defined __riscv
void rt_clock(){
    TIMER1PRE = TIMERPRE_DIV16;

    /* unlock TIMER1 for reset */
    TIMER1 = TIMERSET;
    TIMER1 = 0;

	/* TIMER1 frequency: (39063 * 16) = 250000 cycles (10ms timer @ 25MHz) */
	TIMER1CTC = 15625;

    /* enable interrupt mask for TIMER1 CTC and OCR events */
    TIMERMASK |= MASK_TIMER1CTC;
}

void timer1ctc_handler(void){
    printf("TIMER1CTC %d\n", TIMER0);
//    if (DELAY) delay_ms(DELAY_TIME);
    rt_context_switch();
}
#endif

void rt_idle_function(void){
    volatile char cushion[1000];    /* reserve some stack space */
    cushion[0] = '@';        /* don't optimize my cushion away */

    if (!setjmp(rt_jmp[1])){
        longjmp(rt_jmp[0], 1);
    }

    while (1) {
        /* thread body */
//        printf("rt_task_idle...%d\n", rt_running);
        if ((LOG == 1) || (LOG == 2)) {
            printf("_");
        }
        if (DELAY) delay_ms(DELAY_TIME);
        #if defined __riscv
            if (TIMER ==0) rt_context_switch();
        #else
            rt_context_switch();
        #endif
    }
}


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
