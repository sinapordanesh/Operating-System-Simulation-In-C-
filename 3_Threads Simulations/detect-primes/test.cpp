#include <pthread.h>
#include <iostream>
#include <unistd.h>

pthread_barrier_t barrier;

void * routine(void * ptr){
    // PARALLEL ================================================
    std::cout << "Waiting for barrier...\n";
    sleep(1);
    pthread_barrier_wait(&barrier);
    std::cout << "Passed the barrier.\n";
    return NULL;
}

int main() {
    pthread_t th[3];

    // Initialize Barrier
    pthread_barrier_init(&barrier, NULL, 3);

    // Create Threads
    for (int i = 0; i < 3; i++) {
        if (0 != pthread_create(&th[i], NULL, &routine, NULL)) {
            std::cerr << "Error creating thread";
        }
    }
    
    // Join Threads
    for (int i = 0; i < 3; i++) {
        if (0 != pthread_join(th[i], NULL)) {
            std::cerr << "Error joining thread";
        }
    }
    
    // Destroy Barrier
    pthread_barrier_destroy(&barrier);

    return 0;
}