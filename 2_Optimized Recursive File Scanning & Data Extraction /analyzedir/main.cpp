/// =========================================================================
/// Copyright (C) 2022 Pavol Federl (pfederl@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// =========================================================================
/// DO NOT EDIT THIS FILE. DO NOT SUBMIT THIS FILE FOR GRADING.

#include "analyzeDir.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

void usage(const std::string & pname, int exit_code)
{
  printf("Usage: %s N directory_name\n", pname.c_str());
  exit(exit_code);
}

int main(int argc, char ** argv)
{
  {
    struct rlimit rlim;
    rlim.rlim_cur = 256;
    rlim.rlim_max = 256;
    int res = setrlimit(RLIMIT_NOFILE, & rlim);
    assert( res == 0);
  }
  
  if (argc != 3 || chdir(argv[2])) usage(argv[0], -1);

  Results res = analyzeDir(std::stoi(argv[1]));
  printf("--------------------------------------------------------------\n");
  printf("Largest file:      \"%s\"\n", res.largest_file_path.c_str());
  printf("Largest file size: %ld\n", res.largest_file_size);
  printf("Number of files:   %ld\n", res.n_files);
  printf("Number of dirs:    %ld\n", res.n_dirs);
  printf("Total file size:   %ld\n", res.all_files_size);
  printf("Most common words from .txt files:\n");
  for (auto & w : res.most_common_words)
    printf(" - \"%s\" x %d\n", w.first.c_str(), w.second);
  printf("Vacant directories:\n");
  for (auto & d : res.vacant_dirs)
    printf(" - \"%s\"\n", d.c_str());
  printf("Largest images:\n");
  for (auto & ii : res.largest_images)
    printf(" - \"%s\" %ldx%ld\n", ii.path.c_str(), ii.width, ii.height );
  printf("--------------------------------------------------------------\n");
  return 0;
}
