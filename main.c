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
#define SRTF 1
#define PandWF 0

ucontext_t context_array[6];
int running_thread, running_thread_temp, randint, sum;
int * current_value;
int flag = 0, flag_firstRunScheduler = 1, flag_firstRunSelect = 1, finishedNum = 0, counter, common_denominator, min, schedulerType;
int i, j;
int thresholds[5], remainingTime[5];

struct ThreadInfo {
    ucontext_t context;
    int state;
    int count_val;
    int current_val; 
};

struct ThreadInfo thread_array[6];

void printSpace(int arg){
    for (i = 1; i < arg; ++i){ printf(" \t");}
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

    for (i = 0; i < 6-counter; ++i){ printf("  ");}
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

int selectThread (int arg){
    if(arg == PandWF){
        if(flag_firstRunSelect){
            sum = 0;
            for (i = 1; i < 6; ++i){ 
                sum += thread_array[i].count_val;
                thresholds[i-1] = sum;
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
                    selectThread(arg);
                    break;
                }
            }
        }
    }
    else{
        for(i = 1; i < 6; ++i){
            remainingTime[i-1] = thread_array[i].count_val - thread_array[i].current_val;
            if(remainingTime[i-1] != 0){
                running_thread = i;
                // printf("running thread: %d\n", running_thread);
            }
        }

        for(i = 1; i < 6; ++i){
            if((remainingTime[running_thread - 1] > remainingTime [i-1]) && (thread_array[i].state != finished)){
                running_thread = i;
                // printf("running thread: %d\n", running_thread);
            }
        }

    }
}
    


int printShares (void){
    min = thread_array[1].count_val;
    sum = 0;
    for (i = 1; i < 6; ++i){ 
            sum += thread_array[i].count_val;
        }  
    
    for (i = 2; i < 6; ++i){ 
        if(min > thread_array[i].count_val){ min = thread_array[i].count_val;}
    }

    for (i = 1; i <= min ; ++i){ 
        for(j = 1; j<6; ++j){
            if(thread_array[j].count_val % i == 0 ){continue;}
            else{break;}
        }
        if(j == 6){ common_denominator = i;}
    }

    printf("Share:\n");
    for (i = 1; i < 6; ++i){ 
        printf("%d/%d\t", thread_array[i].count_val/common_denominator, sum/common_denominator);
    }
    printf("\n\n");
}

void scheduler (int arg){
    if(flag_firstRunScheduler == 0){ running_thread_temp = running_thread; }
    else { flag_firstRunScheduler = 0;
        printShares();
        printf("Threads:\nT1\tT2\tT3\tT4\tT5\n");
        }
    if(thread_array[running_thread].state == finished){finishedNum ++;}
    if(finishedNum != 5){ 
        selectThread(arg);
        thread_array[running_thread].state = running;
        if(!(running_thread_temp == running_thread)){ printStatus(); }
        runThread(&context_array[running_thread]); 
        if(thread_array[running_thread].state == finished){
            exitThread(&thread_array[running_thread].context);
            // printf("finished state: %d\n", running_thread);
        }
    }
    else {
        running_thread = sizeof(thread_array) + 10;
        printStatus();
    }
    
}

void PWF_scheduler (void){
    scheduler(PandWF);
}

void SRTF_scheduler (void){
    scheduler(SRTF);
}

int main()
{      
    printf("Select P&WF(0) or SRTF(1): ");
    scanf("%d", &schedulerType);
    if(schedulerType != 0 && schedulerType != 1){
        printf("Invalid input\n");
        return 0;
    }
    printf("\n");
   

    int nArray[5];
    printf("Enter 'n' values below for each thread (Even positive integers only):\n");    
    for(i=0; i<5; i++){
        printf("n%d: ", i+1);
        scanf("%d", &nArray[i]);
        if(nArray[i]%2 != 0 || nArray[i] < 2 ){
            printf("Invalid Input\n");
            return 0;
        }
    }
    printf("\n");
    

    srand(time(NULL));
    getcontext(&context_array[0]);
    i = 1;
    while(i < 6){
        initializeThread(i, nArray[i-1], &context_array[i]);
        createThread(&context_array[i], &thread_array[i]);
        i++;
    }
    
    while(finishedNum != 5){
        if(schedulerType = PandWF) {PWF_scheduler();}
        else if( schedulerType = SRTF) {SRTF_scheduler();}
    }
    return 0;
}