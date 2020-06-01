//
// Created by tarsio on 26/05/2020.
//

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
// 0 = no log, 1 = task id, 2 = task name, 4 = schedule time
#define LOG		    4
// if use timer ctc in riscv or not
#define TIMER       1

#if defined __riscv
    typedef uint32_t jmp_buf[20];
    int32_t setjmp(jmp_buf env);
    void longjmp(jmp_buf env, int32_t val);
    int32_t _interrupt_set(int32_t s);
    char mem_pool[8192];
#endif
