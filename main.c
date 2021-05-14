#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include<unistd.h>

#define STACK_SIZE 4096
#define empty 0
#define ready 1
#define running 2
#define finished 3

ucontext_t c0, c1, c2, c3, c4, c5, context_obj;
int n = 2;

struct ThreadInfo {
 ucontext_t context;
 int state;
 int count_val; 
 };

struct ThreadInfo thread_array[6];



void decrement(void) { 
    printf("%d", thread_array[1].count_val = thread_array[1].count_val - 1);
    setcontext(&c0);
}



void initializeThread (int index, int count_val, ucontext_t *arg){
    thread_array[index].context = *arg;
    thread_array[index].state = ready;
    thread_array[index].count_val = count_val;
}

void createThread (ucontext_t *arg){
    getcontext(arg);
    arg->uc_link = &c0;
    arg->uc_stack.ss_sp = malloc(STACK_SIZE);
    arg->uc_stack.ss_size = STACK_SIZE;
    makecontext(arg, (void (*)(void))decrement, 0);
}

void runThread (ucontext_t *arg){
    swapcontext(&c0, arg);
}

void exitThread (ucontext_t *arg){
    free(arg->uc_stack.ss_sp);
}

void PWF_scheduler (void){
    printf("inside scheduler\n");
    getcontext(&c0);
    runThread(&c1);
    signal(SIGALRM, PWF_scheduler);
    alarm(2);
    sleep(2);
}

int main()
{   
    initializeThread(1, 6, &c1);
    initializeThread(2, 8, &c2);
    initializeThread(3, 10, &c3);
    initializeThread(4, 4, &c4);
    initializeThread(5, 14, &c5);
    createThread(&c1);
    createThread(&c2);
    createThread(&c3);
    createThread(&c4);
    createThread(&c5);
    
    PWF_scheduler();
    printf("exiting");
    free(c1.uc_stack.ss_sp);
    return 0;
}