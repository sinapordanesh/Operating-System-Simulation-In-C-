// this is the only file you should modify and submit for grading

#include "scheduler.h"
#include "common.h"
#include <iostream>
#include <climits>


// this is the function you should implement
//
// runs Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrivals, and bursts
// output:
//   seq[] - will contain the execution sequence but trimmed to max_seq_len size
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//

using namespace std;

void simulate_rr(
    int64_t quantum,
    int64_t max_seq_len,
    std::vector<Process> & processes,
    std::vector<int> & seq
) {

    int64_t currentTime = 0;
    int64_t nextTime = 0;
    vector<int> rq;
    vector<int64_t> jq;
    vector<int64_t> remainingBursts;


    int nextProcessId = 0; // next arriving process
    vector<int> tempSeq; // temporary sequence tracker

    for (auto & process : processes) {
        // Collect all processes burst time in an array,
        // Array index == process ID
        remainingBursts.push_back(process.burst);

        // Collect all processes arriving time in an array,
        // Array index == process ID
        jq.push_back(process.arrival);
    }


    while(1){

        // Starting of the four different conditions that can occur

        //---- First condition ----
        if (rq.empty() && (size_t)nextProcessId >= jq.size()  ){ // When rq empty and jq empty
            break ;
            //---- Second condition ----
        } else if((rq.empty()) && ((size_t)nextProcessId < jq.size())){ // When rq empty, but jq not empty

            if (jq[nextProcessId] > currentTime) { // CPU is in idle mode
                tempSeq.push_back(-1);
                currentTime = jq[nextProcessId]; // jump the time to the next arrival time
            }

            while ((size_t)nextProcessId < jq.size() && jq[nextProcessId] == currentTime){
                rq.push_back(nextProcessId); // adding arrival processes to the rq
                nextProcessId++; // move the pointer to the next process in the jq
            }

            //---- Third condition ----
        } else if((!rq.empty()) && ((size_t)nextProcessId >= jq.size())) { // When rq not empty, but jq empty

            nextTime = currentTime;

            bool mpAlgorithm = true; // use multiprocess algorithm if true
            int64_t  smallestRemainingBurst = LLONG_MAX;
            vector<int> repeatingSequencePattern; // chain of sequence which is repeating each time

            for (size_t i = 0; i < rq.size(); ++i) {
                if (remainingBursts[rq[i]] <= quantum){
                    mpAlgorithm = false;
                    break ;
                }
                if (remainingBursts[rq[i]] < smallestRemainingBurst){
                    smallestRemainingBurst = remainingBursts[rq[i]];
                }
                repeatingSequencePattern.push_back(rq[i]); // creating the sequence pattern
            }

            if (mpAlgorithm){ // when the multiprocess algorithm is applicable

                int64_t timeCoefficient = ((smallestRemainingBurst - quantum) / quantum); // coefficient of remaining quantum's to the end of the shortest process (remove one quantum to make sure won't be finish during mpAlgorithm)
                int64_t shareOfTimeCoefficient = (timeCoefficient / (int64_t)rq.size());

                for (size_t i = 0; i < rq.size(); ++i) { // run one loop to record start time of any process
                    if (remainingBursts[rq[i]] == processes[rq[i]].burst){ // record start time for the current process, if it starts
                        processes[rq[i]].start_time = nextTime; // here "nextTime" is an exception (figure it out)
                    }
                    remainingBursts[rq[i]] -= quantum; // reduce each process's remaining burst time by a quantum
                    tempSeq.push_back(rq[i]); // record the process which was proceed
                    nextTime += quantum; // move next time to the start of the next quantum time
                }

                shareOfTimeCoefficient--; // decrement the coefficient by one as we did once above

                if (shareOfTimeCoefficient >= 1){
                    for (size_t i = 0; i < rq.size(); ++i) {
                        remainingBursts[rq[i]] -= (quantum * shareOfTimeCoefficient); // reduce each process's remaining burst time by multiple quantum
                        nextTime += (quantum * shareOfTimeCoefficient); // move next time to the start of the next multiple quantum time
                    }

                    if (rq.size() > 1) {
                        for (int i = 0; i < shareOfTimeCoefficient && tempSeq.size() <= (size_t)max_seq_len; ++i) { // insert repeating pattern of sequences by number of quantum coefficient to the main sequence recorder
                            tempSeq.insert(tempSeq.end(), repeatingSequencePattern.begin(), repeatingSequencePattern.end());
                        }
                    }
                }

                currentTime = nextTime; // move the current time to the estimated next time

            } else{ // when the multiprocess algorithm is NOT applicable

                if (remainingBursts[rq[0]] >= quantum){ // in this case process will continue after quantum time
                    nextTime = currentTime + quantum; // move current time by a quantum

                    if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                        processes[rq[0]].start_time = currentTime;
                    }
                    remainingBursts[rq[0]] -= quantum; // reduce a quantum from the process remaining burst time
                    tempSeq.push_back(rq[0]); // track the current process
                    if (remainingBursts[rq[0]] > 0){
                        rq.push_back(rq[0]); // adding to the tail
                    } else{
                        processes[rq[0]].finish_time = nextTime; // record the finishing time of the process
                    }
                    rq.erase(rq.begin()); // removing from the head of the queue

                    currentTime = nextTime; // move current time to the end time of the current process
                } else{ // in this case, process will finish earlie than quantum

                    int64_t remainingToTheFinish = remainingBursts[rq[0]]; // remaining time to the end of the process <= quantum

                    nextTime = currentTime + remainingToTheFinish; // move current time to the finishing time of the process

                    if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                        processes[rq[0]].start_time = currentTime;
                    }
                    remainingBursts[rq[0]] -= remainingToTheFinish; // reduce time from the process remaining burst time
                    tempSeq.push_back(rq[0]); // track the current process
                    if (remainingBursts[rq[0]] == 0){
                        processes[rq[0]].finish_time = nextTime; // record the finishing time of the process
                        rq.erase(rq.begin()); // removing from the head of the queue
                    }

                    currentTime = nextTime; // move current time to the end time of the current process
                }
            }


            //---- Fourth condition ----
        }else if ((!rq.empty()) && ((size_t)nextProcessId < jq.size())){ // When rq not empty and jq not empty

            nextTime = currentTime + quantum;
            bool arrive = false; //check if something arrive during period
            bool finish = false; // check if the current process finishes early before end of the period

            if (jq[nextProcessId] <= nextTime){
                arrive = true;
            }

            if (remainingBursts[rq[0]] <= quantum){ // if current process will finish at the end or earlier than quantum
                finish = true;
            }

            if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                processes.at(rq.at(0)).start_time = currentTime;
            }

            if (!arrive && !finish){ // nothing arrive, process won't finish early
                nextTime = currentTime;
//                                                /*

                if (rq.size() == 1) {
                    int64_t quantumToTheEndCurrentP = (remainingBursts[rq[0]])
                        / quantum; // number of quantum to the finish of the current process
                    int64_t quantumToTheNextP = (jq[nextProcessId] - nextTime)
                        / quantum; // number of quantum to the arrival of the next process

                    if (remainingBursts[rq[0]]
                        == processes[rq[0]]
                               .burst) { // record start time for the current process, if it starts
                        processes[rq[0]].start_time = currentTime;
                    }

                    if (quantumToTheEndCurrentP < quantumToTheNextP) {
                        nextTime += remainingBursts[rq[0]];
                        remainingBursts[rq[0]]
                            = 0; // reduce time from the process remaining burst time
                        tempSeq.push_back(rq[0]); // track the current process
                        if (remainingBursts[rq[0]] == 0) {
                            processes[rq[0]].finish_time
                                = nextTime; // record the finishing time of the process
                            rq.erase(rq.begin()); // removing from the head of the queue
                        }
                    } else {
                        nextTime += quantumToTheNextP * quantum;
                        remainingBursts[rq[0]] -= quantumToTheNextP
                            * quantum; // reduce time from the process remaining burst time
                        tempSeq.push_back(rq[0]); // track the current process
                        if (remainingBursts[rq[0]] == 0) {
                            processes[rq[0]].finish_time
                                = nextTime; // record the finishing time of the process
                            rq.erase(rq.begin()); // removing from the head of the queue
                        }
                    }

                    currentTime = nextTime; // move the current time to the next time

                } else {
                    bool mpAlgorithm = true; // use multiprocess algorithm if true
                    int64_t  smallestRemainingBurst = LLONG_MAX; //shortest remaining burst time among different processes
                    int64_t  limitOfRepeating = 0; // the max period of time which we will be able to use mpAl
                    vector<int> repeatingSequencePattern; // chain of sequence which is repeating each time

                    for (size_t i = 0; i < rq.size(); ++i) { // check whether we can use mpAl or not
                        if (remainingBursts[rq[i]] <= quantum){
                            mpAlgorithm = false;
                            break ;
                        }
                        if (remainingBursts[rq[i]] < smallestRemainingBurst){
                            smallestRemainingBurst = remainingBursts[rq[i]];
                        }
                        repeatingSequencePattern.push_back(rq[i]); // creating the sequence pattern
                    }

                    if (mpAlgorithm){ // when the multiprocess algorithm is applicable
                        // determine the limit of algorithm

                        if (smallestRemainingBurst <= (jq[nextProcessId] - nextTime)){ // finishing a process
                            limitOfRepeating = smallestRemainingBurst;
                        } else { // arriving a new process
                            limitOfRepeating = (jq[nextProcessId] - nextTime);
                        }

                        int64_t timeCoefficient = ((limitOfRepeating -  quantum) / quantum); // coefficient of remaining quantum's to the end of the shortest process (remove a quantum to make sure we won't have any finishing during mpAlgorithm)
                        int64_t shareOfTimeCoefficient = (timeCoefficient / (int64_t)rq.size());

                        for (size_t i = 0; i < rq.size(); ++i) { // run one loop to record start time of any process
                            if (remainingBursts[rq[i]] == processes[rq[i]].burst){ // record start time for the current process, if it starts
                                processes[rq[i]].start_time = nextTime; // here "nextTime" is an exception (figure it out)
                            }
                            remainingBursts[rq[i]] -= quantum; // reduce each process's remaining burst time by a quantum
                            tempSeq.push_back(rq[i]); // record the process which was proceed
                            nextTime += quantum; // move next time to the start of the next quantum time
                        }

                        shareOfTimeCoefficient--; // decrement the coefficient by one as we did once above

                        if (shareOfTimeCoefficient >= 1){
                            for (size_t i = 0; i < rq.size(); ++i) {
                                remainingBursts[rq[i]] -= (quantum * shareOfTimeCoefficient); // reduce each process's remaining burst time by multiple quantum
                                nextTime += (quantum * shareOfTimeCoefficient); // move next time to the start of the next multiple quantum time
                            }

                            if (rq.size() > 1){
                                for (int i = 0; i < shareOfTimeCoefficient && tempSeq.size() <= (size_t)max_seq_len; ++i) { // insert repeating pattern of sequences by number of quantum coefficient to the main sequence recorder
                                    tempSeq.insert(tempSeq.end(), repeatingSequencePattern.begin(), repeatingSequencePattern.end());

                                }
                            }
                        }

                        currentTime = nextTime; // move the current time to the estimated next time

                    } else {
                        nextTime += quantum; // move current time by a quantum

                        if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                            processes[rq[0]].start_time = currentTime;
                        }
                        remainingBursts[rq[0]] -= quantum; // reduce a quantum from the process remaining burst time
                        tempSeq.push_back(rq[0]); // track the current process
                        if (remainingBursts[rq[0]] > 0){
                            rq.push_back(rq[0]); // adding to the tail
                        }
                        rq.erase(rq.begin()); // removing from the head of the queue

                        currentTime = nextTime; // move current time to the end time of the current process
                    }
                }


//                */
                                /*
                nextTime += quantum; // move current time by a quantum

                if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                    processes[rq[0]].start_time = currentTime;
                }
                remainingBursts[rq[0]] -= quantum; // reduce a quantum from the process remaining burst time
                tempSeq.push_back(rq[0]); // track the current process
                if (remainingBursts[rq[0]] > 0){
                    rq.push_back(rq[0]); // adding to the tail
                }
                rq.erase(rq.begin()); // removing from the head of the queue
                currentTime = nextTime;
                                */

            } else if (!arrive && finish){ // nothing arrive, process finishes early

                int64_t remainingToTheFinish = remainingBursts[rq[0]]; // remaining time to the end of the process <= quantum
                nextTime = currentTime + remainingToTheFinish; // move current time to the finishing time of the process

                if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                    processes[rq[0]].start_time = currentTime;
                }
                remainingBursts[rq[0]] -= remainingToTheFinish; // reduce time from the process remaining burst time
                tempSeq.push_back(rq[0]); // track the current process
                if (remainingBursts[rq[0]] == 0){
                    processes[rq[0]].finish_time = nextTime; // record the finishing time of the process
                    rq.erase(rq.begin()); // removing from the head of the queue
                }

                currentTime = nextTime; // move current time to the end time of the current process

            } else if (arrive && !finish){// something arrives, process won't finish early

                nextTime = currentTime + quantum; // move current by quantum as the process will not finish early

                while((size_t)nextProcessId < jq.size() && jq[nextProcessId] < nextTime){
                    rq.push_back(nextProcessId);
                    nextProcessId++;
                }

                if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                    processes[rq[0]].start_time = currentTime;
                }
                remainingBursts[rq[0]] -= quantum; // reduce the burst time of the current process by quantum
                tempSeq.push_back(rq[0]); // track the current process
                if (remainingBursts[rq[0]] > 0){ // always should be true
                    rq.push_back(rq[0]); // adding to the tail
                }
                rq.erase(rq.begin()); // removing from the head of the queue

                if ((size_t)nextProcessId < jq.size() && jq[nextProcessId] == nextTime){ // if new process arrives at the end of the quantum
                    rq.push_back(nextProcessId); // adding new process, after adding previous process
                    nextProcessId++; // move pointer to the next process in the job queue
                }

                currentTime = nextTime; // move current time to the end time of the current process

            } else if (arrive && finish){ // something arrive, process finishes early
                int64_t remainingToTheFinish = remainingBursts[rq[0]]; // remaining time to the end of the process <= quantum
                nextTime = currentTime + remainingToTheFinish; // move current time to the finishing time of the process

                while ((size_t)nextProcessId < jq.size() && jq[nextProcessId] <= nextTime){ // if a process arrives before or at finishing time of the current P
                    rq.push_back(nextProcessId);
                    nextProcessId++;
                }

                if (remainingBursts[rq[0]] == processes[rq[0]].burst){ // record start time for the current process, if it starts
                    processes[rq[0]].start_time = currentTime;
                }
                remainingBursts[rq[0]] -= remainingToTheFinish; // reduce time from the process remaining burst time
                tempSeq.push_back(rq[0]); // track the current process
                if (remainingBursts[rq[0]] == 0){ // this condition should be always true
                    processes[rq[0]].finish_time = nextTime; // record the finishing time of the process
                    rq.erase(rq.begin()); // removing from the head of the queue
                }

                currentTime = nextTime; // move current time to the next time (after finishing)
            }
        }
    }

    seq.clear();
    if (!tempSeq.empty()){
        seq.push_back(tempSeq.at(0));
        for (int i = 1; (size_t)i < tempSeq.size(); ++i) {
            if (seq.size() == (size_t)max_seq_len){
                break ;
            }
            if (tempSeq.at(i) == tempSeq.at(i - 1)){
                continue ;
            } else{
                seq.push_back(tempSeq.at(i));
            }
        }
    }
}
