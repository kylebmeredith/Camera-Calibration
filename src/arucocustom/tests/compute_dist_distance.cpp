#include "dictionary.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <vector>
using namespace std;
int main(int argc,char **argv){
    if (argc ==1 ) {
        cerr << "Invalid number of arguments" << endl;
        cerr << "Usage:  dictionary " << endl;
        cerr<<"\tDictionaries: "; for(auto dict:aruco::Dictionary::getDicTypes())    cerr<<dict<<" ";cerr<<endl;
        return false;
    }

    cerr<<"\tDictionaries: "; for(auto dict:aruco::Dictionary::getDicTypes())    cerr<<dict<<" ";cerr<<endl;

    auto dict=aruco::Dictionary::loadPredefined(aruco::Dictionary::getTypeFromString( argv[1]));
    cout<<aruco::Dictionary::computeDictionaryDistance(dict)<<endl;
}
