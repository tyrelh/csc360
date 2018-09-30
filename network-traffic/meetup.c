/*
 * Hiebert, Tyrel
 * V00898825
 * CSC 360
 * Assignment 2
 * Meetup
 */

/*Required Headers*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "meetup.h"
#include "resource.h"

/*
 * Codeword must be stored in meetup.c as an instance of resource_t when the
 * first of n join_meetup() calls are made. After n join_meetup() calls are
 * made, the stored codeword will be given to all n people, n and the 
 * codeword will be reset.
 * 
 * Via a command line flag, can switch between the first of n having the
 * codeword, or the last of n having the codeword.
 * 
 * $ ./myserver --meetup 4 --meetlast
 * or
 * $ ./myserver -- meetup 2 --meetfirst
 * 
 * n is given as a command line argument.
 * 
 * Server process runs until Ctrl-C terminates it.
 */

/*
 * Declarations for barrier shared variables, plus concurrency-control
 * variables, must START here.
 */
static resource_t codeword;
static int codewordLen;
sem_t mutex;
sem_t turnstile;
sem_t turnstile2;
static int num;
static int count;
static int mode;
// static int debug1;
// static int debug2;

/*
 * Any code for initializing syncronization constructs or other shared data
 * needed for the correct operation of your meetup solution must appear in
 * this function. The first parameter is the value of N to be used for all 
 * calls to join_meetup(); the second parameter has one of two values: 
 * MEET_FIRST or MEET_LAST. MEET_FIRST IS 1, MEET_LAST is 0.
 */
void initialize_meetup(int n, int mf) {
    // char label[100];
    // int i;

    if (n < 1) {
        fprintf(stderr, "Who are you kidding?\n");
        fprintf(stderr, "A meetup size of %d??\n", n);
        exit(1);
    }

    /*
     * Initialize the shared structures, including those used for
     * synchronization.
     */
    if (sem_init(&mutex, 0, 1) != 0) {
        fprintf(stderr, "Semaphore 'mutex' could not init.\n");
        exit(1);
    }
    if (sem_init(&turnstile, 0, 0) != 0) {
        fprintf(stderr, "Semaphore 'turnstile' could not init.\n");
        exit(1);
    }
    if (sem_init(&turnstile2, 0, n) != 0) {
        fprintf(stderr, "Semaphore 'turnstile2' could not init.\n");
        exit(1);
    }

    // init codeword resource
    init_resource(&codeword, "CODEWORD");

    // init control vars
    num = n;
    count = 0;
    mode = mf;
}


/*
 * If appropriate for this call of join_meetup(), use write_resource() to copy
 * the contents of the char array referred to parameter value into the codeword
 * resource variable. If too few threads have arrived to allow all to proceed,
 * then block the caller of join_meetup(). If enough have already arrived, then
 * use read_resource() to copy the contents of the codeword resource into char
 * array referred to by parameter value. (The len field is the size of the char 
 * array passed in the first argument.)
 */
void join_meetup(char *value, int len) {

    // only allow n threads through turnstile2
    sem_wait(&turnstile2);

    // increment counter as threads come in
    sem_wait(&mutex);
        // get codeword from first thread
        if (count == 0 && mode == MEET_FIRST) {
            write_resource(&codeword, value, len);
            codewordLen = len;
        }
        count++;
    sem_post(&mutex);

    // if final thread has arrived, post turnstile1
    if (count == num) {
        // get codeword from final thread
        if (mode == MEET_LAST) {
            write_resource(&codeword, value, len);
            codewordLen = len;
        }
        // open turnstile1 for this group of n threads
        sem_post(&turnstile);
    }

    // block n-1 threads at sturnstile1
    sem_wait(&turnstile);
    sem_post(&turnstile);

    // give codeword to all n passing threads
    read_resource(&codeword, value, codewordLen);

    // decrement counter as threads are leaving
    sem_wait(&mutex);
        count--;
        // the last thread will reset turnstiles
        if (count == 0) {
            // block turnstile1
            sem_wait(&turnstile);
            int i;
            // open turnstile2 for next group of n threads
            // this is hapening in mutex so next group will block at top mutex after turnstile2
            for (i = 0; i < num; i++) {
                sem_post(&turnstile2);
            }
        }
    sem_post(&mutex);

}
