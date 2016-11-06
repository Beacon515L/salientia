// Recursive file lister using opendir/readdir/closedir
//Currently Linux-only
#define _GLIBCXX_USE_CXX11_ABI 0
#include <algorithm>
#include <vector>
#include <string>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

class Recurse {
public:
    static vector<string> read_files(string path);

};
