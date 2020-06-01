//
// Created by tarsio on 12/05/2020.
//

#ifndef SCHEDULER_RT_H
#define SCHEDULER_RT_H

#endif //SCHEDULER_SYS_H

#include "sys.h"

//static jmp_buf *rt_jmp;
//static int rt_running_id;

extern jmp_buf *rt_jmp;
extern int rt_running_id;


// reserve some stack space
//#define RT_RESERVE_SPACE volatile char cushion[1000]
// don't optimize my cushion away
//#define RT_DONT_OPTIMIZE  cushion[0] = '@'


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

int rt_get_self_id();
const char * rt_get_self_name();
int rt_get_self_period();
int rt_get_self_capacity();
int rt_get_self_deadline();
int rt_get_self_state();
int rt_get_self_executed();
void rt_context_switch_circular();
void rt_context_switch();
void rt_start();
void rt_create();
void rt_setjmp();
void rt_idle_function();
int rt_del_task(int id);
int rt_add_task(void (*function), int period, int capacity, int deadline, char *name, int state);
const int * rt_get_states();
const int * rt_get_ids();
int rt_task_count();

#if defined __riscv
    void rt_clock();
    void timer1ctc_handler(void);
#endif
void rt_idle_function(void);