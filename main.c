#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define STACK_SIZE 4096
#define empty 0
#define ready 1
#define running 2
#define finished 3

ucontext_t context_array[6];
int running_thread, running_thread_temp, randint, sum;
int * current_value;
int flag = 0, flag_firstRunScheduler = 1, flag_firstRunSelect = 1, finishedNum = 0, counter;
int i;
int thresholds[5];

struct ThreadInfo {
    ucontext_t context;
    int state;
    int count_val;
    int current_val; 
};

struct ThreadInfo thread_array[6];

void printSpace(int arg){
    for (i = 1; i < arg; ++i){
        printf(" \t");
        }
}

void increment(void) { 
    current_value = &thread_array[running_thread].current_val;
    sleep(1);
    printSpace(running_thread);
    printf("%d\n", ++(*current_value));
    sleep(1);
    printSpace(running_thread);
    printf("%d\n", ++(*current_value));
    if(thread_array[running_thread].count_val == *current_value){
        thread_array[running_thread].state = finished;
    }
    else{
        thread_array[running_thread].state = ready;
    }
    setcontext(&context_array[0]);
}

void initializeThread (int index, int count_val, ucontext_t *arg){
    thread_array[index].context = *arg;
    thread_array[index].count_val = count_val;
    thread_array[index].current_val = 0;
}

void createThread (ucontext_t *arg, struct ThreadInfo * thread){
    getcontext(arg);
    thread->state = ready;
    arg->uc_link = &context_array[0];
    arg->uc_stack.ss_sp = malloc(STACK_SIZE);
    arg->uc_stack.ss_size = STACK_SIZE;
    makecontext(arg, (void (*)(void))increment, 0);
}

void runThread (ucontext_t *arg){
    swapcontext(&context_array[0], arg);
}

void exitThread (ucontext_t *arg){
    free(arg->uc_stack.ss_sp);
}

void printThisStatus(int arg){
    i = 1;
    flag = 0;
    counter = 0;
    while(i<6){
        if(thread_array[i].state == arg){
            if(!flag){ flag = 1; }
            else{ printf(","); }
            printf("T%d", i);
            counter ++;
        }
        i ++;
    }

    for (i = 0; i < 6-counter; ++i){
        printf("  ");
    }
    
}

void printStatus (void){
    printf("running>");
    printThisStatus(running);
    printf("\tready>");
    printThisStatus(ready);
    printf("\tfinished>");
    printThisStatus(finished);
    printf("\n");
}

int selectThread (void){
    if(flag_firstRunSelect){
        for (i = 1; i < 6; ++i){ 
            sum += thread_array[i].count_val;
            thresholds[i-1] = sum;
            printf("%d\n", sum);
        }
        flag_firstRunSelect = 0;
    }
    randint = rand() % sum;
    for (i = 1; i < 6; ++i){
        if(randint < thresholds[i-1]){
            if(thread_array[i].state != finished){
                running_thread = i;
                break;
            }
            else{
                selectThread();
                break;
            }
        }
    }
}

void scheduler (void){
    if(flag_firstRunScheduler == 0){ running_thread_temp = running_thread; }
    else { flag_firstRunScheduler = 0;
        printf("Threads:\nT1\tT2\tT3\tT4\tT5");
        }
    if(thread_array[running_thread].state == finished){finishedNum ++;}
    if(finishedNum != 5){ 
        selectThread();
        thread_array[running_thread].state = running;
        if(!(running_thread_temp == running_thread)){ printStatus(); }
        runThread(&context_array[running_thread]); 
        }
    else {
        running_thread = sizeof(thread_array) + 10;
        printStatus();
    }
    
}

void PWF_scheduler (void){
    scheduler();
}

void SRTF_scheduler (void){
    scheduler();
}

int main()
{   
    srand(time(NULL));
    
    i = 1;
    while(i < 6){
        initializeThread(i, i*2, &context_array[i]);
        createThread(&context_array[i], &thread_array[i]);
        i++;
    }
    
    while(finishedNum != sizeof(thread_array) - 1){
        PWF_scheduler();
    }
    
    printf("exiting");
    return 0;
}