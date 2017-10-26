#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

/* maximum time in seconds to sleep*/
#define MAX_SLEEP_TIME 3
/*number of potential students*/
#define NUM_OF_STUDENTS 4
#define NUM_OF_HELPS 2
/*number of available seats*/
#define NUM_OF_SEATS 2

/*mutex declarations, a global variable*/
pthread_mutex_t mutex_lock;

/*semaphore declarations, global variables*/
sem_t students_sem;
sem_t ta_sem;
/*the number of waiting students, a global variable*/
int waiting_students;

// function for ta thread to run
void * ta_run(void * arg){
    // check if ta is busy variable
    int sval;
    
    // number for ta's helping time
    int help_time = 3;
    while (true){
        //ta sleeping is ta semaphore waiting student to wake him up
        printf("TA sleeping\n");
        sem_wait(&ta_sem);
        // let the students know the ta is sleeping
        sem_post(&students_sem);
        // lock the mutex to decrement the number of waiting students
        pthread_mutex_lock(&mutex_lock);
        //inside critical section
        waiting_students--;
        // unlock mutex so other threads could wait and increase num of waiting students
        pthread_mutex_unlock(&mutex_lock);
        printf("Helping a student for %i seconds, # of waiting students = %i\n", help_time, waiting_students);
        sleep((unsigned)help_time);
    }
    return (void*)0;
}

//function for student thread to run
void * student_run(void * arg){
    // number for student
    int * student_number = (int*)arg;
    printf("student number %i\n", *student_number);
    // seeder for random sleep function
    int seeder = *student_number;
    // times student has been helped
    int helped = 0;
    // check if ta is busy
    int ta_busy = 0;
    // seed is the pointer to seeder
    int * seed = &seeder;
    // sleep time randomly generated later
    int sleep_time = 0;
    // check if student is waiting
    int waiting = 0;
    
    // if student number is over max sleep time, default seeder is 1
    if (*student_number > MAX_SLEEP_TIME)
        *seed = 1;
    
    while(helped < NUM_OF_HELPS){
        if (waiting_students < NUM_OF_SEATS && !waiting){
            // acquire mutex lock
            pthread_mutex_lock(&mutex_lock);
            // increase number of waiting students shared between threads
            waiting_students++;
            // unlock for other threads to access variable
            pthread_mutex_unlock(&mutex_lock);
            printf("\t Student %i takes a seat, # of waiting students = %i\n", *student_number, waiting_students);
            // check if ta is sleeping
            sem_getvalue(&ta_sem,&ta_busy);
            waiting = 1;
            // if ta is sleeping then wake him up
            if (!ta_busy)
                sem_post(&ta_sem);
            // wait for ta to respond
            sem_wait(&students_sem);
            printf("Student %i receiving help\n", *student_number);
            helped++;
            //if (helped > NUM_OF_HELPS)
                //return (void*)0;
        }
        else{
            // student is not waiting student is sleeping
            waiting = 0;
            // generate random number to sleep
            sleep_time = (rand_r((unsigned*)seed) % MAX_SLEEP_TIME) + 1;
            sleep((unsigned)sleep_time);
            printf("\t\t Student %i will try again later\n", *student_number);
            printf("Student %i programming for %i seconds\n", *student_number, sleep_time);
        }   
    }
    
    return (void*)0;
}

int main(int argc, char** argv) {
    // pthreads for ta and students
    pthread_t ta;
    pthread_t students[NUM_OF_STUDENTS];
    
    //initiate mutex lock
    pthread_mutex_init(&mutex_lock,NULL);
    // initiate ta and students semaphores
    sem_init(&students_sem,0,0);
    sem_init(&ta_sem,0,0);
    
    //create ta thread
    pthread_create(&ta, NULL, ta_run, NULL);
    
    printf("CS149 SleepingTA from Nhat Trinh\n");
    
    // create student threads and create array to instantiate student numbers
    int student_number[NUM_OF_STUDENTS];
    for (int i = 0; i < NUM_OF_STUDENTS; i++){
        student_number[i] = i + 1;
        pthread_create(&students[i], NULL, student_run, &student_number[i]);
    }
    
    // join the student threads, not ta
    for (int j = 0; j < NUM_OF_STUDENTS; j++){
        pthread_join(students[j],NULL);
    }
    
    // finally cancel ta thread
    pthread_cancel(ta);
    
    // for every mutex init there must be mutex destroy
    pthread_mutex_destroy(&mutex_lock);
    // close all semaphores
    sem_close(&students_sem);
    sem_close(&ta_sem);
    
    return (EXIT_SUCCESS);
}

