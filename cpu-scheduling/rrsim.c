/* Tyrel Hiebert
 * CSC 360 - Assignment 3
 * July 14, 2018
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

#define MAX_BUFFER_LEN 80
#define IDLE 1
#define DISPATCHING 2
#define PROCESSING 3
#define PROCESS_EXIT 4

taskval_t *event_list = NULL;
int state = 1;

// void print_task(taskval_t *t, void *arg) {
void print_task(taskval_t *t) {
    printf("task %03d: %5d %3.2f %3.2f\n",
        t->id,
        t->arrival_time,
        t->cpu_request,
        t->cpu_used
    );  
}


void increment_count(taskval_t *t, void *arg) {
    int *ip;
    ip = (int *)arg;
    (*ip)++;
}


void run_simulation(int quantum, int dispatch) {
    taskval_t *ready_q = NULL;
    int tick = 0;


    if (event_list) {
        taskval_t *next_arrival = peek_front(event_list);
        taskval_t *current_arrival = NULL;
        taskval_t *process = NULL;
        int d, q;
        // while not finished all tasks
        while (next_arrival || ready_q || process) {
            // int current_task_id = next_arrival->id;
            #ifdef DEBUG
                printf("    DEBUG: Next task to arrive at time %d\n", next_arrival==NULL ? 0 : next_arrival->arrival_time);
                printf("    DEBUG: Current time is %d\n", tick);
            #endif

            // check if any new tasks have arrived
            if (next_arrival && next_arrival->arrival_time <= tick) {
                current_arrival = next_arrival;
                next_arrival = next_arrival->next;
                if (next_arrival) {
                    event_list = remove_front(event_list);
                }
                else {
                    #ifdef DEBUG
                        printf("    DEBUG: No more tasks in event list.\n");
                    #endif
                }
                ready_q = add_end(ready_q, current_arrival);
                #ifdef DEBUG
                    printf("    DEBUG: Added task %d to ready queue.\n", current_arrival->id);
                #endif
                
            }

            // PROCESS

            // if idle remove task from front of ready q
            if (state == IDLE) {
                #ifdef DEBUG
                    printf("    DEBUG: In idle state.\n");
                #endif
                process = peek_front(ready_q);
                if (process) {
                    ready_q = remove_front(ready_q);
                    d = dispatch - 1;
                    if (d > 0) {
                        #ifdef DEBUG
                            printf("    DEBUG: Entering dispatch state.\n");
                        #endif
                        printf("[%05d] DISPATCHING\n", tick);
                        state = DISPATCHING;
                    }
                    else if (d == 0) {
                        #ifdef DEBUG
                            printf("    DEBUG: Dispatch time was 1. Dispatched and now entering processing state.\n");
                        #endif
                        printf("[%05d] DISPATCHING\n", tick);
                        state = PROCESSING;
                        q = quantum;
                    }
                    else {
                        #ifdef DEBUG
                            printf("    DEBUG: Dispatch time is 0, skip dispatching.\n");
                        #endif
                        state = PROCESSING;
                        q = quantum;
                    }
                }
                else {
                    printf("[%05d] IDLE\n", tick);
                }
            }
            else if (state == DISPATCHING) {
                #ifdef DEBUG
                    printf("    DEBUG: In dispatch state\n");
                #endif
                if (d > 0) {
                    printf("[%05d] DISPATCHING\n", tick);
                }
                if (--d == 0) {
                    #ifdef DEBUG
                        printf("    DEBUG: Dispatch state finished. Enter processing state.\n");
                    #endif
                    state = PROCESSING;
                    q = quantum;
                }
            }
            else if (state == PROCESSING) {
                #ifdef DEBUG
                    printf("    DEBUG: In processing state\n");
                #endif
                if (q > 0) {
                    #ifdef DEBUG
                        printf("    DEBUG: Quantum remaining: %d.\n", q);
                    #endif
                    if (process->cpu_used < process->cpu_request) {
                        printf("[%05d] id=%05d req=%.2f used=%.2f\n",
                            tick,
                            process->id,
                            process->cpu_request,
                            process->cpu_used
                        );
                        process->cpu_used++;
                    }
                    else {
                        #ifdef DEBUG
                            printf("    DEBUG: Process used more cpu than requested. Exit process.\n");
                        #endif
                        state = IDLE;
                        
                        process->finish_time = tick;
                        int ta = process->finish_time - process->arrival_time;
                        int w = ta - process->cpu_request;
                        printf("[%05d] id=%05d EXIT w=%.2d ta=%.2d\n",
                            tick,
                            process->id,
                            w,
                            ta
                        );
                        tick--;
                        end_task(process);
                        process = NULL;
                    }
                }
                else if (q <= 0 && process->cpu_used < process->cpu_request) {
                    #ifdef DEBUG
                        printf("    DEBUG: Quantum expired and process requires more time.\n");
                    #endif
                    // print_task(process);
                    ready_q = add_end(ready_q, process);
                    process = NULL;
                    state = IDLE;
                    tick--;
                    #ifdef DEBUG
                        printf("    DEBUG: Process added back to ready queue.\n    DEBUG: Next process in ready queue:\n    ");
                        ready_q==NULL ? printf("None.\n") : print_task(ready_q);
                        // if (ready_q == NULL) {
                        //     printf("None.\n");
                        // }
                        // else {
                        //     print_task(ready_q);
                        // }
                    #endif
                }
                else if (q <= 0 && process->cpu_used > process->cpu_request) {
                    #ifdef DEBUG
                        printf("    DEBUG: Process used more cpu than requested AND quantum expired on same tick. Exit process.\n");
                    #endif
                    state = IDLE;
                    
                    process->finish_time = tick;
                    int turnAround = process->finish_time - process->arrival_time;
                    int waitTime = turnAround - process->cpu_request;
                    printf("[%05d] id=%05d EXIT w=%.2d ta=%.2d\n",
                        tick,
                        process->id,
                        waitTime,
                        turnAround
                    );
                    tick--;
                    end_task(process);
                    process = NULL;
                }
                q--;

            }
            else {
                // error must have occurred
                fprintf(stderr, "An error has occurred. State not recognized.\n");
            }
            tick++;
        }
        #ifdef DEBUG
            printf("    DEBUG: Reached end of while.\n");
        #endif
    }
    else {
        fprintf(stderr, "There were no tasks to run.\n");
    }
}


int main(int argc, char *argv[]) {
    char   input_line[MAX_BUFFER_LEN];
    int    i;
    int    task_num;
    int    task_arrival;
    float  task_cpu;
    int    quantum_length = -1;
    int    dispatch_length = -1;

    taskval_t *temp_task;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--quantum") == 0 && i+1 < argc) {
            quantum_length = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--dispatch") == 0 && i+1 < argc) {
            dispatch_length = atoi(argv[i+1]);
        }
    }

    if (quantum_length == -1 || dispatch_length == -1) {
        fprintf(stderr, 
            "usage: %s --quantum <num> --dispatch <num>\n",
            argv[0]);
        exit(1);
    }


    while(fgets(input_line, MAX_BUFFER_LEN, stdin)) {
        sscanf(input_line, "%d %d %f", &task_num, &task_arrival,
            &task_cpu);
        temp_task = new_task();
        temp_task->id = task_num;
        temp_task->arrival_time = task_arrival;
        temp_task->cpu_request = task_cpu;
        temp_task->cpu_used = 0.0;
        event_list = add_end(event_list, temp_task);
    }

#ifdef DEBUG
    int num_events;
    apply(event_list, increment_count, &num_events);
    printf("DEBUG: # of events read into list -- %d\n", num_events);
    printf("DEBUG: value of quantum length -- %d\n", quantum_length);
    printf("DEBUG: value of dispatch length -- %d\n", dispatch_length);
#endif

    run_simulation(quantum_length, dispatch_length);

    return (0);
}
