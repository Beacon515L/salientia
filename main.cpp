/* SALIENTIA - By Liam Greenough - v0.1a
 * Dependency-traversing multithreaded batch-engraver CLI frontend for GNU Lilypond
 * Usage: salientia [flags] [directory or file]
 * Flags:
 * -P [1...N] - Number of threads to run (default: autodetected)
 * -M         - Override forced single-core engrave memory limit (default 75% of memory, 0 disables serial-engraving)
 * -Iutd      - Don't skip engrave of up-to-date files
 * -Ine       - Don't skip engrave of files marked %NOENGRAVE
 * -B         - Benchmark - runs single-core engrave and tags all files with memory usage and standardized CPU time (forces -Iutd -P 1 -M 0)
 */
#define _GLIBCXX_USE_CXX11_ABI 0
//#include <boost/filesystem.hpp>
//#include <boost/range/iterator_range.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
//#include <thread>
//#include <mutex>
#include <vector>
#include <string>
#include "ly.h"
#include "recurse.h"
#include <sstream>
#include <unistd.h>
using namespace std;

bool overrideUTD = false, overrideNoEngrave = false, benchmark = false;
int cpuMHZ; float cpuMHZRatioWith1GHz;

//TODO
/*
int getCPUClockMHz(){ifstream file;
     file.exceptions ( ifstream::failbit | ifstream::badbit );
     file.open("/proc/cpuinfo");
    stringstream b; b << file.rdbuf(); string contents = b.str();
    file.close();
    return 0;
    
}

bool compareLyByExecTime(Ly* i, Ly* j){return (i->execSeconds>j->execSeconds);}

int engrave(vector<Ly*> files){time_t time1, time2, difftime, equivtime; 
    for(int i = 0; i < files.size(); i++){
    boost::filesystem::path p(files[i]->filename); chdir(p.parent_path().c_str());
    const char * execString = ("lilypond \"" + p.filename().string() + "\"").c_str();
    if(benchmark) time(&time1);
    system(execString);
    if(benchmark){
        //Calculate equivalent seconds at 1GHz
        time(&time2);
        difftime = time2-time1;
        equivtime = difftime * cpuMHZRatioWith1GHz;
        //Write that value to the file
        ofstream filewrite;
        filewrite.open(p.parent_path().string() + "/" + p.filename().string());
        filewrite << endl << "\%PERF " << equivtime;
        
    }    
    
    }
    return 0;
}

*/

Ly* ly_wrapper(string filenm){Ly* retValue = NULL;
    for (int i = 0; i < Ly::files.size(); i++)
        if(filenm.compare(Ly::files[i]->filename) ==0) {
           retValue = Ly::files[i];
           cerr << "Already scanned " << filenm << ", skipping" << endl;
           break;
        }
        if (retValue == NULL) retValue = new Ly(filenm);
return retValue;}

int main(int argc, char **argv) {
    
    //vector<Ly*>* files = new vector<Ly*>();
    vector<Ly*> engrave;
    
    const string validFlags[] = {"-P", "-Iutd", "-Ine" //,"-M", "-B" //Disabled currently due to no performance-checking being done
    };
    const int validFlagsNum = 5;
    
    //Parse commandline arguments
    int retValue = 0; //Is ultimately the program's exit code
    
    if(argc==1) //No arguments, print usage information
    cerr << "SALIENTIA - By Liam Greenough - v0.1a" << endl
    << "Dependency-traversing multithreaded batch-engraver CLI frontend for GNU Lilypond" << endl
    << "Usage: salientia [flags] [directory or file]" << endl
    << "Flags:" << endl
    << "-P [1...N] - Number of threads to run (default: autodetected)" << endl
   // << "-M         - Override forced single-core engrave memory limit (default 75% of memory, 0 disables serial-engraving)" << endl
    << "-Iutd      - Don't skip engrave of up-to-date files" << endl
    << "-Ine       - Don't skip engrave of files marked \%NOENGRAVE" << endl
  //  << "-B [MHz]   - Benchmark - runs single-core engrave and tags all files with memory usage and standardized CPU time (forces -Iutd -P 1 -M 0)" << endl
  //  << "             MHz is maximum CPU clock frequency of your machine (mandatory, autodetection to be implemented)"
  << endl;
    
    
    
    
    else { string targetPath = argv[argc-1];
        int threads = sysconf(_SC_NPROCESSORS_ONLN), memoryLimit = 0;
        if(benchmark){
            
        }
        
        
        for (int i = 1; i < argc-1; i++){
            string tmp = string(argv[i]); retValue = 1;
            for (int j = 0; j < validFlagsNum; j++)
                if(tmp.compare(validFlags[j])==0) {retValue = 0;
                    switch (j){
                        case 0: if(++i<argc-1) threads = atoi(argv[i]); else retValue = 1; break; 
                        case 1: overrideUTD = true; break;
                        case 2: overrideNoEngrave = true; break;
                     //   case 3: if(++i<argc-1) memoryLimit = atoi(argv[i]); else retValue = 1; break;
                     //   case 4: benchmark = true; threads = 1; memoryLimit = 0; if(++i<argc-1) cpuMHZ = atoi(argv[i]); break;
                        
                    }
                    break;}
            
        }
        
        cerr << "SALIENTIA v0.1a" << endl
        << "Threads: " << threads << endl
        << ((overrideUTD)?"Ignoring timestamps":"") << endl
        << ((overrideNoEngrave)?"Ignoring \%NOENGRAVE flag":"") << endl;
        
     //   if(benchmark) cpuMHZRatioWith1GHz = cpuMHZ / 1000;
    
    //Determine if targetPath is a directory (needing recursion) or a simple file
    //Having done that, populate the list of files for engrave
    const char * tgpath = targetPath.c_str();
    struct stat statBuf; 
    
   
    
    if(stat(tgpath,&statBuf)!=0) { retValue = 1; }
    
    
    else { 
        if(statBuf.st_mode & S_IFDIR) {
            cerr << targetPath << endl;
            vector<string> files_in_path = Recurse::read_files(targetPath);
                     
                cerr << "FILES RECURSED: " << files_in_path.size() << endl;
        
               for (int i = 0; i < files_in_path.size(); i++) {
                   string tempfilename = files_in_path[i].substr(files_in_path[i].find_last_of(".") + 1);
                   transform(tempfilename.begin(),tempfilename.end(), tempfilename.begin(), ::tolower);
                    
                    if(tempfilename.compare("ly") ==0 ) {
                        cerr << "-----------------------------------------------------------------------" << endl <<
                        "Scanning " << files_in_path[i] << endl << endl;
                        Ly* temp = ly_wrapper(files_in_path[i]);
                        cerr << "Done scanning " << files_in_path[i] << endl << endl;
                        engrave.push_back(temp);}}
                   
                    
           
        }
        
        else if(statBuf.st_mode & S_IFREG) {
            if(targetPath.substr(targetPath.length()-3).compare(".ly")==0) engrave.push_back(ly_wrapper(targetPath));}
        
        else retValue = 1; //Unhandled case presently (symlinks?)
    }
    
    if(retValue!=0) cerr << "Insane arguments; I'm outta here." << endl;
    else { cerr << "----------------------------------" 
        << endl << "Done scanning " << engrave.size() << " files" << endl << endl;
        //for (int i = 0; i < engrave.size(); i++) cerr << engrave[i]->filename << endl;
        
        //PHASE 1: EXECUTION-PLANNING
        //Eliminate files flagged %NOENGRAVE or that are up-to-date unless overridden
        //if(!overrideNoEngrave && !overrideUTD)
       // for(int i = 0; i < engrave.size(); i++)
       //     if((engrave[i]->noEngrave && !overrideNoEngrave) || (!overrideUTD && engrave[i]->checkIfNeedsReEngrave())) 
       //         engrave.erase(engrave.begin()+i--);
        
        //Allocate files exceeding memory-limit to serial queue
            /*
        vector<Ly*> serial, nonserial;
        for (int i = 0; i<engrave->size(); i++)
            if(memoryLimit<(*engrave)[i]->execMemory) serial.push_back((*engrave)[i]); else nonserial.push_back((*engrave)[i]);
       
            
        //Allocate parallel queue to threads
        //Sort list by most execution time to least, then each iteration allocate the next item to the core with the least load
        //sort(nonserial.begin(),nonserial.end(),compareLyByExecTime);
        vector<vector<Ly*>*> threadLists;
        
        //Begin populating threadlists
        for (int i = 0; i < threads; i++) threadLists.push_back(new vector<Ly*>());
        
        cerr << "Populating " << threads << " threads..." << endl;
        for (int i = 0; i < nonserial.size(); i++){ int minSize = -1, minCore = -1;
            //Totalize each core's loading
            for (int j = 0; j < threadLists.size(); j++){ int coreTotal = 0;
                cerr << "U/A: " << threadLists.size()-j << " ";
                for (int k = 0; k < threadLists[j]->size(); k++) coreTotal += (*threadLists[j])[k]->execSeconds;
                if(coreTotal<minSize||minSize==-1) {minSize = coreTotal; minCore = j;}
                cerr << j << ":" << coreTotal << " ";
            }
            threadLists[minCore]->push_back(nonserial[i]);
            cerr << endl;
        }
        //Execute threads
       // vector<thread> threadHandles; for(int i = 0; threadLists.size(); i++)
        //    threadHandles.push_back(thread(engrave,threadLists[i]));
        //    for (int i=0; i < threadHandles.size(); i++)
         //       threadHandles[i].join();
         
         */
            
                cerr << engrave.size() << " files to engrave" << endl;
                
            //Trim files tagged %NOENGRAVE
            struct no_engrave_check : public std::unary_function<const std::string&, bool> {
            bool operator()(const Ly* file) const{
        return file->noEngrave;
    }
};

engrave.erase(remove_if(engrave.begin(), engrave.end(), no_engrave_check()),engrave.end());

            cerr << engrave.size() << " files remain after removing %NOENGRAVE-tagged files" << endl;
            
            //Trim files that are up-to-date
                struct up_to_date_check : public std::unary_function<const std::string&, bool> {
            bool operator()(const Ly* file) const{
                Ly* fileDeconst = const_cast<Ly*>(file);
        return !fileDeconst->checkIfNeedsReEngrave();
    }
};

engrave.erase(remove_if(engrave.begin(), engrave.end(), up_to_date_check()),engrave.end());

                cerr << engrave.size() << " files remain after removing up-to-date files" << endl;

//CURRENTLY NOT USING C++ THREAD DISPATCH, INSTEAD USING XARGS

cerr << "Printing list of files needing engraving to stdout..." << endl << endl;
            for (int i = 0; i < engrave.size(); i++)
                cerr << engrave[i]->filename << endl;
        
        }
        
    }
    
  
    
    
    
    

    return retValue;
}


