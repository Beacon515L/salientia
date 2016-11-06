#include "recurse.h"
#include <iostream>
using namespace std;

vector<string> Recurse::read_files(string path){
    vector<string> retValue;
    
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string dname = string(ent->d_name);
            if (dname.compare(".") !=0 && dname.compare("..") !=0 && dname.compare(".git") !=0){
           // cerr << path << "/" << ent->d_name << endl;
            //Determine if path is a directory
            struct stat s;
    if( stat(string(path + string("/") + dname).c_str(),&s) == 0 ){
       if(S_ISDIR(s.st_mode)){ //cerr << "^Is dir" << endl;
        vector<string> temp = Recurse::read_files(path + string("/") + dname);
       // cerr << temp.size() << endl;
        retValue.insert(retValue.end(), temp.begin(), temp.end());}
    else {retValue.push_back(string(path + string("/") + dname));
       // cerr << "LOL" << endl;
        
    }
    }

            
        }
            
        }
        
    }
    //cout << retValue.size() << endl;
    
    sort(retValue.begin(), retValue.end());
    
return retValue; }
