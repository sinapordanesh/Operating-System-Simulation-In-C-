// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2021, Saman Pordanesh, saman.pordanesh@ucalgary.ca
// Do not distribute this file.

#include "fatsim.h"
#include <iostream>
#include <stack>
#include <utility>


// reimplement this function
std::vector<long> fat_check(const std::vector<long> & fat)
{

  std::vector<std::vector<long>> adjacencyLists (fat.size()+1);
  std::stack<std::pair<long, long>> stack;

  long pointed = -2;
  for (long i = 0; (size_t)i < fat.size(); ++i) {
    pointed = fat[i];
    adjacencyLists[pointed + 1].push_back(i+1);
  }

  stack.push(std::pair(0, 0));


  long bestDistance = 0;
  std::vector<long> result;
  std::pair<long, long> temp;

  while (!stack.empty()){
    temp = stack.top();
    stack.pop();

    if (temp.second > bestDistance){
      bestDistance = temp.second;
      result.clear();
      result.push_back(temp.first);
    } else if (temp.second == bestDistance && temp.second != 0){
      result.push_back(temp.first);
    }

    for (long i = 0; (size_t)i < adjacencyLists[temp.first].size(); ++i) {
      stack.push(std::pair(adjacencyLists[temp.first][i], temp.second + 1));
    }
  }

  for (long i = 0; (size_t)i < result.size(); ++i) {
    result[i]--;
  }

  return result;
}