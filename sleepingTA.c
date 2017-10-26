#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_SLEEP_TIME 3
#define NUM_OF_STUDENTS 4
#define NUM_OF_HELPS 2
#define NUM_OF_SEATS 2

pthread_mutex_t mutex_lock;

sem_t students_sem;
sem_t ta_sem;
int waiting_students;

void * ta_run(void * arg){
    int sval;
    while (true){
        printf("TA sleeping\n");
        sem_wait(&ta_sem);
        sem_post(&students_sem);
        pthread_mutex_lock(&mutex_lock);
        waiting_students--;
        printf("helped a student\n");
        pthread_mutex_unlock(&mutex_lock);
    }
    return (void*)0;
}

void * student_run(void * arg){
    int * seed = (int*)arg;
    int helped = 0;
    int ta_busy = 0;
    while(helped < NUM_OF_HELPS){
        if (waiting_students < NUM_OF_STUDENTS){
            // acquire mutex lock
            pthread_mutex_lock(&mutex_lock);
            // increase number of waiting students shared between threads
            waiting_students++;
            pthread_mutex_unlock(&mutex_lock);
            printf("waiting for TA\n");
            sem_getvalue(&ta_sem,&ta_busy);
            if (!ta_busy)
                sem_post(&ta_sem);
            sem_wait(&students_sem);
            helped++;
            printf("student got help\n");
        }
        else{
            sleep((rand_r((unsigned*)seed) % MAX_SLEEP_TIME) + 1);
            printf("student sleeping\n");
        }   
    }
    printf("student got all helps\n");
    
    return (void*)0;
}

int main(int argc, char** argv) {
    pthread_t ta;
    pthread_t students[NUM_OF_STUDENTS];
    int * seed;
    int seeder;
    
    pthread_mutex_init(&mutex_lock,NULL);
    sem_init(&students_sem,0,0);
    sem_init(&ta_sem,0,0);
    
    pthread_create(&ta, NULL, ta_run, NULL);
    
    for (int i = 0; i < NUM_OF_STUDENTS; i++){
        // seeder moves in the direction of the loop
        // make seed point to address of seeder
        if (seeder == MAX_SLEEP_TIME)
            seeder = 0;
        seed = &seeder;
        seeder++;
        pthread_create(&students[i], NULL, student_run, &seed);
    }
    
    
    for (int j = 0; j < NUM_OF_STUDENTS; j++){
        pthread_join(students[j],NULL);
    }
    
    // for every mutex init there must be mutex destroy
    pthread_mutex_destroy(&mutex_lock);
    sem_close(&students_sem);
    sem_close(&ta_sem);
    
    return (EXIT_SUCCESS);
}

