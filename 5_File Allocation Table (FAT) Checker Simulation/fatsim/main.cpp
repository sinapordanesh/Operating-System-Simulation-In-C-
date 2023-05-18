/// =========================================================================
/// Copyright (C) 2022 Pavol Federl (pfederl@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// =========================================================================
/// DO NOT EDIT THIS FILE. DO NOT SUBMIT THIS FILE FOR GRADING.

#include "fatsim.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

namespace {
struct Timer {
  // return elapsed time (in seconds) since last reset/or construction
  // reset_p = true will reset the time
  double elapsed(bool resetFlag = false)
  {
    double result = 1e-6
        * std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::steady_clock::now() - start)
              .count();
    if (resetFlag) reset();
    return result;
  }
  // reset the time to 0
  void reset() { start = std::chrono::steady_clock::now(); }
  Timer() { reset(); }

  private:
  std::chrono::time_point<std::chrono::steady_clock> start;
};
}; // anon namespace

int main(int argc, char **)
{
  if (argc != 1) {
    std::cerr << "Thanks for all the command line arguments.\n"
              << "But I don't like any of them... Bye.\n";
    exit(-1);
  }

  std::vector<long> fat;
  long maxnum = 0;
  while (1) {
    long num;
    if (1 != scanf("%ld", &num)) break;
    if (num < -1) {
      std::cerr << "I don't like your FAT, it's too negative.\n";
      exit(-1);
    }
    maxnum = std::max(maxnum, num);
    fat.push_back(num);
  }

  if (long(fat.size()) <= maxnum) {
    std::cerr << "Some of your FAT entries are too big.\n";
    exit(-1);
  }

  std::cout << "FAT has " << fat.size() << " entries.\n";

  Timer t;
  std::vector<long> result = fat_check(fat);
  auto elapsed = t.elapsed();

  std::string line = "blocks:";
  for (auto n : result) {
    line += " " + std::to_string(n);
    if (line.size() > 80) {
      printf("%s\n", line.c_str());
      line = " ";
    }
  }
  if (line.size() > 1) std::cout << line << "\n";
  std::cout << "elapsed time: " << std::fixed << std::setprecision(2) << elapsed << "s\n";

  return 0;
}
