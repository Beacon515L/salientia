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
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include "ly.h"
#include "recurse.h"
#include <sstream>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <cctype>
#include <iomanip>

using namespace std;

bool overrideUTD = false, overrideNoEngrave = false, benchmark = false, outputList = false, pathRelativization = false, custom_args = false;
int cpuMHZ; float cpuMHZRatioWith1GHz;
static vector<Ly*> engrave, serial_engrave_list; static mutex mtx;
static vector<thread> threadList;
string extra_commands = "";

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

string url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

string* obtain_command(bool serial){
    int foo = 0;
    string* command = new string[3];
    command[0] = "";
    command[1] = "";
    command[2] = "";
    Ly* filepath = NULL;
    mtx.lock();
    if((!serial && engrave.size() > 0) || (serial && serial_engrave_list.size() > 0)){
        filepath = engrave.back();
        engrave.pop_back();
        if(filepath != NULL){
            command[1] = filepath->filename;
        cerr << endl << "ENGRAVING: " << command[1] << endl;
        mtx.unlock();
        string filepath_base = command[1];
        
        const char* filepath_str = string(filepath_base.c_str()).c_str();
        
        filepath_base = string(dirname(const_cast<char*>(filepath_str)));
        command[2] = filepath_base;
        //cerr << command[1] << endl;
        
        command[0] = "lilypond " + extra_commands + "--output=\"" + filepath_base + "\" --loglevel=NONE \"" + command[1] + "\"";
        }
        else mtx.unlock();
        
    }
    else mtx.unlock();
    return command;
}

int engrave_thread(int integer){ int retValue = -1;
        
        string* command = obtain_command(false);
        retValue = system(command[0].c_str());
        //Output file path relativization
        
      /*  if(pathRelativization){
            ifstream originalPDF;
            ofstream outputPDF;
            string filename = command[2]+"/"+command[1];
            string tmpfilename = filename+".tmp";
            originalPDF.open(filename, ios::in | ios::binary | ios::ate);
            outputPDF.open(tmpfilename, ios::out | ios::trunc | ios::binary);
            if(originalPDF.is_open()&&outputPDF.is_open()){
                streampos size = originalPDF.tellg();
                if(size!=-1){
                originalPDF.seekg (0, ios::beg);
                char * memblock = new char[size]; 
                originalPDF.read(memblock,size);
                
                const char * searchString = command[2].c_str();
                int searchStringLength = command[2].length();
              //TODO 
              //  if(size > searchStringLength)
               // for(s i = 0; i < (size - searchStringLength); i++){
             //      if (searchString[0]==memblock[i]
                    
                //}
                
                delete[] memblock;
            }
                
            }
            
            originalPDF.close();
            outputPDF.close();
            
            
        } */
        
        mtx.lock();
        if(command[1].length() > 0){
        cerr << endl << ((retValue==0)?"DONE: ":"ERROR: ") << command[1] << " - " << engrave.size() << " files remain" << endl;
        mtx.unlock();
        engrave_thread(0);}
        else mtx.unlock();
        
    
    return retValue;
}

int serial_engrave(bool benchmark){
    //TODO
    return 0;
}

//Based on GreenScape's answer to http://stackoverflow.com/questions/22802902/how-to-get-pid-of-process-executed-with-system-command-in-c
pid_t system2(const char * command, int * infp, int * outfp)
{
    int p_stdin[2];
    int p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) == -1)
        return -1;

    if (pipe(p_stdout) == -1) {
        close(p_stdin[0]);
        close(p_stdin[1]);
        return -1;
    }

    pid = fork();

    if (pid < 0) {
        close(p_stdin[0]);
        close(p_stdin[1]);
        close(p_stdout[0]);
        close(p_stdout[1]);
        return pid;
    } else if (pid == 0) {
        close(p_stdin[1]);
        dup2(p_stdin[0], 0);
        close(p_stdout[0]);
        dup2(p_stdout[1], 1);
        dup2(::open("/dev/null", O_RDONLY), 2);
        /// Close all other descriptors for the safety sake.
        for (int i = 3; i < 4096; ++i)
            ::close(i);

        setsid();
        execl("/bin/sh", "sh", "-c", command, NULL);
        _exit(1);
    }
}




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
    
    
    const string validFlags[] = {"-P", "-Iutd", "-Ine", "-o", "--args" //,"-M", "-B" //Disabled currently due to no performance-checking being done
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
    << "-o         - Outputs list of files to stdout instead of directly dispatching engraves" << endl
    << "--args     - Bypasses sanity-checking and supplies all unsupported arguments directly to the Lilypond threads" << endl
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
                        case 3: outputList = true; break;
                        case 4: custom_args = true; break;
                     //   case 4: if(++i<argc-1) memoryLimit = atoi(argv[i]); else retValue = 1; break;
                     //   case 5: benchmark = true; threads = 1; memoryLimit = 0; if(++i<argc-1) cpuMHZ = atoi(argv[i]); break;
                        
                    }
                    break;}
                    //Experimental feature - Direct Lilypond argument supplication
                    else { if(custom_args) {extra_commands += string(argv[i]) + " "; retValue = 0;} }
            
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
                if(!overrideNoEngrave)
                {  struct no_engrave_check : public std::unary_function<const std::string&, bool> {
            bool operator()(const Ly* file) const{
        return file->noEngrave;
    }
};

engrave.erase(remove_if(engrave.begin(), engrave.end(), no_engrave_check()),engrave.end());

            cerr << engrave.size() << " files remain after removing %NOENGRAVE-tagged files" << endl;}
            
            //Trim files that are up-to-date
            if(!overrideUTD)
               { struct up_to_date_check : public std::unary_function<const std::string&, bool> {
            bool operator()(const Ly* file) const{
                Ly* fileDeconst = const_cast<Ly*>(file);
        return !fileDeconst->checkIfNeedsReEngrave();
    }
};

engrave.erase(remove_if(engrave.begin(), engrave.end(), up_to_date_check()),engrave.end());

                cerr << engrave.size() << " files remain after removing up-to-date files" << endl;}

//Thread dispatching
if(outputList){
cerr << "Printing list of files needing engraving to stdout..." << endl << endl;
            for (int i = 0; i < engrave.size(); i++)
                cout << engrave[i]->filename << '\0';}
else { int thread_spawns = engrave.size(); if(thread_spawns > threads) thread_spawns = threads;
    cerr << "Spawning " << thread_spawns << " Lilypond threads" << endl << endl;
    for(int i = 0; i < thread_spawns; i++)
    {
        //TODO - Populate the serial-engraving list as well
        threadList.push_back(thread(engrave_thread,0));}
    auto threadIterator = threadList.begin();
    while (threadIterator != threadList.end())
    { threadIterator->join(); threadIterator++;}
    
}

        
        }
        
    }
    
  
    
    
    
    

    return retValue;
}


