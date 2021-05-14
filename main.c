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

void decrement(void) { 
    printf("In func1\n"); 
    setcontext(&c0);
}

struct ThreadInfo {
 ucontext_t context;
 int state;
 int count_val; 
 };

struct ThreadInfo thread_array[6];

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
    printf("schedule");
}

int main()
{   
    signal(SIGALRM, PWF_scheduler);
    alarm(2);
    sleep(3);
    initializeThread(1, 5, &c1);
    createThread(&c1);
    getcontext(&c0);
    printf("%p, %p", c1.uc_link, &c0);
    runThread(&c1);
    runThread(&c1);
    runThread(&c1);
    printf("exiting");
    free(c1.uc_stack.ss_sp);
    return 0;
}