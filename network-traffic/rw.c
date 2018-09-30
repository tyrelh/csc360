/*
 * Hiebert, Tyrel
 * V00898825
 * CSC 360
 * Assignment 2
 * Readers/Writers
 */

/*Required Headers*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "rw.h"
#include "resource.h"

/*
 * Declarations for reader-writer shared variables, plus concurrency-control
 * variables, must START here.
 */
pthread_mutex_t lock;
pthread_cond_t read_q;
pthread_cond_t write_q;
int readers;
int writers;
/*
 * The server will control this variable data. It will place a resource
 * there to be read from, or place a resource there to be written to. rw_read() 
 * and rw_write() should operate on this resource_t.
 */
static resource_t data;

/*
 * Any code for initializing syncronization constructs or other variables needed
 * for the correct operation of your readers/writers solution must appear in this
 * function. It is called from within myserver.c.
 */
void initialize_readers_writer() {
    // initialize mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        fprintf(stderr, "\nSomething bad happend.\n");
    }
    // initialize read and write conditions
    if (pthread_cond_init(&read_q, NULL) != 0) {
        fprintf(stderr, "\nSomething bad happend.\n");
    }
    if (pthread_cond_init(&write_q, NULL) != 0) {
        fprintf(stderr, "\nSomething bad happend.\n");
    }
    // initialize number of readers and writers
    readers = 0;
    writers = 0;
    // init resource
    init_resource(&data, "DATA");
}


/*
 * As long as there is no thread writing the resource, this function will read the resource
 * variable data and copy it into value via a call to read_resource(). (The len parameter
 * is the size of the character array passed in as the argument value.) If there are threads
 * writing then the thread calling rw_read() must be blocked until the writer is finished.
 */
void rw_read(char *value, int len) {
    pthread_mutex_lock(&lock);
    // check if there are any threads writing
    while (!(writers == 0)) {
        // if so, wait on condition
        pthread_cond_wait(&read_q, &lock);
    }
    readers++;
    pthread_mutex_unlock(&lock);

    // read from data, to value
    read_resource(&data, value, len);

    pthread_mutex_lock(&lock);
    if (--readers == 0) {
        // signal any writers waiting for resource
        pthread_cond_signal(&write_q);
    }
    pthread_mutex_unlock(&lock);
}


/*
 * As long as there are no threads reading the resource, this function will write the value
 * into the resource variable data via a call to write_resource(). (The len field is the size
 * of the character array passed in as the first argument.) If there are threads reading,
 * then the thread that has called rw_write() must block until the readers are finished.
 */
void rw_write(char *value, int len) {
    pthread_mutex_lock(&lock);
    // check if there are any threads currently reading or writing
    while (!((readers == 0) && (writers == 0))) {
        // if so, wait on condition
        pthread_cond_wait(&write_q, &lock);
    }
    writers++;
    pthread_mutex_unlock(&lock);

    // write to data, from value
    write_resource(&data, value, len);

    pthread_mutex_lock(&lock);
    writers--;
    // signal the next writer and broadcast all readers waiting for resource
    pthread_cond_signal(&write_q);
    pthread_cond_broadcast(&read_q);
    pthread_mutex_unlock(&lock);
}
