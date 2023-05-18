/// ============================================================================
/// Copyright (C) 2022 Saman Podanesh (saman.pordanesh@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// ============================================================================

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <atomic>


// C++ barrier class (from lecture notes).
// -----------------------------------------------------------------------------
class simple_barrier {
    std::mutex m_;
    std::condition_variable cv_;
    int n_remaining_, count_;
    bool coin_;

public:
    simple_barrier(int count = 1) { init(count); }
    void init(int count)
    {
        count_ = count;
        n_remaining_ = count_;
        coin_ = false;
    }
    bool wait()
    {
        if (count_ == 1) return true;
        std::unique_lock<std::mutex> lk(m_);
        if (n_remaining_ == 1) {
            coin_ = ! coin_;
            n_remaining_ = count_;
            cv_.notify_all();
            return true;
        }
        auto old_coin = coin_;
        n_remaining_--;
        cv_.wait(lk, [&]() { return old_coin != coin_; });
        return false;
    }
};


// Will be true, when we reach to the end of the array of target numbers
std::atomic<bool> globalFinished(false);

// means I have the answer (false: I don't know, true: I know it's prime or not)
std::atomic<bool> found(false);

// Collect all prime numbers globally here
std::vector<int64_t> globalResult;

// Global array of all target numbers which we want to exemine
std::vector<int64_t> numbers;

// Index of the current target number, which is being exemined, in the target numbers array 
size_t numsIndex = 0;

// An object of the simple_barrier class which will be used as our barrier later to seperate ...
// ... serail parts and parallel ones during thread processing 
simple_barrier barrier;

// First evaluation of a target number to see wheather can be an even number or not 
// It is ran in serial part and once for each target number
void serialPart(int64_t targetNumber){
    // handle trivial cases
    if (targetNumber < 2) {
        found = true;
        return;
    }
    if (targetNumber <= 3) {
        found = true;
        globalResult.push_back(targetNumber);
        return;
    } // 2 and 3 are primes

    if (targetNumber % 2 == 0) {
        found = true;
        return;
    } // handle multiples of 2

    if (targetNumber % 3 == 0){
        found = true;
        return;
    } // handle multiples of 3
}

int64_t targetNumber = 0; // Target number which was taken theough a loop from array of numbers
int64_t max; // = sqrt(targetNumber), global var, dynamic for each target number 


// A function to evaluate the target number with a specific divisor which is passed as a argument
// It will be run in parallel
void parallelPart(int64_t divisor){
    if (targetNumber % divisor == 0) {
        found = true;
        return;
    };
    if (targetNumber % (divisor + 2) == 0) {
        found = true;
        return;
    };
}



// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
static void is_prime(int threadId, int nThread){

    while (globalFinished == 0){

        //===== Serial part
        if (barrier.wait() != 0){
            if (numsIndex == numbers.size()){
                globalFinished = true; //reaches to the end of the inputs array
            } else{
                targetNumber = numbers[numsIndex];
                numsIndex++;
                //By default, "found" should be false
                found = false;
                serialPart(targetNumber);

                // if targetNumber needs more search, we set this vars
                if (!found){
                    max = sqrt(targetNumber); // end of the loop
                }
            }
        }
        //===== Parallel part
        barrier.wait();

        // *** Loop through all prime numbers, until reach to the sqrt(target) -> max
        // The point is that our steps on this loop depends on the number of threads that...
        // ... we have in the program. As a result, each thread will recieve a specific...
        // ... divisor to work with it, which makes programm run faster as we ran this loop...
        // ... in parallel.
        for (int64_t i = 5 + (6 * threadId); i <= max; i += (6*nThread)) {
            if (!found){ // This is useful for thread cancellation 
                parallelPart(i);
            } else {
                break;
            }
        }
        // ===== Serial part
        if (barrier.wait() != 0){
            if (!globalFinished && !found){
                // If not fount, means we failed on determining even or prime. As a result, it's prime
                globalResult.push_back(targetNumber);  
            }
        }

        barrier.wait();
    }
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{
    numbers = nums;
    std::vector<int64_t> result;
    std::thread threads[n_threads];

    // Initialize Barrier
    barrier.init(n_threads);

    // Create Threads
    for (int i = 0; i < n_threads; i++) {
        threads[i] = std::thread(is_prime, i, n_threads);
    }

    // Join Threads
    for (int i = 0; i < n_threads; ++i) {
        if (threads[i].joinable()){
            threads[i].join();
        }
    }

    // Assign the global results to the local one as a return value of the function 
    result = globalResult;
    return result;
}