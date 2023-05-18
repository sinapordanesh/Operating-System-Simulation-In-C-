/// ============================================================================
/// Copyright (C) 2022 Pavol Federl (pfederl@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// ============================================================================
///
/// You must modify this file and then submit it for grading to D2L.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the detect_primes() function as
/// defined in "detectPrimes.h".

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <iostream>

// C++ barrier class (from lecture notes).
// -----------------------------------------------------------------------------
// You do not have to use it in your implementation. If you don't use it, you
// may delete it.
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



std::vector<int64_t> globalResult;
std::vector<int64_t> numbers;
std::mutex mt;
size_t numsIndex = 0;

// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
static void is_prime()
{
    while (numsIndex < numbers.size()){

        bool isPrime = true;

        mt.lock();
        int64_t n = numbers[numsIndex];
        numsIndex++;
        mt.unlock();

        // std::cout << "Thread for: " << n << std::endl;
        // handle trivial cases
        if (n < 2) continue;
        if (n <= 3) {
            // lock to add a prime number to the global list of founded prime numbers.
            mt.lock();
            globalResult.push_back(n);
            // unlock mutex after adding the number.
            mt.unlock();
            continue;
        }; // 2 and 3 are primes
        if (n % 2 == 0) continue; // handle multiples of 2
        if (n % 3 == 0) continue; // handle multiples of 3
        // try to divide n by every number 5 .. sqrt(n)
        int64_t i = 5;
        int64_t max = sqrt(n);
        while (i <= max) {
            if (n % i == 0) {
                isPrime = false;
                break;
            };
            if (n % (i + 2) == 0) {
                isPrime = false;
                break;
            };;
            i += 6;
        }

        if (isPrime){
            // lock to add a prime number to the global list of founded prime numbers.
            mt.lock();
            globalResult.push_back(n);
            // unlock mutex after adding the number.
            mt.unlock();
            continue;
        }
        
    }
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
// You will most likely need to re-implement this function entirely.
// Note that the current implementation ignores n_threads. Your multithreaded
// implementation must use this parameter.
std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads){

    numbers = nums;
    std::vector<int64_t> result;
    std::thread threads[n_threads];

    for (int i = 0; i < n_threads; i++) {
        threads[i] = std::thread(is_prime);
    }

    for (int i = 0; i < n_threads; ++i) {
        if (threads[i].joinable()){
            threads[i].join();
        }
    }

    result = globalResult;
    return result;
}
