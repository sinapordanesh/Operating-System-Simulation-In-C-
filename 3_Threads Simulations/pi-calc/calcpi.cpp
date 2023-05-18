// ======================================================================
// Modified by Saman Pordanesh (saman.pordanesh@ucalgary.ca)
// ======================================================================
//
// count_pi() calculates the number of pixels that fall into a circle
// using the algorithm explained here:
//
// https://en.wikipedia.org/wiki/Approximations_of_%CF%80
//
// count_pixels() takes 2 paramters:
//  r         =  the radius of the circle
//  n_threads =  the number of threads you should create
//
// Currently the function ignores the n_threads parameter. Your job is to
// parallelize the function so that it uses n_threads threads to do
// the computation.

#include "calcpi.h"
#include <thread>
#include <iostream>



// structure which we will pass to each thread
struct Task {
    double start_x, end_x;
    uint64_t partial_count;
};

// 256 will the the maximum number of threads...
// ...So, we are creating this at start statically
Task tasks[256];


// thread function which receives a task struct, radius(r), and
// square of the radius(rsq)
static void threadFunction(Task * task, int r, double rsq){

    // local variable to keep partial count and assign it to the...
    // ...appropriate structure at the end of the function.
    uint64_t localCount = 0; 

    // this loop is the parallel loop through different threads
    for(double x = task->start_x; x <= task->end_x ; x ++)
        for( double y = 0 ; y <= r ; y ++)
            if( x*x + y*y <= rsq) localCount ++;
    task->partial_count = localCount;
}

// main function for counting pixels
// here we will devide the interval of radius by number of threads,
// creating task objects and join them finally
uint64_t count_pixels(int r, int n_threads)
{
    double rsq = double(r) * r;
    uint64_t count = 0; // total count
    std::thread threads[n_threads]; // array of threads

    for (int i = 0; i < n_threads; ++i) {

        // Define appropriate start_x for each thread
        if (i == 0){
            tasks[i].start_x = 1; // interval should start from 1
        } else {
            tasks[i].start_x = ((i/(double)n_threads) * (r)) + 1; // formula to calc start of the N th interval
        }        

        // Define appropriate end_x for each thread
        tasks[i].end_x = ((i+1)/(double)n_threads) * (r); // formula to calc end of the N th interval

        threads[i] = std::thread(threadFunction, &tasks[i], r, rsq); // creating i th thread and pass its values
    }

    
    // Join Threads
    for (int i = 0; i < n_threads; ++i) {
        if (threads[i].joinable()){
            threads[i].join();
        }

        // std::cout << "Count for: " << i << " is: " << tasks[i].partial_count << std::endl;
        count += tasks[i].partial_count; // calc the final value of the count from different partial values
    }

    return count * 4 + 1;
}