#include "ly.h"
using namespace std;

vector<Ly*> Ly::files;

Ly::Ly(std::string filenm)
{   cerr << "   Reading " << filenm << endl; 
    const char * fnm = filenm.c_str();
    ifstream file; string contents; filename = realpath(fnm,NULL);
    struct stat buf; stat(fnm,&buf);
    modifiedTime = buf.st_mtime;
    
    file.exceptions ( ifstream::failbit | ifstream::badbit );
  try {
      //Read file to string for parsing dependencies
      const char * fnm = filename.c_str();
    file.open(fnm);
    stringstream b; b << file.rdbuf(); contents = b.str();
    file.close();
    
    //Check for %NOENGRAVE, %PERF and %MEM details
    noEngrave = (contents.find("NOENGRAVE",0) != string::npos);
    size_t perfPos = contents.find("\%PERF ",0);
    if(perfPos == string::npos) execSeconds = 0;
    else {string perfString = contents.substr(perfPos+6,contents.find('\n',perfPos)-perfPos-7);
        execSeconds = atoi(perfString.c_str());}
    size_t memPos = contents.find("\%MEM ",0);
    if(memPos == string::npos) execSeconds = 0;
    else {string memString = contents.substr(memPos+6,contents.find('\n',memPos)-memPos-7);
        execSeconds = atoi(memString.c_str());}
    
    //Identify all positions of \include predicates and following line breaks
    size_t pos = 0; vector<size_t> results; string needle = "\\include";
    do { pos = contents.find(needle,pos);
        if(pos != string::npos) {
            results.push_back(pos); pos = contents.find("\n",pos);
            results.push_back(pos); if (pos != string::npos) pos++;
    } }
    while (pos < string::npos);
    
    //Construct the array of \include lines from the offset positions
    for (int i = 0; i < results.size(); i+=2)
        dependencyPaths.push_back(contents.substr(results[i],results[i+1]-results[i]));
    
    //Truncate the \include lines to a list of actual paths and sanitize to canonical paths
    for (int i = 0; i < dependencyPaths.size(); i++){
        size_t firstQuote = -1, lastQuote = -1;
        
        size_t tmp = 0;
        do {tmp = dependencyPaths[i].find("\"",tmp);
        if(tmp != string::npos && dependencyPaths[i].at(tmp-1)!='\\') firstQuote = tmp++; else tmp++;}
        while(tmp != string::npos && firstQuote == -1);
        
        do {tmp = dependencyPaths[i].find("\"",tmp);
        if(tmp != string::npos && dependencyPaths[i].at(tmp-1)!='\\') lastQuote = tmp++; else tmp++;}
        while(tmp != string::npos && lastQuote == -1);
        char* path = const_cast<char*>(string(filename).c_str());
        
        //Obtain path to included file relative to directory of including file
        string includefnm = string(dependencyPaths[i].substr(firstQuote+1,lastQuote-firstQuote-1));
        char* base = basename(path);
        string basenm = ((base != NULL) ? base : "");
        string localdir = ((path != NULL) ? path : "");
        localdir = (localdir.substr(0,localdir.length() - basenm.length()) + "/./" + includefnm);
      
        char * rlpth = realpath(localdir.c_str(),NULL);
        
        //cerr << strerror(errno) << endl;
        dependencyPaths[i] = ((rlpth == NULL) ? "" : string(rlpth));
    }
    for (int i = 0; i < dependencyPaths.size(); i++){ bool isSet = false;
        
        //Check global files array for corresponding filenames; assign if present, newly create object if not
        for (int j = 0; j < Ly::files.size(); j++)
            if(dependencyPaths[i].compare(Ly::files[j]->filename) == 0) {dependencies.push_back(Ly::files[j]); isSet = true; break;}
        if(!isSet) {Ly* newLy = new Ly(dependencyPaths[i]); dependencies.push_back(newLy); Ly::files.push_back(newLy);}
    }
    
    //cerr << "Read: " << filename << endl;
    
    cerr << this->toString() << endl;
    
  }
  catch (ifstream::failure e) {
    cerr << "I/O error opening "+filename+"\n";
  }
}

    bool Ly::checkIfNeedsReEngrave(){ return this->checkIfNeedsReEngrave(getLastEngraveTime()); }
    bool Ly::checkIfNeedsReEngrave(time_t lastEngraveTime){
        bool retValue = (modifiedTime < lastEngraveTime);
        if(!retValue){
            for(int i = 0; i < dependencies.size(); i++){
                retValue = dependencies[i]->checkIfNeedsReEngrave(lastEngraveTime); if(retValue) break;}
        }
        return retValue;
    }
    
    time_t Ly::getLastEngraveTime(){time_t retValue;
        string engraveFilePath = filename.substr(0,filename.length() - 3) + ".pdf";
        const char * efp = engraveFilePath.c_str();
        struct stat statBuf; if(stat(efp,&statBuf)==0) retValue = statBuf.st_mtime;
        else retValue = 0;
        return retValue;
    }
    
    string Ly::toString(){stringstream retValue;
       retValue << filename << endl
       << modifiedTime << endl;
       if(noEngrave) retValue << "%NOENGRAVE" << endl
        << "Dependencies:" << endl;
        for (int i = 0; i < dependencies.size(); i++)
            retValue << "->" << dependencies[i]->filename << endl;
        retValue << endl;
        
        
        return retValue.str();
    }

