//
// Created by tarsio on 17/05/2020.
//

void delay_ms(int milliseconds)
{
    long pause;
    clock_t now,then;
    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while((now-then) < pause)
        now = clock();
}

void delay_ms (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

void delay_ms(int delay)
{
    long pause,  tick=0;
    pause = CLOCKS_PER_SEC;
    for (tick=0; tick < pause*delay; tick++);
}

void sigalrm_handler(int sig)
{
    // This gets called when the timer runs out.  Try not to do too much here;
    // the recommended practice is to set a flag (of type sig_atomic_t), and have
    // code elsewhere check that flag (e.g. in the main loop of your program)
    printf("INTERRUPTION \n");
//    printf( "Signal catcher called for signal %d\n", sig );
//    alarm(1);
//    signal(SIGALRM, &sigalrm_handler);  // set a signal handler
}


int main(void) {

    void (*func)();

    struct sigaction s_act;
    sigemptyset( &s_act.sa_mask );
    s_act.sa_flags = 0;
    s_act.sa_handler = sigalrm_handler;
    sigaction(SIGALRM, &s_act, NULL );
//    signal(SIGALRM, &sigalrm_handler);  // set a signal handler
    alarm(1);
    return 0
}