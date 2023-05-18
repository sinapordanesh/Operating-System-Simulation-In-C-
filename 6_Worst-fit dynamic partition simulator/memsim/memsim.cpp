/// -------------------------------------------------------------------------------------
/// this is the only file you need to edit
/// -------------------------------------------------------------------------------------
///
/// (c) 2022, Saman Pordanesh, Saman.pordanesh@ucalgary.ca
/// Do not distribute this file.

#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <set>
#include <unordered_map>
#include <cmath>


struct Partition{
  int tag;
  int64_t size, addr;

  Partition(int tag, int64_t size, int64_t addr){
    this->tag = tag;
    this->size = size;
    this->addr = addr;
  }
};

typedef std::list<Partition>::iterator PartitionRef;

struct scmp{
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const{
    if (c1->size == c2->size)
      return c1->addr < c2->addr;
    else
      return c1->size > c2->size;
  }
};

// I recommend you implement the simulator as a class. This is only a suggestion.
// If you decide not to use this class, feel free to remove it.
struct Simulator {

  int64_t pageSize;

  int64_t pageNumber; // for debugging

  // all partitions, in a linked list
  std::list<Partition> allBlocks;
  // quick access to all tagged partitions
  std::unordered_map<long, std::vector<PartitionRef>> taggedBlocks;
  // sorted partitions by size/address
  std::set<PartitionRef,scmp> freeBlocks;


  Simulator(int64_t page_size)
  {
    // constructor
    this->pageSize = page_size;
    pageNumber = 0;
    this->allBlocks = std::list<Partition>();
    this->taggedBlocks = std::unordered_map<long, std::vector<PartitionRef>>();
    this->freeBlocks = std::set<PartitionRef, scmp>();
  }

  int64_t pageEstimator(int64_t size){
    int64_t extraPage = size % this->pageSize;

    if (extraPage == 0){
      return size / this->pageSize;
    } else{
      return  (size/ this->pageSize) + 1;
    }
  }

  void addToTheEnd(){

  }

  void allocate(int tag, int size)
  {
    // Pseudocode for allocation request:
    // - search through the list of partitions from start to end, and
    //   find the largest partition that fits requested size
    //     - in case of ties, pick the first partition found
    // - if no suitable partition found:
    //     - get minimum number of pages from OS, but consider the
    //       case when last partition is free
    //     - add the new memory at the end of partition list
    //     - the last partition will be the best partition
    // - split the best partition in two if necessary
    //     - mark the first partition occupied, and store the tag in it
    //     - mark the second partition free

    if (size == 0){
      return ;
    }

    if (allBlocks.empty()){
      int64_t requiredPages = pageEstimator(size); // number of pages that we need for the first allocation
      this->pageNumber += requiredPages; // add new requested pages to the total pages recorder
      int64_t requiredSize = requiredPages * pageSize; // size which will be provided by new pages
      int64_t remainedSize = requiredSize - size; // total provided size - needed size for allocation
      Partition newPartition = Partition(tag, size, 0); // creating new allocation partition
      allBlocks.push_back(newPartition); // add the new partition to the end (start as well in this case) of the list
      this->taggedBlocks[tag].push_back(allBlocks.begin()); // record partition address based on its tag to the allocated partitions map
      if (remainedSize != 0){ // if any free space remained after allocation
        Partition freePortion = Partition (-1, remainedSize, size); // create a new free partition to put on the list
        allBlocks.push_back(freePortion); // add the free portion at the end of the list
        freeBlocks.insert(std::prev(allBlocks.end())); // put the free block address on the set of free blocks addresses
      }
      return ;
    }

    PartitionRef targetItr = allBlocks.end();
    if (!freeBlocks.empty()){
      if (freeBlocks.begin()->operator->()->size >= size){
        targetItr = *freeBlocks.begin();
      }
    }

    if (targetItr != allBlocks.end()){ // suitable free portion exists on the linked list

      freeBlocks.erase(freeBlocks.begin()); // delete the first element of the free blocks set (means we found a suitable free portion to allocate or new partition )

      if (size == targetItr->size){ // free portion with same requested size
        targetItr->tag = tag; // tag placed
        taggedBlocks[tag].push_back(targetItr); // record node address on the hashmap to the belonging tag
        return ;

      } else{ // free portion bigger than requested size
        Partition newPartition = Partition(tag, size, targetItr->addr); //new partition node
        allBlocks.insert(targetItr, newPartition); // allocate new node before iterator
        taggedBlocks[tag].push_back(std::prev(targetItr)); // record the node address to its belongign tag (note that we use std::prev, as created node is located before the targetItr, when we insert it)
        targetItr->size = targetItr->size - size; // reduce the size of the free portion by requested size
        targetItr->addr = std::prev(targetItr)->addr + std::prev(targetItr)->size; // change the remaining free portion's address
        freeBlocks.insert(targetItr); // insert the new free portion with new size and address to the free blocks set
        return ;
      }

    } else{ // when we didn't find any suitable free portion for allocation; targetItr = allBlocks.end()

      int64_t availableSize = 0; // temp variable
      if (std::prev(targetItr)->tag == -1){ // if the las node is free, but not enough for the partition

        freeBlocks.erase(freeBlocks.find(std::prev(targetItr))); // delete the free portion from the set (not suitable free portion but will use in the new allocation at the end of the linked list)

        availableSize = std::prev(targetItr)->size;
        allBlocks.erase(std::prev(targetItr)); // erasing the free portion and allocate its size to the new partition
      }

      int64_t requiredPages = pageEstimator(size - availableSize); // calc needed pages - by considering the previous free portion
      this->pageNumber += requiredPages;
      int64_t providedFreeSize = (requiredPages * pageSize) + availableSize; // provided free size by page requesting and previous free portion
      int64_t remainedSize = providedFreeSize - size; // calc remained size after allocation

      Partition newPartition = Partition (tag, size, std::prev(targetItr)->addr + std::prev(targetItr)->size); // creating new partition for allocation
      allBlocks.push_back(newPartition); // inserting new partition before targetItr
      taggedBlocks[tag].push_back(std::prev(allBlocks.end())); // as we did push back in the previous line, it should be last node of the linked list

      targetItr = allBlocks.end(); // set targetIt to the end of the linked list again, as by previous pushback it was changed to something else
      if (remainedSize != 0 ){ // if still free portion remained
        Partition freePortion = Partition (-1, remainedSize, std::prev(targetItr)->addr + std::prev(targetItr)->size); // creating new free portion and put it after allocated memory
        allBlocks.push_back(freePortion); // insert remained free portion at the end of the linked list
        freeBlocks.insert(std::prev(allBlocks.end()));
      }
    }
  }

  void deallocate(int tag)
  {
    // Pseudocode for deallocation request:
    // - for every partition
    //     - if partition is occupied and has a matching tag:
    //         - mark the partition free
    //         - merge any adjacent free partitions

    while (!taggedBlocks[tag].empty()){ // loop through all partitions belong to the target tag
      auto targetItr = taggedBlocks[tag].back(); // take the address of the partition from the end of the vector
      taggedBlocks[tag].pop_back();  // remove the partition's address from the end of the vector

      targetItr->tag = -1; // make the partition free by changing its tag to -1

      if (targetItr != allBlocks.begin() && std::prev(targetItr)->tag == -1 && targetItr->tag == -1){ // check previous partition, if it's free to merge
        targetItr->size += std::prev(targetItr)->size; // change the size
        targetItr->addr = std::prev(targetItr)->addr; // change the address
        freeBlocks.erase(freeBlocks.find(std::prev(targetItr))); // delete free the portion from set because of merging
        allBlocks.erase(std::prev(targetItr)); // erase the previous free partition
      }

      if (targetItr != allBlocks.end() && std::next(targetItr)->tag == -1 && targetItr->tag == -1){ // check next partition, if it's free to merge
        targetItr->size += std::next(targetItr)->size;
        freeBlocks.erase(freeBlocks.find(std::next(targetItr))); // delete the free portion from set because of merging
        allBlocks.erase(std::next(targetItr));
      }

      freeBlocks.insert(targetItr); // insert the address of the newly gotten free partition to the set of all free partition's addresses.

    }
  }
  MemSimResult getStats()
  {
    // let's guess the result... :)
    MemSimResult result;
    if (!freeBlocks.empty()){
      result.max_free_partition_size = freeBlocks.begin()->operator->()->size;
      result.max_free_partition_address = freeBlocks.begin()->operator->()->addr;
    } else{
      result.max_free_partition_size = 0;
      result.max_free_partition_address = 0;
    }
    result.n_pages_requested = pageNumber;
    return result;
  }
  void check_consistency()
  {
    // mem_sim() calls this after every request to make sure all data structures
    // are consistent. Since this will probablly slow down your code, you should
    // disable comment out the call below before submitting your code for grading.
    check_consistency_internal();
  }
  void check_consistency_internal()
  {
    // you do not need to implement this method at all - this is just my suggestion
    // to help you with debugging

    // here are some suggestions for consistency checks (see appendix also):

    // make sure the sum of all partition sizes in your linked list is
    // the same as number of page requests * page_size

    // make sure your addresses are correct

    // make sure the number of all partitions in your tag data structure +
    // number of partitions in your free blocks is the same as the size
    // of the linked list

    // make sure that every free partition is in free blocks

    // make sure that every partition in free_blocks is actually free

    // make sure that none of the partition sizes or addresses are < 1
  }
};

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  // if you decide to use the simulator class above, you probably do not need
  // to modify below at all
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
    sim.check_consistency();
  }
  return sim.getStats();
}
