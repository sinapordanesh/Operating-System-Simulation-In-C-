
//============================== Adopted code from slow-poli.cpp =================================

#include <unistd.h>
#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <iostream>

//================================= Adopted code from fast-int.cpp ===============================
// we'll use global variables to track of characters that we
// read() into memory

char buffer[1024*1024]; // 1MB sotrage to hold results of read()
int buff_size = 0;      // stores how many characters are stored in buffer
int buff_pos = 0;       // position in bufffer[] where we extract next char

// returns the next char from stdin, or -1 on EOF
// returns characters stored in buffer[] until it is empty
// when buffer[] gets empty, it is replenished by calling read()

int fast_read_from_stdin()
{
  // check if buffer[] is empty
  if( buff_pos >= buff_size) {
    // buffer is empty, let's replenish it
    buff_size = read( STDIN_FILENO, buffer, sizeof(buffer));
    // detect EOF
    if(buff_size <= 0) return -1;
    // reset position from where we'll return characters
    buff_pos = 0;
  }
  // return the next character from the buffer and update position
  return buffer[buff_pos++];
}

//================================= End of adopted code from fast-int.cpp ===========================

// reads in a word from STDIN
// reads until space or \n or EOF and result includes \n if present
// returns empty string on EOF

std::string stdin_readword()
{
  int c;
  std::string result;
  bool in_str = false;

  while(true) {
    c = fast_read_from_stdin();
    if(c == -1) break; //an error occured 
    if(isspace(c) || c == '\n'){
      if(in_str){
        break;
      } else{
        continue;
      }
    } else{
      result.push_back(c);
      in_str = true;
    }
    if(c == '\n') break;
  }
  return result;
}


// returns true if a word is palindrome
// palindrome is a string that reads the same forward and backward
//    after converting all characters to lower case
bool
is_palindrome( const std::string & s)
{
  for( size_t i = 0 ; i < s.size() / 2 ; i ++)
    if( tolower(s[i]) != tolower(s[s.size()-i-1]))
      return false;
  return true;
}

// Returns the longest palindrome on standard input.
// In case of ties for length, returns the first palindrome found.


std::string get_longest_palindrome()
{
  std::string max_pali;
  while(1) {
    std::string word = stdin_readword();
    if( word.size() == 0) break;
    if( word.size() <= max_pali.size())
      continue;
    if(is_palindrome(word)){
      max_pali = word;
    }
  }
  return max_pali;
}

int main()
{
  std::string max_pali = get_longest_palindrome();
  printf("Longest palindrome: %s\n", max_pali.c_str());
  return 0;
}

//============================ End of adopted code from slow-poli.cpp ============================