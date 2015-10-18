#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <cerrno>
#include <cstring>

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#define DEFAULT_DICT "/usr/share/dict/words"
#define DEFAULT_FILE "."
#define DEFAULT_EXT "txt"
#define DEFAULT_DEXT "dict"

#define GRAY         "\x1B[00;30m"
#define BLACK        "\x1B[01;30m"
#define RED          "\x1B[00;31m"
#define BOLD_RED     "\x1B[00;31m"
#define GREEN        "\x1B[00;32m"
#define BOLD_GREEN   "\x1B[01;32m"
#define YELLOW       "\x1B[00;33m"
#define BOLD_YELLOW  "\x1B[01;33m"
#define BLUE         "\x1B[00;34m"
#define BOLD_BLUE    "\x1B[01;34m"
#define PURPLE       "\x1B[00;35m"
#define BOLD_PURPLE  "\x1B[01;35m"
#define CYAN         "\x1B[00;36m"
#define BOLD_CYAN    "\x1B[01;36m"
#define CLEAR        "\x1B[0m"

#define ALLOWED_PUNCT "_'"

#define SPELL_DCT_ENV "SPELL_DICTIONARIES"

typedef vector<string> strings;

typedef map<string,strings> report;

class Letters {
public:
  Letters();
  ~Letters();
  Letters *addLetter(char l,bool isEnd);
  bool match(const char *w, const char *e);

private:
  int bit(char l);
  
  Letters **lower;
  Letters **upper;
  Letters **punct;
  
  unsigned long end,next;
};

