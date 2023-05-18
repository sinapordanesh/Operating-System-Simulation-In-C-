/// =========================================================================
/// Written by pfederl@ucalgary.ca in 2020, for CPSC457.
/// =========================================================================
/// You need to edit this file.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the analyzeDir() function as
/// defined in "analyzeDir.h".

#include "analyzeDir.h"

#include <cassert>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <bits/stdc++.h>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <algorithm>


using namespace std;


/// check if path refers to a directory
static bool is_dir(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISDIR(buff.st_mode);
}

/// check if path refers to a file
static bool is_file(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISREG(buff.st_mode);
}

/// check if string ends with another string
static bool ends_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

//=======Citation: code from the assignment 1========
/// split an string to several strings
vector<string> pixelSplit( const string & statement){
  auto line = statement + " ";
  bool in_str = false;
  string currentWord;
  vector<string> result;
  for( auto c : line){
    if(c == 'x' || isspace(c)){
      if(in_str) result.push_back(currentWord);
      in_str = false;
      currentWord = "";
    }
    else {
      currentWord.push_back(c);
      in_str = true;
    }
  }
  return result;
}


//===============End of the citation=============

//-----------------------Find word list on a string---------------------------

const int MAX_WORD_SIZE = 1024;
const int MIN_WORD_SIZE = 5;

//===============Code from provided repository in assignment manual==============
string nextWord(FILE* file){

  string result;
  while(1) {
    int c = fgetc(file);
    if(c == EOF) break;
    c = tolower(c);
    if(! isalpha(c)) {
      if(result.size() < MIN_WORD_SIZE || result.size() > MAX_WORD_SIZE){
        result.clear();
        continue;
      } else{
        break;
      }
    }
    else {
      result.push_back(c);
    }
  }
  return result;
}
//========================================End of citation========================



//-------------------------------------------------------------------------

//-------------------------Recursive directory search function----------------
string largestFilePath = "";
int largestFileSize = -1;
int totalFiles = 0;
int totalDirectories = 0;
int totalFileSizes = 0;
vector<ImageInfo> imageInfos {};
unordered_map<string,int> history;


void recursiveDirectorySearch(const string &basePath){

  if (is_file(basePath)){

    //---------------------------------------------------------
    /*
     * Largest file path
     * Largest file size
     * Number of files
     * Total file size
     */
    totalFiles++;
    struct stat buff;
    stat(basePath.c_str(), &buff);
    totalFileSizes += buff.st_size;
    if (buff.st_size > largestFileSize){
      largestFileSize = buff.st_size;
      largestFilePath = basePath;
    }
    //-----------------------------------------------------------

    //------------------------Read text files--------------------
    if (ends_with(basePath, ".txt")) {
      FILE * file;
      file = fopen(basePath.c_str(), "r");

      // this read the whole text file's words and count their iteration
      while (1) {
        auto w = nextWord(file);
        if (w.size() == 0) break;
        history[w]++;
      }
      fclose(file);
    }

    //-----------------------------------------------------------

    //----------------------Image file info----------------------
    string cmd = "identify -format '%wx%h' " + basePath + " 2> /dev/null";
    string img_size;
    auto fp = popen(cmd.c_str(), "r");
    assert(fp);
    char buff1[PATH_MAX];
    if(fgets(buff1, PATH_MAX, fp) != NULL) {
      img_size = buff1;
    }
    int status = pclose(fp);
    if( status != 0 or img_size[0] == '0')
      img_size = "";


    if (!img_size.empty()){

      vector<string> widthLengthString = pixelSplit(img_size);
      int tempWidth = stoi(widthLengthString.at(0));
      int tempLength = stoi(widthLengthString.at(1));
      string tempImagePath = basePath;

      ImageInfo tempStruct {tempImagePath.erase(0, 2), tempWidth, tempLength};
      imageInfos.push_back(tempStruct);
    }
    //---------------------------------------------------------
  } else if (is_dir(basePath)){
    //--------------------Number of directories----------------
    totalDirectories++;
    //---------------------------------------------------------
  }

  char path[1000];
  struct dirent *dp;
  DIR *dir = opendir(basePath.c_str());

  if (!dir)
    return;

  while ((dp = readdir(dir)) != NULL){
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
      strcpy(path, basePath.c_str());
      strcat(path, "/");
      strcat(path, dp->d_name);
      recursiveDirectorySearch(path);
    }
  }
  closedir(dir);
}

//---------------------------------------------------------------------------

//-------------------------Find vacant directories--------------------------------

vector<pair<string, string> > allVacants;

bool recursiveVacantSearch(
    const string &basePath,
    const string & parentName){

  if (is_file(basePath)){
    return false;
  }

  char path[1000];
  struct dirent *dp;
  DIR *dir = opendir(basePath.c_str());

  if (!dir)
    return false;

  bool result = true;
  while ((dp = readdir(dir)) != NULL){
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
      strcpy(path, basePath.c_str());
      strcat(path, "/");
      strcat(path, dp->d_name);
      bool tempResult = recursiveVacantSearch(path, basePath);
      result = result && tempResult;
    }
  }
  closedir(dir);
  if (result){
    pair<string, string> tempPair (basePath, parentName);
    allVacants.push_back(tempPair);
  }
  return result;
}
//-------------------------------------------------

//helper function to sort imageInfo vector<ImageInfo>
bool compareImagesByPixels(const ImageInfo &a, const ImageInfo &b)
{
  return (a.width * a.height) > (b.width * b.height);
}



Results analyzeDir(int n){
  Results res;
  //======================================================
  recursiveDirectorySearch(".");
  if(!largestFilePath.empty()){
    largestFilePath.erase(0, 2);
  }

  //--------------Most common word

  // ===== this code is from provided code by the professor =====
  vector<pair<int,string> > arr;
  // if(!history.empty()){
  for(auto & h : history){
    arr.emplace_back(-h.second, h.first);
  }
  // if we have more than N entries, we'll sort partially, since
  // we only need the first N to be sorted
  if(arr.size() > size_t(n)) {
    std::partial_sort(arr.begin(), arr.begin() + n, arr.end());
    // drop all entries after the first n
    arr.resize(n);
  } else {
    std::sort(arr.begin(), arr.end());
  }
  // }

  //======End of citation======

  vector<pair<string, int> > mostCommonWords;
  if(!arr.empty()){
    if (arr.size() > size_t(n)){
      for (int i = 0; i < n; i++) {
        auto &h = arr.at(i);
        mostCommonWords.emplace_back(h.second, -h.first);
      }
    } else{
      for (auto &h : arr){
        mostCommonWords.emplace_back(h.second, -h.first);
      }
    }
  }

  //---------------------------------------

  //---------------------Finding Vacant dir
  recursiveVacantSearch(".", "");

  vector<string> finalVacants {};
  if(!allVacants.empty()){
    for (int i = 0; size_t(i) < allVacants.size(); i++){
      string tempDirectory = allVacants.at(i).second;
      bool isVacant = true;
      for (int j = 0; size_t (j) < allVacants.size(); j++) {
        if (tempDirectory == allVacants.at(j).first){
          isVacant = false;
        }
      }
      if (isVacant){
        if(allVacants.at(i).first.size() > 2){
          finalVacants.push_back(allVacants.at(i).first.substr(2));
        } else{
          finalVacants.push_back(allVacants.at(i).first);
        }
      }
    }
  }
  //----------------------------------------

  //---------------------Sort the vector of images

  sort(imageInfos.begin(), imageInfos.end(), compareImagesByPixels);

  //----------------------------------------

  //====================================================================


  res.largest_file_path = largestFilePath;
  res.largest_file_size = largestFileSize;
  res.n_files = totalFiles;
  res.n_dirs = totalDirectories;
  res.all_files_size = totalFileSizes;

  res.most_common_words = mostCommonWords;

  if(!imageInfos.empty()){
    if (imageInfos.size() > size_t(n)){
      for (int i = 0; i < n; i++) {
        res.largest_images.push_back(imageInfos.at(i));
      }
    } else{
      res.largest_images = imageInfos;
    }
  }

  res.vacant_dirs = finalVacants;
  return res;
}
