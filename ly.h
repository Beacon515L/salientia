/* Ly class - describes an .ly file, its engravability and dependency structure */
#define _GLIBCXX_USE_CXX11_ABI 0
#include <string>
#include <vector>
 #include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <libgen.h>
using namespace std;
class Ly {
public:
    static vector<Ly*> files;
    string filename;
    time_t modifiedTime;
    int execSeconds;
    int execMemory;
    bool noEngrave;
    vector<Ly*> dependencies;
    vector<string> dependencyPaths;
    Ly(string filenm); //vector<Ly*>* files);
    bool checkIfNeedsReEngrave();
    bool checkIfNeedsReEngrave(time_t lastEngraveTime);
    time_t getLastEngraveTime();
    string toString();
    
};
